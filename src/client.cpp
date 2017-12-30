#include <iostream>
#include "../inc/catalogue.h"
#include "../inc/client.h"
#include "../inc/shared.h"

void displayMenu()
{
	std::cout << "=========================================" << std::endl;
	std::cout << "=          AMAZOOM ONLINE STORE         =" << std::endl;
	std::cout << "=========================================" << std::endl;
	std::cout << " (1) View Catalogue" << std::endl;
	std::cout << " (2) View Cart" << std::endl;
	std::cout << " (3) Add to Cart" << std::endl;
	std::cout << " (4) Remove from Cart"  << std::endl;
	std::cout << " (5) Place Order"  << std::endl;
	std::cout << " (6) Exit"  << std::endl;
	std::cout << "=========================================" << std::endl;
	std::cout << "Enter number: ";
	std::cout.flush();
}

void viewCatalogue(Catalogue catalogue)
{
	std::cout << "**********Catalogue**********" << std::endl;
	for (int i = 1; i < CATALOGUE_SIZE; ++i) {
    	std::cout << "(" << i << ") " << catalogue.getItemName(i) << " - $" << catalogue.getItemPrice(i) << std::endl;
	}
}

void viewCart(Catalogue catalogue, int cart[MAX_ORDER_SIZE])
{
	std::cout << "**********Cart**********" << std::endl;
	for (int i = 0; i < MAX_ORDER_SIZE; ++i) {
		if (cart[i] != 0) {
			std::cout << catalogue.getItemName(cart[i]) << std::endl;
		}
	}
}

void addToCart(Catalogue catalogue, int cart[MAX_ORDER_SIZE])
{
	int id = 0;

	std::cout << "Please enter the item ID: ";
	std::cin >> id;
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	std::string name = catalogue.getItemName(id);

	if (name == ""){
		std::cout << "Invalid item ID." << std::endl;
	} else {
		for (int i = 0; i < MAX_ORDER_SIZE; ++i) {
			if (cart[i] == 0) {
				cart[i] = id;
				std::cout << name << " added to cart." << std::endl;
				return;
			}
		}
		std::cout << "Item not added because your cart is full." << std::endl;
	}
}

void removeFromCart(Catalogue catalogue, int cart[MAX_ORDER_SIZE])
{
	int id = 0;

	std::cout << "Please enter the item ID: ";
	std::cin >> id;
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	std::string name = catalogue.getItemName(id);

	if (name == ""){
		std::cout << "Invalid item ID." << std::endl;
	} else {
		for (int i = 0; i < MAX_ORDER_SIZE; ++i) {
			if (cart[i] == id) {
				cart[i] = 0;
				std::cout << name << " removed from cart." << std::endl;
				return;
			}
		}
		std::cout << "Item not removed because it is not in your cart." << std::endl;
	}
}

void orderPlaced(int cart[MAX_ORDER_SIZE], int id)
{
	std::cout << "Order placed! Your order number is: " << id << std::endl;

	for (int i = 0; i < MAX_ORDER_SIZE; ++i) {
		cart[i] = 0;
	}
}

void orderOutOfStock()
{
	std::cout << "Order not placed because an item is out of stock." << std::endl;
}

int main(void)
{
	cpen333::process::shared_object<SimulationInfo> sinfo(AMAZOOM_SIMULATION_MEMORY_NAME);
	if (sinfo->magic != MAGIC_NUMBER) {
		std::cout << "Amazoom is not open for business :(" << std::endl;
		return 0;
	}

	cpen333::process::socket client("localhost", AMAZOOM_SERVER_PORT);
	std::cout << "Client connecting...";
	std::cout.flush();

	if (client.open()) {
		std::cout << "connected." << std::endl;

		Catalogue catalogue;
  		catalogue.load("./data/catalogue.json");
		OrderStatus order_status;
		int cart[MAX_ORDER_SIZE] = {0};
		int cmd = 0;

		while (cmd != CLIENT_EXIT) {
			displayMenu();
			std::cin >> cmd;
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

			switch(cmd) {
				case CLIENT_VIEW_CATALOGUE:
					viewCatalogue(catalogue);
					cpen333::pause();
					break;

				case CLIENT_VIEW_CART:
					viewCart(catalogue, cart);
					cpen333::pause();
					break;

				case CLIENT_ADD_TO_CART:
					addToCart(catalogue, cart);
					break;

				case CLIENT_REMOVE_FROM_CART:
					removeFromCart(catalogue, cart);
					break;

				case CLIENT_PLACE_ORDER:
					if (cart[0] == 0) {
                        std::cout << "Order not placed because your cart is empty." << std::endl;
						break;
					}
                    if (client.write(&cart, sizeof(cart)) != true) {
#ifdef TESTING
						std::cout << "Client: error writing to socket." << std::endl;
						order_status.status = STATUS_ERROR;
#endif
					}
					if (client.read_all(&order_status, sizeof(order_status)) != true) {
						order_status.status = STATUS_ERROR;
#ifdef TESTING
						std::cout << "Client: error reading from socket." << std::endl;
#endif
					}
					if (order_status.status != STATUS_OUT_OF_STOCK) {
						orderPlaced(cart, order_status.id);
					} else {
						orderOutOfStock();
					}
					// } else {
					// 	std::cout << "ERROR placing order!" << std::endl;
					// }
					break;

				case CLIENT_EXIT:
					std::cout << "Thank you for shopping with us :)" << std::endl;
					break;

				default:
					std::cout << "Invalid command number: " << cmd << std::endl << std::endl;

			}
		}
	} else {
		std::cout << "failed." << std::endl;
	}

	return 0;
};
