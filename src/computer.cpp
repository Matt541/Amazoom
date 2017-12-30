#include <fstream>
#include <thread>
#include <vector>
#include "../inc/catalogue.h"
#include "../inc/circular_queue.h"
#include "../inc/computer.h"
#include "../inc/robot.h"
#include "../inc/shared.h"
#include "../inc/trucks.h"
#include "../inc/ui.h"

void initLayout(char (&layout)[LAYOUT_ROWS][LAYOUT_COLUMNS])
{
    std::ifstream fin("./data/layout.txt");
    std::string line;

    if (fin.is_open()) {
        for (int row = 0; row < LAYOUT_ROWS; ++row) {
            std::getline(fin, line);
            for (int col = 0; col < LAYOUT_COLUMNS; ++col) {
                layout[row][col] = line[col];
            }
        }
        fin.close();
    }
}

void drobotHandler()
{
    CircularQueue queue;
    DeliveryRobot drobot[NUMBER_OF_ROBOTS/2];
    bool drobot_busy[NUMBER_OF_ROBOTS/2];
    int busy_count = 0;
    cpen333::process::shared_object<BayInfo> binfo_(AMAZOOM_BAY_MEMORY_NAME);
    cpen333::process::semaphore ltruck(AMAZOOM_LTRUCK_SEMAPHORE_NAME, 0);
    int idx = NUMBER_OF_BAYS;

    for (int i = 0; i < NUMBER_OF_ROBOTS/2; ++i) {
        drobot[i].start();
    }

    while (true) {
        while(ltruck.try_wait() == false) {
			std::this_thread::sleep_for(std::chrono::seconds(3));
		}
        for (int i = 0; i < NUMBER_OF_BAYS; ++i) {
            if (binfo_->loading[i] == true) {
				binfo_->loading[i] = false;
                idx = i;
				break;
			}
        }
        for (int i = 0; i < NUMBER_OF_ROBOTS/2; ++i) {
            if (drobot_busy[i] == false) {
                drobot_busy[i] = true;
                drobot[i].doDelivery(idx);
                break;
            } else {
                busy_count++;
            }
        }
        if(busy_count == 3) {
            for (int i = 0; i < NUMBER_OF_ROBOTS/2; ++i) {
                drobot_busy[i] = false;
            }
            drobot[0].doDelivery(idx);
        }
    }
}

void irobotHandler()
{
    InventoryRobot irobot[NUMBER_OF_ROBOTS/2];
    bool irobot_busy[NUMBER_OF_ROBOTS/2];
    int busy_count = 0;
    cpen333::process::shared_object<BayInfo> binfo_(AMAZOOM_BAY_MEMORY_NAME);
    cpen333::process::semaphore utruck(AMAZOOM_UTRUCK_SEMAPHORE_NAME, 0);
    int idx = NUMBER_OF_BAYS;

    for (int i = 0; i < NUMBER_OF_ROBOTS/2; ++i) {
        irobot[i].start();
    }

    while (true) {
        while(utruck.try_wait() == false) {
			std::this_thread::sleep_for(std::chrono::seconds(3));
		}
        for (int i = 0; i < NUMBER_OF_BAYS; ++i) {
            if (binfo_->unloading[i] == true) {
				binfo_->unloading[i] = false;
                idx = i;
				break;
			}
        }
        for (int i = 0; i < NUMBER_OF_ROBOTS/2; ++i) {
            if (irobot_busy[i] == false) {
                irobot_busy[i] = true;
                irobot[i].doInventory(idx);
                break;
            } else {
                busy_count++;
            }
        }
        if(busy_count == 3) {
            for (int i = 0; i < NUMBER_OF_ROBOTS/2; ++i) {
                irobot_busy[i] = false;
            }
            irobot[0].doInventory(idx);
        }
    }
}

void truckHandler()
{
    int num = 0;
    std::vector<Truck*> trucks;

    std::default_random_engine rnd((unsigned int)std::chrono::system_clock::now().time_since_epoch().count());
	std::uniform_int_distribution<int> dist(1, 15);

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        num = dist(rnd);
        if (num == 10 && trucks.size() < NUMBER_OF_TRUCKS) {
            trucks.push_back(new Truck());
            trucks.back()->start();
            trucks.back()->detach();
        }

        for (int i = 0; i < trucks.size(); ++i) {
            if (trucks.at(i)->done() == true) {
                delete trucks.at(i);
                trucks.at(i) = nullptr;
                trucks.erase(trucks.begin() + i);
            }
        }
    }
}

int main(void)
{
    // Initialize shared resources
    cpen333::process::shared_object<SimulationInfo> sinfo(AMAZOOM_SIMULATION_MEMORY_NAME);
    cpen333::process::shared_object<WarehouseInfo> winfo(AMAZOOM_WAREHOUSE_MEMORY_NAME);
    cpen333::process::shared_object<ItemInfo> iinfo(AMAZOOM_ITEM_MEMORY_NAME);
    cpen333::process::semaphore bsem(AMAZOOM_BAY_SEMAPHORE_NAME, 3);

    Catalogue catalogue;
    catalogue.load("./data/catalogue.json");
    for (int i = 0; i < CATALOGUE_SIZE; ++i) {
        iinfo->item[i] = catalogue.getItem(i);
    }

    // Initialize the warehouse layout, robots, trucks, server, and user interface
    cpen333::process::subprocess server("./server");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    initLayout(winfo->layout);

    std::thread drobots(drobotHandler);
    drobots.detach();
    std::thread irobots(irobotHandler);
    irobots.detach();
    std::thread trucks(truckHandler);
    trucks.detach();

    UserInterface ui;
    ui.start();

    // Indicate that Amazoom is open for business!
    sinfo->shutdown = false;
    sinfo->magic = MAGIC_NUMBER;

    // Loop until shutdown
    while (sinfo->shutdown != true) {}

    // Shutdown and free all resources
    ui.join();
    server.terminate();

    cpen333::process::shared_object<SimulationInfo>::unlink (AMAZOOM_SIMULATION_MEMORY_NAME);
    cpen333::process::shared_object<WarehouseInfo>::unlink (AMAZOOM_WAREHOUSE_MEMORY_NAME);
    cpen333::process::shared_object<BayInfo>::unlink (AMAZOOM_BAY_MEMORY_NAME);
    cpen333::process::shared_object<RobotFleet>::unlink(AMAZOOM_ROBOT_MEMORY_NAME);
    cpen333::process::shared_object<ItemInfo>::unlink(AMAZOOM_ITEM_MEMORY_NAME);
    cpen333::process::shared_object<CircularBuffer>::unlink(AMAZOOM_BUFFER_MEMORY_NAME);
    cpen333::process::shared_object<OrderHistory>::unlink(AMAZOOM_ORDER_HISTORY_MEMORY_NAME);

    cpen333::process::mutex::unlink (AMAZOOM_WAREHOUSE_MUTEX_NAME);
    cpen333::process::mutex::unlink (AMAZOOM_BAY_MUTEX_NAME);
    cpen333::process::mutex::unlink (AMAZOOM_ROBOT_MUTEX_NAME);
    // cpen333::process::mutex::unlink (AMAZOOM_ITEM_MUTEX_NAME);
    cpen333::process::mutex::unlink (AMAZOOM_ORDER_HISTORY_MUTEX_NAME);
    cpen333::process::mutex::unlink (AMAZOOM_PMUTEX_NAME);
    cpen333::process::mutex::unlink (AMAZOOM_CMUTEX_NAME);

    cpen333::process::semaphore::unlink (AMAZOOM_BAY_SEMAPHORE_NAME);
    cpen333::process::semaphore::unlink (AMAZOOM_UTRUCK_SEMAPHORE_NAME);
    cpen333::process::semaphore::unlink (AMAZOOM_LTRUCK_SEMAPHORE_NAME);
    cpen333::process::semaphore::unlink (AMAZOOM_PSEMAPHORE_NAME);
    cpen333::process::semaphore::unlink (AMAZOOM_CSEMAPHORE_NAME);

    std::cout << "completed." << std::endl;
    return 0;
}
