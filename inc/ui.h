#ifndef AMAZOOM_UI_H
#define AMAZOOM_UI_H

#include "shared.h"
#include "catalogue.h"

#define MANAGER_GET_ORDER_STATUS 1
#define MANGER_GET_ITEM_STOCK 2
#define MANAGER_GET_ALERTS 3
#define MANAGER_SHUTDOWN 4

class UserInterface : public cpen333::thread::thread_object {
private:
    cpen333::process::shared_object<SimulationInfo> sinfo_;
	cpen333::process::shared_object<WarehouseInfo> winfo_;
    cpen333::process::shared_object<OrderHistory> ohist_;
    cpen333::process::mutex winfo_mutex_;
    cpen333::process::mutex ohist_mutex_;
	Catalogue catalogue_;

    void displayMenu()
    {
        std::cout << "=========================================" << std::endl;
        std::cout << "=            AMAZOOM TERMINAL           =" << std::endl;
        std::cout << "=========================================" << std::endl;
        std::cout << " (1) Get Order Status" << std::endl;
        std::cout << " (2) Get Item Stock" << std::endl;
        std::cout << " (3) Get Alerts" << std::endl;
        std::cout << " (4) Exit"  << std::endl;
        std::cout << "=========================================" << std::endl;
        std::cout << "Enter number: ";
        std::cout.flush();
    }

    void getOrderStatus()
    {
        int id;
        order_status_t status = STATUS_OK;
        std::string msg;


        std::cout << "Please enter the order ID: ";
        std::cin >> id;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::unique_lock<cpen333::process::mutex> ohist_lock(ohist_mutex_);
        for (int i = 0; i < ORDER_HISTORY_SIZE; ++i) {
            if (ohist_->history[i].id == id) {
                status = ohist_->history[i].status;
            }
        }
        ohist_lock.unlock();

        if (status == 1) {
            std::cout << "The status of order '" << id << "' is: processing" << std::endl;
        } else if (status == 2) {
            std::cout << "The status of order '" << id << "' is: gathering" << std::endl;
        } else if (status == 3) {
            std::cout << "The status of order '" << id << "' is: delivering" << std::endl;
        } else {
            std::cout << "Invalid order ID." << std::endl;
        }
    }

    void getItemStock()
    {
        int id = 0;
        int stock = 0;

        std::cout << "**********Inventory**********" << std::endl;
        for (int i = 1; i < CATALOGUE_SIZE; ++i) {
            std::cout << "(" << i << ") " << catalogue_.getItemName(i) << std::endl;
        }

        std::cout << "Please enter the item ID: ";
        std::cin >> id;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::unique_lock<cpen333::process::mutex> winfo_lock(winfo_mutex_);
        for (int i = 0; i < NUMBER_OF_SHELVES; ++i) {
            stock += winfo_->sinfo[i].supply[id];
        }
        winfo_lock.unlock();

        std::cout << "The stock of item '" << catalogue_.getItemName(id) << "' is: " << stock << std::endl;
    }

    void getAlerts()
    {
        int stock = 0;
		int alert = false;

        for (int i = 1; i < CATALOGUE_SIZE; ++i) {
            std::unique_lock<cpen333::process::mutex> winfo_lock(winfo_mutex_);
            for (int n = 0; n < NUMBER_OF_SHELVES; ++n) {
                stock += winfo_->sinfo[n].supply[i];
            }
            winfo_lock.unlock();

			if (stock < 5) {
				alert = true;
				std::cout << "WARNING! The stock of item '" << catalogue_.getItemName(i) << "' is low" << std::endl;
			}

			stock = 0;
        }

		if (alert == false) {
			std::cout << "Warehouse stock OK!" << std::endl;
		}
    }

    void shutdown()
    {
        std::cout << "System shutting down...";
        sinfo_->shutdown = true;
    }

    int main()
    {
        int cmd = 0;
		catalogue_.load("./data/catalogue.json");

        while (cmd != MANAGER_SHUTDOWN) {
            displayMenu();
            std::cin >> cmd;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            switch(cmd) {
                case MANAGER_GET_ORDER_STATUS:
                    getOrderStatus();
					cpen333::pause();
                    break;

                case MANGER_GET_ITEM_STOCK:
                    getItemStock();
					cpen333::pause();
                    break;

                case MANAGER_GET_ALERTS:
                    getAlerts();
					cpen333::pause();
                    break;

                case MANAGER_SHUTDOWN:
                    shutdown();
                    break;

                default:
                    std::cout << "Invalid command number: " << cmd << std::endl;
            }
        }

		return 0;
    }
public:
	UserInterface() : sinfo_(AMAZOOM_SIMULATION_MEMORY_NAME), winfo_(AMAZOOM_WAREHOUSE_MEMORY_NAME), ohist_(AMAZOOM_ORDER_HISTORY_MEMORY_NAME), winfo_mutex_(AMAZOOM_WAREHOUSE_MUTEX_NAME), ohist_mutex_(AMAZOOM_ORDER_HISTORY_MUTEX_NAME){}
};

#endif //AMAZOOM_UI_H
