#ifndef ORDERMANAGER_H
#define ORDERMANAGER_H

#include <queue>
#include <deque>
#include <vector>
#include <algorithm> // for heap operations
#include <iostream>
#include <string>
#include "Order.h"
#include "WarehouseGraph.h"
#include "ActionHistory.h"

using namespace std;

// Manages order processing using Heap (Priority Queue) and Dispatch Queue (FIFO)
class OrderManager {
private:
    // We use a vector with heap algorithms instead of priority_queue
    // to allow random access removal for UNDO functionality.
    vector<Order> orderHeap;
    
    // FIFO Queue for dispatched orders.
    // std::deque allows removal from back if we want to undo dispatch easily?
    // standard queue only allows pop_front. For undoing dispatch (putting back to pending), 
    // we might need to be creative or just use deque as the container.
    // Let's stick to deque for flexibility.
    deque<Order> dispatchQueue;

    ActionHistory* historyLogger;

public:
    OrderManager(ActionHistory* history) : historyLogger(history) {}

    void addOrder(const Order& order) {
        orderHeap.push_back(order);
        push_heap(orderHeap.begin(), orderHeap.end());
        
        // Log for Undo
        historyLogger->logAction({ADD_ORDER, order.id, 0, order.quantity, order.priority}); 
        // Note: itemId is not strictly in Order struct, simplistic mapping would need improvement 
        // but main.cpp passes itemId to history ideally. Order struct has itemName only.
        // We will fix main.cpp to log the ID. The OrderManager logAction might be redundant if main handles it.
        // Let's ONLY use OrderManager helper methods and let MAIN log the high level transaction to ensure 
        // Inventory + Order undo are coupled.
    }
    
    // Helper to remove a specific order by ID (needed for Undo Add)
    bool removeOrder(int orderId) {
        auto it = find_if(orderHeap.begin(), orderHeap.end(), [orderId](const Order& o) {
            return o.id == orderId;
        });

        if (it != orderHeap.end()) {
            // Remove from heap: swap with back, pop back, make_heap
            orderHeap.erase(it);
            make_heap(orderHeap.begin(), orderHeap.end());
            return true;
        }
        return false;
    }

    void processNextOrder(WarehouseGraph& graph) {
        if (orderHeap.empty()) {
            cout << "No pending orders to process.\n";
            return;
        }

        // Pop highest priority order
        pop_heap(orderHeap.begin(), orderHeap.end());
        Order currentOrder = orderHeap.back();
        orderHeap.pop_back();

        cout << "\nProcessing Order ID: " << currentOrder.id << " (Priority: " << currentOrder.priority << ")\n";
        
        // Path Optimization (Graph)
        pair<int, vector<int>> result = graph.getShortestPath(0, currentOrder.itemLocationNode);
        
        if (result.first != -1) {
            dispatchQueue.push_back(currentOrder); // Push to dispatch
            // Log isn't strictly needed here if we rely on main's "UNDO" command flow, 
            // but for tracking PROCESS actions:
             // historyLogger->logAction({...}); // Main.cpp will handle logging to capture state
        } else {
            cout << "Error: Unreachable item location!\n"; 
            // Put it back?
            orderHeap.push_back(currentOrder);
            push_heap(orderHeap.begin(), orderHeap.end());
        }
    }

    void dispatchNextOrder() {
        if (dispatchQueue.empty()) {
            cout << "No orders ready for dispatch.\n";
            return;
        }

        Order order = dispatchQueue.front();
        dispatchQueue.pop_front();
        cout << "Dispatching Order ID: " << order.id << "\n";
    }

    // Undo Process: Move from Dispatch Back to Pending
    bool revertProcess() {
        if (dispatchQueue.empty()) return false;
        
        // The last processed order is at the BACK of the dispatch queue? 
        // No, processNextOrder calls push_back(). 
        // So the most recently processed is at the back.
        Order last = dispatchQueue.back();
        dispatchQueue.pop_back();
        
        // Put back into heap
        orderHeap.push_back(last);
        push_heap(orderHeap.begin(), orderHeap.end());
        return true;
    }

    // Undo Dispatch: Move from "Shipped" back to Dispatch Queue?
    // We don't really store "Shipped" except in logs. 
    // If the user "Undoes" a Dispatch, we need to recover the order.
    // For this demo, let's say Dispatch is final unless we have a "Shipped" vector.
    // Let's add a `vector<Order> shippedHistory` if we want to undo dispatch.
    // For now, let's support undoing ADD and PROCESS primarily as they affect the visible lists.
    
    void showPendingOrders() {
        if (orderHeap.empty()) {
            cout << "No pending orders.\n";
            return;
        }
        // Copy to sort for display without destroying heap
        vector<Order> temp = orderHeap;
        sort_heap(temp.begin(), temp.end()); // sorts in ascending, so reverse for desc priority
        
        cout << "\n--- Pending Orders (Priority (Heap)) ---\n";
        for (auto it = temp.rbegin(); it != temp.rend(); ++it) {
             cout << "ID: " << it->id << " | Prio: " << it->priority << " | Item: " << it->itemName << endl;
        }
        cout << "----------------------------------------\n";
    }

    // --- Accessors for GUI/API Serialization ---
    
    vector<Order> getPendingOrders() {
        // Return sorted list
         vector<Order> temp = orderHeap;
         sort_heap(temp.begin(), temp.end());
         // sort_heap puts smallest at begin? No, max heap -> sort_heap -> sorted (ascending).
         // So we need to reverse to show highest priority first.
         reverse(temp.begin(), temp.end());
         return temp;
    }

    vector<Order> getDispatchedOrders() {
        vector<Order> list;
        for(const auto& o : dispatchQueue) {
            list.push_back(o);
        }
        return list;
    }
    
    // Check if queue has dispatch items (for undoing process)
    bool hasDispatched() { return !dispatchQueue.empty(); }
};

#endif
