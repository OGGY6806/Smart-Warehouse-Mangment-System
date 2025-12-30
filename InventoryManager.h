#ifndef INVENTORYMANAGER_H
#define INVENTORYMANAGER_H

#include <unordered_map>
#include <string>
#include <iostream>

using namespace std;

struct Item {
    int id;
    string name;
    int quantity;
    int locationNode;
};

// Manages warehouse inventory using a Hash Map for O(1) access
class InventoryManager {
private:
    unordered_map<int, Item> inventory; // Key: ItemID, Value: Item object

public:
    // Add new item to inventory
    void addItem(int id, string name, int qty, int loc) {
        inventory[id] = {id, name, qty, loc};
    }

    // Retrieve item details
    Item* getItem(int id) {
        if (inventory.find(id) != inventory.end()) {
            return &inventory[id];
        }
        return nullptr;
    }

    // Check availability
    bool hasStock(int id, int qty) {
        if (inventory.find(id) != inventory.end()) {
            return inventory[id].quantity >= qty;
        }
        return false;
    }

    // Update stock level (can be negative for deduction)
    bool updateStock(int id, int change) {
        if (inventory.find(id) != inventory.end()) {
            inventory[id].quantity += change;
            return true;
        }
        return false;
    }
    
    void displayInventory() {
        cout << "\n--- Current Inventory (Hash Map) ---\n";
        cout << "ID\tName\t\tQty\tLocation\n";
        cout << "------------------------------------\n";
        for (auto& pair : inventory) {
             cout << pair.first << "\t" << pair.second.name 
                  << "\t\t" << pair.second.quantity << "\tNode " << pair.second.locationNode << "\n";
        }
        cout << "------------------------------------\n";
    }

    // --- Accessor for GUI ---
    vector<Item> getInventory() {
        vector<Item> items;
        for (auto& pair : inventory) {
            items.push_back(pair.second);
        }
        return items;
    }
};

#endif
