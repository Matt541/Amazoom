#ifndef AMAZOOM_ROBOT_H
#define AMAZOOM_ROBOT_H

#include <random>
#include "shared.h"

class Robot : public cpen333::thread::thread_object {
private:
protected:
    cpen333::process::shared_object<WarehouseInfo> winfo_;
	cpen333::process::shared_object<RobotFleet> finfo_;
	cpen333::process::mutex winfo_mutex_;
	cpen333::process::mutex finfo_mutex_;
    int route_[MAX_ORDER_SIZE] = {0};

	void moveToLocation(int shelf, int rid)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::unique_lock<cpen333::process::mutex> finfo_lock(finfo_mutex_);
		finfo_->rinfo[rid].rloc[ROW_IDX] = winfo_->sinfo[shelf].sloc.row;
		finfo_->rinfo[rid].rloc[COL_IDX] = winfo_->sinfo[shelf].sloc.col;
	}

public:
    Robot() : winfo_(AMAZOOM_WAREHOUSE_MEMORY_NAME), finfo_(AMAZOOM_ROBOT_MEMORY_NAME),
	winfo_mutex_(AMAZOOM_WAREHOUSE_MUTEX_NAME), finfo_mutex_(AMAZOOM_ROBOT_MUTEX_NAME){
		// finfo_->rinfo[rid].rloc[ROW_IDX] = DEFAULT_ROW;
		// finfo_->rinfo[rid].rloc[COL_IDX] = DEFAULT_COL;
	}
};

class DeliveryRobot : public Robot {
private:
	cpen333::process::shared_object<BayInfo> binfo_;
	cpen333::process::shared_object<OrderHistory> ohist_;
	cpen333::process::mutex ohist_mutex_;
	int rid = 0;
	Order order;
	CircularQueue queue;
	int order_size = 0;
	int bay = NUMBER_OF_BAYS;
	bool full = false;
	bool busy = false;

	int main()
	{
		std::unique_lock<cpen333::process::mutex> finfo_lock(finfo_mutex_);
		for (int i = NUMBER_OF_ROBOTS - 1; i > 0; --i) {
			if (finfo_->rinfo[i].id == 0) {
				finfo_->rinfo[i].id = i;
				rid = i;
				break;
			}
		}
		finfo_lock.unlock();

		while (true) {
			if (busy == true) {
				while(full == false) {
					order = queue.get();
#ifdef TESTING
					std::cout << "dRobot doing delivery" << std::endl;
#endif
					std::unique_lock<cpen333::process::mutex> ohist_lock(ohist_mutex_);
					for (int i = 0; i < ORDER_HISTORY_SIZE; ++i) {
						if (ohist_->history[i].id == order.id) {
							ohist_->history[i].status = STATUS_GATHERING;
							break;
						}
					}
					ohist_lock.unlock();

					getOrderSize(order.items);
					calculateRoute(order.items, route_);
					for (int i = 0; i < order_size; ++i) {
						moveToLocation(route_[i], rid);
						takeItem(order.items[i], route_[i], i);
					}
					moveToLocation(BAY_LOCATION, rid);
					dropLoad();

					ohist_lock.lock();
					for (int i = 0; i < ORDER_HISTORY_SIZE; ++i) {
						if (ohist_->history[i].id == order.id) {
							ohist_->history[i].status = STATUS_DELIVERING;
							break;
						}
					}
					ohist_lock.unlock();

					if (binfo_->cargo[bay][MAX_TRUCK_CARGO - TRUCK_FULL_ENOUGH] != 0) {
                        full = true;
					}
#ifdef TESTING
					std::cout << "dRobot finished doing delivery" << std::endl;
#endif
				}
				full = false;
				busy = false;
			}
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}

		return 0;
	}

	void getOrderSize(int order[MAX_ORDER_SIZE])
	{
		order_size = 0;

		for (int i = 0; i < MAX_ORDER_SIZE; ++i) {
			if (order[i] != 0) {
				order_size += 1;
			}
		}
	}

    void calculateRoute(int order[MAX_ORDER_SIZE],
						int (&route_)[MAX_ORDER_SIZE])
    {
		for (int i = 0; i < order_size; ++i) {
			for (int n = 0; n < NUMBER_OF_SHELVES; ++n) {
				std::unique_lock<cpen333::process::mutex> winfo_lock(winfo_mutex_);
				if (winfo_->sinfo[n].supply[order[i]] > 0) {
					route_[i] = n;
				}
			}
		}
    }

	void takeItem(int id, int shelf, int idx)
    {
		std::unique_lock<cpen333::process::mutex> winfo_lock(winfo_mutex_);
		winfo_->sinfo[shelf].supply[id]--;
		finfo_->rinfo[rid].cargo[idx] = id;
#ifdef TESTING
		std::cout << "dRobot took item: " << finfo_->rinfo[rid].cargo[idx] << std::endl;
#endif
    }

	void dropLoad()
	{
		std::unique_lock<cpen333::process::mutex> finfo_lock(finfo_mutex_);
#ifdef TESTING
		std::cout << "dRobot dropped load: ";
		for (int i = 0; i < MAX_ROBOT_CARGO; ++i) {
			std::cout << finfo_->rinfo[rid].cargo[i];
		}
		std::cout << std::endl;
#endif
		for (int i = 0; i < order_size; ++i) {
			for (int n = 0; n < MAX_TRUCK_CARGO; ++n) {
				if (binfo_->cargo[bay][n] == 0) {
					binfo_->cargo[bay][n] = finfo_->rinfo[rid].cargo[i];
					finfo_->rinfo[rid].cargo[i] = 0;
				}
			}
		}

	}

public:
	DeliveryRobot() :  Robot(), binfo_(AMAZOOM_BAY_MEMORY_NAME), ohist_(AMAZOOM_ORDER_HISTORY_MEMORY_NAME), ohist_mutex_(AMAZOOM_ORDER_HISTORY_MUTEX_NAME){}

	void doDelivery(int idx)
	{
		bay = idx;
		busy = true;
    }
};

class InventoryRobot : public Robot {
private:
	cpen333::process::shared_object<BayInfo> binfo_;
	int rid = 0;
	int bay = NUMBER_OF_BAYS;
	bool empty = false;
	bool busy = false;

	int main() {
		std::unique_lock<cpen333::process::mutex> finfo_lock(finfo_mutex_);
		for (int i = NUMBER_OF_ROBOTS - 1; i > 0; --i) {
			if (finfo_->rinfo[i].id == 0) {
				finfo_->rinfo[i].id = i;
				rid = i;
				break;
			}
		}
		finfo_lock.unlock();

		while (true) {
			if (busy == true) {
#ifdef TESTING
				std::cout << "iRobot doing inventory" << std::endl;
#endif
				while (empty == false) {
					moveToLocation(BAY_LOCATION, rid);
					takeLoad(bay);
					calculateRoute(route_);
					for (int i = 0; i < MAX_ROBOT_CARGO; ++i) {
						if (finfo_->rinfo[rid].cargo[i] != 0) {
							moveToLocation(route_[i], rid);
							dropItem(finfo_->rinfo[rid].cargo[i], route_[i], i);
						}
					}
					if (binfo_->cargo[bay][0] == 0) {
                        empty = true;
					}
				}
#ifdef TESTING
				std::cout << "iRobot finished doing inventory" << std::endl;
#endif
				empty = false;
				busy = false;
			}
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		return 0;
	}

	void takeLoad(int bay)
	{
		std::unique_lock<cpen333::process::mutex> finfo_lock(finfo_mutex_);
		for (int i = 0; i < MAX_ROBOT_CARGO; ++i) {
			for (int n = MAX_TRUCK_CARGO - 1; n >= 0; --n) {
				if (binfo_->cargo[bay][n] != 0) {
					// if (weight + iinfo_->item[id].weight < MAX_ROBOT_WEIGHT) {
					finfo_->rinfo[rid].cargo[i] = binfo_->cargo[bay][n];
					binfo_->cargo[bay][n] = 0;
					break;
				}
			}
		}
#ifdef TESTING
		std::cout << "iRobot took load: ";
		for (int i = 0; i < MAX_ROBOT_CARGO; ++i) {
			std::cout << finfo_->rinfo[rid].cargo[i];
		}
		std::cout << std::endl;
#endif
	}

	void calculateRoute(int (&route_)[MAX_ORDER_SIZE])
    {
		std::default_random_engine rnd((unsigned int)std::chrono::system_clock::now().time_since_epoch().count());
		std::uniform_int_distribution<int> dist(0, NUMBER_OF_SHELVES - 1);

		std::unique_lock<cpen333::process::mutex> finfo_lock(finfo_mutex_);
		for (int i = 0; finfo_->rinfo[rid].cargo[i] != 0; ++i) {
			route_[i] = dist(rnd);
		}
    }

	void dropItem(int id, int shelf, int idx)
    {
		std::unique_lock<cpen333::process::mutex> winfo_lock(winfo_mutex_);
		// if (winfo_->sinfo[shelf].weight + iinfo_->item[id].weight < MAX_SHELF_WEIGHT) {
		winfo_->sinfo[shelf].supply[id]++;
		winfo_->stock[id]++;
#ifdef TESTING
		std::cout << "iRobot dropped item: " << finfo_->rinfo[rid].cargo[idx] << std::endl;
#endif
		finfo_->rinfo[rid].cargo[idx] = 0;
		// } else {
		// 	std::default_random_engine rnd((unsigned int)std::chrono::system_clock::now().time_since_epoch().count());
		// 	std::uniform_int_distribution<int> dist(0, NUMBER_OF_SHELVES - 1);

		// 	dropItem(id, dist(rnd), idx);
		// }
    }

public:
	InventoryRobot() : Robot(), binfo_(AMAZOOM_BAY_MEMORY_NAME){}

	void doInventory(int idx)
	{
		bay = idx;
		busy = true;
	}

};

#endif //AMAZOOM_ROBOT_H
