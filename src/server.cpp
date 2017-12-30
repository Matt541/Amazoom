#include <thread>
#include "../inc/circular_queue.h"
#include "../inc/server.h"
#include "../inc/shared.h"

void service(cpen333::process::socket client)
{
	cpen333::process::shared_object<WarehouseInfo> winfo(AMAZOOM_WAREHOUSE_MEMORY_NAME);
	cpen333::process::shared_object<OrderHistory> ohist(AMAZOOM_ORDER_HISTORY_MEMORY_NAME);
	cpen333::process::mutex winfo_mutex(AMAZOOM_WAREHOUSE_MUTEX_NAME);
	cpen333::process::mutex ohist_mutex(AMAZOOM_ORDER_HISTORY_MUTEX_NAME);
	CircularQueue queue;
	Order order;
	OrderStatus order_status;
	static int id = 1;

	while (true) {
		int stock[CATALOGUE_SIZE];

		if (client.read_all(&order.items, sizeof(order.items)) != true) {
#ifdef TESTING
			std::cout << "Server: error reading from socket" << std::endl;
#endif
		}

		std::unique_lock<cpen333::process::mutex> winfo_lock(winfo_mutex);
		for (int i = 0; i < CATALOGUE_SIZE; ++i) {
			stock[i] = winfo->stock[i];
		}
		for (int i = 0; i < MAX_ORDER_SIZE; ++i) {
			if (order.items[i] != 0) {
				if (stock[order.items[i]] > 0) {
					stock[order.items[i]]--;
				} else {
					order_status.status = STATUS_OUT_OF_STOCK;
				}
			}
		}
		if (order_status.status != STATUS_OUT_OF_STOCK) {
			for (int i = 0; i < CATALOGUE_SIZE; ++i) {
				winfo->stock[i] = stock[i];
			}
		}
		winfo_lock.unlock();

		if (order_status.status != STATUS_OUT_OF_STOCK) {
			std::unique_lock<cpen333::process::mutex> ohist_lock(ohist_mutex);
			ohist->history[ohist->idx].id = id;
			ohist->history[ohist->idx].status = STATUS_PROCESSING;
			ohist->idx = (ohist->idx + 1) % ORDER_HISTORY_SIZE;
			ohist_lock.unlock();

			order.id = id;
			order_status.id = id;
			id++;
			queue.add(order);
		}

		if (client.write(&order_status, sizeof(order_status)) != true) {
#ifdef TESTING
			std::cout << "Server: error writing to socket" << std::endl;
#endif
		}

		order_status.status = STATUS_OK;
	}
}

int main()
{
  	cpen333::process::socket_server server(AMAZOOM_SERVER_PORT);
  	server.open();
  	std::cout << "Server started on port " << server.port() << std::endl;

  	cpen333::process::socket client;

  	while (server.accept(client)) {
   		std::thread thread(service, std::move(client));
        thread.detach();
  	}

  	server.close();

  	return 0;
}
