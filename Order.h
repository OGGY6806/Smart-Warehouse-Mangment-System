#ifndef ORDER_H
#define ORDER_H

#include <string>

// Represents a customer order in the warehouse
struct Order {
    int id;
    int priority;           // Higher value = higher priority
    std::string itemName;
    int quantity;
    int itemLocationNode;   // Node ID in the graph where item is located

    // Overloading < operator for priority_queue
    // The priority_queue is a max-heap, so the largest element is at the top.
    // By returning true if this.priority < other.priority, we ensure higher priority values come first.
    bool operator<(const Order& other) const {
        return priority < other.priority; 
    }
};

#endif
