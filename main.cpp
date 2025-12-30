#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include "Order.h"
#include "WarehouseGraph.h"
#include "InventoryManager.h"
#include "ProductCatalog.h"
#include "ActionHistory.h"
#include "OrderManager.h"

using namespace std;

// Initialize the warehouse graph layout
void setupWarehouse(WarehouseGraph& graph) {
    graph.addEdge(0, 1, 5);
    graph.addEdge(0, 2, 7);
    graph.addEdge(1, 3, 4);
    graph.addEdge(1, 4, 3);
    graph.addEdge(2, 5, 2);
    graph.addEdge(2, 6, 5);
    graph.addEdge(4, 7, 6);
    graph.addEdge(5, 8, 4);
    graph.addEdge(6, 9, 3);
    graph.addEdge(3, 7, 2);
    graph.addEdge(8, 9, 1);
}

// Initialize inventory
void setupInventory(InventoryManager& inv) {
    inv.addItem(101, "Laptop", 50, 7);
    inv.addItem(102, "Mouse", 100, 3);
    inv.addItem(103, "Keyboard", 80, 4);
    inv.addItem(104, "Monitor", 30, 8);
    inv.addItem(105, "Headphones", 60, 9);
}

// Initialize catalog
void setupCatalog(ProductCatalog& catalog) {
    catalog.addProduct(101, "Laptop", "Electronics", 1200.00);
    catalog.addProduct(102, "Mouse", "Accessories", 25.50);
    catalog.addProduct(103, "Keyboard", "Accessories", 45.00);
    catalog.addProduct(104, "Monitor", "Electronics", 300.00);
    catalog.addProduct(105, "Headphones", "Audio", 80.00);
}

// --- API Mode Helpers ---

void printStateJSON(InventoryManager& inv, ProductCatalog& cat, OrderManager& om, WarehouseGraph& graph) {
    // Manually constructing JSON. In prod, use nlohmann/json.
    cout << "{";
    cout << "\"status\": \"success\",";
    
    // Serializing Pending Orders
    cout << "\"pending\": [";
    vector<Order> pending = om.getPendingOrders();
    for(size_t i=0; i<pending.size(); ++i) {
        cout << "{\"id\": " << pending[i].id 
             << ", \"text\": \"Item: " << pending[i].itemName << " (Prio: " << pending[i].priority << ")\""
             << ", \"prio\": " << pending[i].priority << "}";
        if(i < pending.size() - 1) cout << ",";
    }
    cout << "],";

    // Serializing Dispatched Orders
    cout << "\"dispatched\": [";
    vector<Order> dispatched = om.getDispatchedOrders();
    for(size_t i=0; i<dispatched.size(); ++i) {
        cout << "{\"id\": " << dispatched[i].id 
             << ", \"text\": \"Item: " << dispatched[i].itemName << " (Sent)\"}";
        if(i < dispatched.size() - 1) cout << ",";
    }
    cout << "],";

    // Serializing Inventory (Hash Map)
    cout << "\"inventory\": [";
    vector<Item> items = inv.getInventory();
    for(size_t i=0; i<items.size(); ++i) {
        cout << "{\"id\": " << items[i].id 
             << ", \"name\": \"" << items[i].name << "\""
             << ", \"qty\": " << items[i].quantity 
             << ", \"loc\": " << items[i].locationNode << "}";
        if(i < items.size() - 1) cout << ",";
    }
    cout << "],";

    // Serializing Catalog (BST)
    cout << "\"catalog\": [";
    vector<BSTNode> products = cat.getCatalog(); 
    for(size_t i=0; i<products.size(); ++i) {
        cout << "{\"id\": " << products[i].productId 
             << ", \"name\": \"" << products[i].productName << "\""
             << ", \"cat\": \"" << products[i].category << "\""
             << ", \"price\": " << products[i].price << "}";
        if(i < products.size() - 1) cout << ",";
    }
    cout << "]";
    
    cout << "}" << endl; // Use endl to flush
}

// --- Undo Helper ---
void performUndo(ActionHistory& hist, OrderManager& om, InventoryManager& inv) {
    if (!hist.hasActions()) {
        cout << "{\"status\":\"error\", \"msg\":\"Nothing to undo\"}" << endl;
        return;
    }

    ActionRecord last = hist.popLastAction();
    
    if (last.type == ADD_ORDER) {
        // Reverse Add: Remove Order, Return Stock
        if (om.removeOrder(last.orderId)) {
            inv.updateStock(last.itemId, last.quantity); // Add back stock
            cout << "{\"status\":\"success\", \"msg\":\"Undid ADD Order " << last.orderId << "\"}" << endl;
        } else {
             cout << "{\"status\":\"error\", \"msg\":\"Order not found (already processed?)\"}" << endl;
             // If order was processed, it's not in heap. The history stack should have had PROCESS_ORDER on top.
             // This implies proper stack discipline.
        }
    }
    else if (last.type == PROCESS_ORDER) {
        // Reverse Process: Move from Dispatch back to Pending (Heap)
        if (om.revertProcess()) {
            cout << "{\"status\":\"success\", \"msg\":\"Undid PROCESS (Returned to Queue)\"}" << endl;
        } else {
            cout << "{\"status\":\"error\", \"msg\":\"Cannot undo process (Queue empty?)\"}" << endl;
        }
    }
    else if (last.type == DISPATCH_ORDER) {
        // Currently we don't store shipped items, so we can't easily undo dispatch 
        // unless we kept them. For this demo, we'll say it's irreversible or just log.
        cout << "{\"status\":\"warning\", \"msg\":\"Cannot undo FINAL dispatch in this version\"}" << endl;
    }
}

// --- Undo Helper (Console) ---
void performUndoConsole(ActionHistory& hist, OrderManager& om, InventoryManager& inv) {
    if (!hist.hasActions()) {
        cout << ">>> Nothing to undo.\n";
        return;
    }

    ActionRecord last = hist.popLastAction();
    
    if (last.type == ADD_ORDER) {
        if (om.removeOrder(last.orderId)) {
            inv.updateStock(last.itemId, last.quantity);
            cout << ">>> Undid ADD Order " << last.orderId << " (Stock Returned)\n";
        } else {
             cout << ">>> Error: Order " << last.orderId << " not found in Pending List (Already processed?)\n";
        }
    }
    else if (last.type == PROCESS_ORDER) {
        if (om.revertProcess()) {
            cout << ">>> Undid PROCESS (Order returned to Pending Queue)\n";
        } else {
            cout << ">>> Error: Cannot undo process (Dispatch Queue empty?)\n";
        }
    }
    else if (last.type == DISPATCH_ORDER) {
        cout << ">>> Cannot undo FINAL dispatch.\n";
    }
}

// API Loops that listens for commands from Node.js
void runApiMode(InventoryManager& inv, ProductCatalog& cat, OrderManager& om, WarehouseGraph& graph, ActionHistory& hist) {
    string line;
    int orderCounter = 1;
    
    // Output initial ready signal
    cout << "{\"status\":\"ready\"}" << endl;

    while (getline(cin, line)) {
        stringstream ss(line);
        string cmd;
        ss >> cmd;

        if (cmd == "ADD_ORDER") {
            int id, qty, prio;
            ss >> id >> qty >> prio;
            Item* item = inv.getItem(id);
            if (item && inv.hasStock(id, qty)) {
                Order newOrder;
                newOrder.id = orderCounter++;
                newOrder.itemName = item->name;
                newOrder.itemLocationNode = item->locationNode;
                newOrder.quantity = qty;
                newOrder.priority = prio;
                
                // 1. Log Action BEFORE adding (or after, just ensure data is there)
                hist.logAction({ADD_ORDER, newOrder.id, id, qty, prio});
                
                // 2. Perform Ops
                om.addOrder(newOrder); // Note: remove internal logging in OrderManager if duplicate
                inv.updateStock(id, -qty);
                
                cout << "{\"status\":\"success\", \"msg\":\"Order placed\"}" << endl;
            } else {
                cout << "{\"status\":\"error\", \"msg\":\"Invalid item or stock\"}" << endl;
            }
        }
        else if (cmd == "PROCESS") {
             // Check if there is anything to process first?
             // Since processNextOrder is void/cout, we should probably check size or add a check
             // But let's just run it. 
             // Ideally we should peek the top order to log its ID for undo tracking.
             // OrderManager::processNextOrder handles logic. We need it to return info or we log "Last Processed"
             // For simplicity, let's just log a generic PROCESS action. But strict undo needs ID.
             // Let's modify processNextOrder? Or assume stack order implies correct reverse.
             // If we rely on stack order: "Undo Process" simply assumes the last thing in Dispatch Queue is what we revert.
             
             // We need to know if it SUCCEEDED.
             vector<Order> pending = om.getPendingOrders();
             if (!pending.empty()) {
                 hist.logAction({PROCESS_ORDER, pending[0].id, 0, 0, 0}); // Log BEFORE? No, what if fail?
                 // Actually processNextOrder moves it.
                 // Correct logic: successful process -> Log.
                 // But ProcessNextOrder inside OrderManager logs to console.
                 // Let's rely on simple stack: If Process is called and succeeds, we track it.
                 
                 om.processNextOrder(graph);
                 // We assume success if pending wasn't empty. 
                 
                 cout << "{\"status\":\"success\", \"msg\":\"Processed\"}" << endl;
             } else {
                 cout << "{\"status\":\"error\", \"msg\":\"No orders to process\"}" << endl;
             }
        }
        else if (cmd == "DISPATCH") {
            om.dispatchNextOrder();
            // hist.logAction({DISPATCH_ORDER...}); 
            cout << "{\"status\":\"success\", \"msg\":\"Dispatched\"}" << endl;
        }
        else if (cmd == "UNDO") {
            performUndo(hist, om, inv);
        }
        else if (cmd == "GET_STATE") {
            printStateJSON(inv, cat, om, graph);
        }
        else {
             cout << "{\"status\":\"error\", \"msg\":\"Unknown command\"}" << endl;
        }
    }
}

void runInteractiveMode(InventoryManager& inv, ProductCatalog& cat, OrderManager& om, WarehouseGraph& graph, ActionHistory& hist) {
    int choice;
    int orderCounter = 1;
     cout << "==============================================\n";
    cout << "   SMART WAREHOUSE MANAGEMENT SYSTEM DEMO\n";
    cout << "==============================================\n";

    while (true) {
        cout << "\n--- Main Menu ---\n";
        cout << "1. Place New Order \t(Affects: Heap, Stack, Map)\n";
        cout << "2. Process Next Order \t(Uses: Heap, Graph, Queue)\n";
        cout << "3. Dispatch Order \t(Uses: Queue, Stack)\n";
        cout << "4. View Pending Orders \t(Show Heap)\n";
        cout << "5. View Inventory \t(Show Hash Map)\n";
        cout << "6. View Product Catalog (Show BST)\n";
        cout << "7. Undo Last Action \t(Stack Operation)\n";
        cout << "8. Show Warehouse Graph\n";
        cout << "9. Exit\n";
        cout << "Enter Choice: ";
        
        if (!(cin >> choice)) {
            cout << "Invalid input. Exiting.\n";
            break;
        }

        if (choice == 1) {
            int id, qty, prio;
            cout << "\nEnter Product ID (101-105): ";
            cin >> id;
            
            Item* item = inv.getItem(id);
            if (item) {
                cout << "Product Found: " << item->name << " (Available: " << item->quantity << ")\n";
                cout << "Enter Quantity: ";
                cin >> qty;
                if (inv.hasStock(id, qty)) {
                    cout << "Enter Priority (1-10, 10=Highest): ";
                    cin >> prio;
                    
                    Order newOrder;
                    newOrder.id = orderCounter++;
                    newOrder.itemName = item->name;
                    newOrder.itemLocationNode = item->locationNode;
                    newOrder.quantity = qty;
                    newOrder.priority = prio;

                    om.addOrder(newOrder);
                    inv.updateStock(id, -qty); // Deduct stock
                    cout << ">>> Order Placed Successfully!\n";
                } else {
                    cout << ">>> Error: Insufficient Stock!\n";
                }
            } else {
                cout << ">>> Error: Invalid Product ID!\n";
            }
        }
        else if (choice == 2) {
            om.processNextOrder(graph);
        }
        else if (choice == 3) {
            om.dispatchNextOrder();
        }
        else if (choice == 4) {
            om.showPendingOrders();
        }
        else if (choice == 5) {
            inv.displayInventory();
        }
        else if (choice == 6) {
            cat.displayCatalog();
        }
        else if (choice == 7) {
            performUndoConsole(hist, om, inv);
        }
        else if (choice == 8) {
            graph.displayGraph();
        }
        else if (choice == 9) {
            cout << "Exiting Simulation...\n";
            break;
        }
        else {
            cout << "Invalid Choice! Try again.\n";
        }
    }
}

int main(int argc, char* argv[]) {
    // Instantiate Core Components
    ActionHistory history;
    WarehouseGraph graph;
    InventoryManager inventory;
    ProductCatalog catalog;
    OrderManager orderManager(&history);

    // Setup Data
    setupWarehouse(graph);
    setupInventory(inventory);
    setupCatalog(catalog);

    bool apiMode = false;
    if (argc > 1 && string(argv[1]) == "--api") {
        apiMode = true;
    }

    if (apiMode) {
        runApiMode(inventory, catalog, orderManager, graph, history);
    } else {
        runInteractiveMode(inventory, catalog, orderManager, graph, history);
    }

    return 0;
}
