#ifndef AMAZOOM_TRUCK_H
#define AMAZOOM_TRUCK_H

#include <random>
#include "shared.h"

class Truck : public cpen333::thread::thread_object {
private:
    cpen333::process::shared_object<ItemInfo> iinfo_;
	cpen333::process::shared_object<BayInfo> binfo_;
	cpen333::process::semaphore bsem_;
	cpen333::process::semaphore utruck_;
	cpen333::process::semaphore ltruck_;
	cpen333::process::mutex binfo_mutex_;
    int cargo[MAX_TRUCK_CARGO];
	bool status = false;
    int bay = NUMBER_OF_BAYS;

	int main()
	{
		generateCargo();
		while(bsem_.try_wait() == false) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
#ifdef TESTING
		std::cout << "Truck arriving" << std::endl;
#endif
		for (int i = 0; i < NUMBER_OF_BAYS; ++i) {
			if (binfo_->full[i] == false) {
				binfo_->full[i] = true;
				bay = i;
				break;
			}
		}
		shareCargo();

		binfo_->unloading[bay] = true;
		utruck_.notify();
#ifdef TESTING
		std::cout << "Truck unloading" << std::endl;
#endif
		while (binfo_->cargo[bay][0] != 0) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}

		binfo_->loading[bay] = true;
		ltruck_.notify();
#ifdef TESTING
		std::cout << "Truck loading" << std::endl;
#endif
		while (binfo_->cargo[bay][MAX_TRUCK_CARGO - TRUCK_FULL_ENOUGH] == 0) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}

		binfo_->full[bay] = false;
		bsem_.notify();
#ifdef TESTING
		std::cout << "Truck leaving" << std::endl;
#endif
		status = true;
		return 0;
	}

public:
    Truck() : iinfo_(AMAZOOM_ITEM_MEMORY_NAME), binfo_(AMAZOOM_BAY_MEMORY_NAME), bsem_(AMAZOOM_BAY_SEMAPHORE_NAME), utruck_(AMAZOOM_UTRUCK_SEMAPHORE_NAME), ltruck_(AMAZOOM_LTRUCK_SEMAPHORE_NAME), binfo_mutex_(AMAZOOM_BAY_MUTEX_NAME){}

	void generateCargo()
	{
		int id = 0;
		int weight = 0;
		int i = 0;

		std::default_random_engine rnd((unsigned int)std::chrono::system_clock::now().time_since_epoch().count());
		std::uniform_int_distribution<int> dist(1, 5);

		do {
			id = dist(rnd);
			cargo[i] = id;
			// weight += iinfo_->item[id].weight;
			i++;
		} while (weight < TRUCK_WEIGHT_LIMIT && i < MAX_TRUCK_CARGO);
	}

	void shareCargo()
	{
		std::unique_lock<cpen333::process::mutex> binfo_lock(binfo_mutex_);
		for (int i = 0; i < MAX_TRUCK_CARGO; ++i) {
			binfo_->cargo[bay][i] = cargo[i];
			cargo[i] = 0;
		}
	}

	bool done()
	{
		return status;
	}
};

#endif //AMAZOOM_TRUCK_H
