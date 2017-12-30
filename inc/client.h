#ifndef AMAZOOM_CLIENT_H
#define AMAZOOM_CLIENT_H

#include "catalogue.h"
#include "shared.h"

#define CLIENT_VIEW_CATALOGUE  		1
#define CLIENT_VIEW_CART  			2
#define CLIENT_ADD_TO_CART  		3
#define CLIENT_REMOVE_FROM_CART  	4
#define CLIENT_PLACE_ORDER  		5
#define CLIENT_EXIT  				6

void displayMenu();
void viewCatalogue(Catalogue catalogue);
void viewCart(Catalogue catalogue, int cart[MAX_ORDER_SIZE]);
void addToCart(Catalogue catalogue, int cart[MAX_ORDER_SIZE]);
void removeFromCart(Catalogue catalogue, int cart[MAX_ORDER_SIZE]);
void orderPlaced(int cart[MAX_ORDER_SIZE], int id);
void orderOutOfStock();

#endif //AMAZOOM_CLIENT_H
