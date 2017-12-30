#ifndef AMAZOOM_CATALOGUE_H
#define AMAZOOM_CATALOGUE_H

#include <fstream>
#include <map>
#include <string>
#include "json.hpp"
using JSON = nlohmann::json;
#include "shared.h"

class Catalogue {
	std::map<int,Item> catalogue_;

public:
	Catalogue() : catalogue_(){}

	void load(const std::string& filename)
	{
		std::ifstream fin(filename);

		if (fin.is_open()) {
			JSON jcatalogue;
			fin >> jcatalogue;

      		for (const auto& jitem : jcatalogue) {
				Item item;
				item.item = jitem["item"];
				item.price = jitem["price"];
				item.weight = jitem["weight"];
				item.id = jitem["id"];

				catalogue_.insert({item.id, item});
			}
		}
	}

	Item getItem(int id)
	{
		Item item = catalogue_[id];
		return item;
	}

	std::string getItemName(int id)
	{
		Item item = catalogue_[id];
		std::string name = item.item;
		return name;
	}

	int getItemPrice(int id)
	{
		Item item = catalogue_[id];
		int price = item.price;
		return price;
	}

	int getItemWeight(int id)
	{
		Item item = catalogue_[id];
		int weight = item.weight;
		return weight;
	}
};

#endif //AMAZOOM_CATALOGUE_H
