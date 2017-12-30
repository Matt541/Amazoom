#ifndef AMAZOOM_SHARED_H
#define AMAZOOM_SHARED_H

#include "cpen333/process/mutex.h"
#include "cpen333/process/semaphore.h"
#include "cpen333/process/shared_memory.h"
#include "cpen333/process/socket.h"
#include "cpen333/process/subprocess.h"
#include "cpen333/thread/thread_object.h"

// #define TESTING 1

#define AMAZOOM_SIMULATION_MEMORY_NAME "amazoom_simulation_memory_name"
#define AMAZOOM_WAREHOUSE_MEMORY_NAME "amazoom_warehouse_memory_name"
#define AMAZOOM_BAY_MEMORY_NAME "amazoom_bay_memory_name"
#define AMAZOOM_ROBOT_MEMORY_NAME "amazoom_robot_memory_name"
#define AMAZOOM_ITEM_MEMORY_NAME "amazoom_item_memory_name"
#define AMAZOOM_BUFFER_MEMORY_NAME "amazoom_buffer_memory_name"
#define AMAZOOM_ORDER_HISTORY_MEMORY_NAME "amazoom_order_history_memory_name"

#define AMAZOOM_WAREHOUSE_MUTEX_NAME "amazoom_warehouse_mutex_name"
#define AMAZOOM_BAY_MUTEX_NAME "amazoom_bay_mutex_name"
#define AMAZOOM_ROBOT_MUTEX_NAME "amazoom_robot_mutex_name"
#define AMAZOOM_ITEM_MUTEX_NAME "amazoom_item_mutex_name"
#define AMAZOOM_ORDER_HISTORY_MUTEX_NAME "amazoom_order_history_mutex_name"
#define AMAZOOM_PMUTEX_NAME "amazoom_pmutex_name"
#define AMAZOOM_CMUTEX_NAME "amazoom_cmutex_name"

#define AMAZOOM_BAY_SEMAPHORE_NAME "amazoom_bay_semaphore_name"
#define AMAZOOM_UTRUCK_SEMAPHORE_NAME "amazoom_utruck_semaphore_name"
#define AMAZOOM_LTRUCK_SEMAPHORE_NAME "amazoom_ltruck_semaphore_name"
#define AMAZOOM_PSEMAPHORE_NAME "amazoom_psemaphore_name"
#define AMAZOOM_CSEMAPHORE_NAME "amazoom_csemaphore_name"

#define AMAZOOM_SERVER_PORT 52101

#define WALL_CHAR 'X'
#define SHELF_CHAR '|'
#define DOCK_CHAR '_'
#define EMPTY_CHAR ' '
#define LAYOUT_ROWS 9
#define LAYOUT_COLUMNS 11
#define ROW_IDX 0
#define COL_IDX 1
#define SIDE_LEFT 0
#define SIDE_RIGHT 1
#define HEIGHT_LOW 0
#define HEIGHT_HIGH 1
#define BAY_LOCATION 30

#define CATALOGUE_SIZE 6
#define MAX_ORDER_SIZE 10
#define CIRCULAR_BUFFER_SIZE 10
#define ORDER_HISTORY_SIZE 10

#define NUMBER_OF_ROBOTS 6
#define NUMBER_OF_TRUCKS 5
#define NUMBER_OF_BAYS 3
#define NUMBER_OF_SHELVES 60

#define MAX_ROBOT_CARGO 10
#define MAX_TRUCK_CARGO 15
#define TRUCK_FULL_ENOUGH 11
#define ROBOT_WEIGHT_LIMIT 20
#define TRUCK_WEIGHT_LIMIT 50
#define SHELF_WEIGHT_LIMIT 50

#define MAGIC_NUMBER 112358

typedef enum order_status_enum {
    STATUS_OUT_OF_STOCK     = -2,
    STATUS_ERROR            = -1,
	STATUS_OK               = 0,
    STATUS_PROCESSING       = 1,
    STATUS_GATHERING        = 2,
    STATUS_DELIVERING       = 3
} order_status_t;

struct Item {
	std::string item;
	int price;
	int weight;
	int id;
};

struct ItemInfo {
    Item item[CATALOGUE_SIZE];
};

struct Order {
    int items[MAX_ORDER_SIZE];
    int id;
};

struct OrderStatus {
    int id;
    order_status_t status;
};

struct OrderHistory {
    OrderStatus history[ORDER_HISTORY_SIZE];
    int idx;
};

struct CircularBuffer {
    Order buffer[CIRCULAR_BUFFER_SIZE];
	int pidx;
	int cidx;
};

struct ShelfLocation {
    int row;
    int col;
    int side;
    int height;
};

struct ShelfInfo {
    ShelfLocation sloc;
    int supply[CATALOGUE_SIZE];
    int weight;
};

struct WarehouseInfo {
    char layout[LAYOUT_ROWS][LAYOUT_COLUMNS];
    int stock[CATALOGUE_SIZE];
    ShelfInfo sinfo[NUMBER_OF_SHELVES];
};

struct BayInfo {
    int cargo[NUMBER_OF_BAYS][MAX_TRUCK_CARGO];
    bool full[NUMBER_OF_BAYS];
    bool loading[NUMBER_OF_BAYS];
    bool unloading[NUMBER_OF_BAYS];
};

struct RobotInfo {
    int id;
    int rloc[2];
    int cargo[MAX_ROBOT_CARGO];
    bool busy;
};

struct RobotFleet {
    RobotInfo rinfo[NUMBER_OF_ROBOTS];
};

struct SimulationInfo {
    int magic;
    bool shutdown;
};

#endif //AMAZOOM_SHARED_H
