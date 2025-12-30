#ifndef ACTIONHISTORY_H
#define ACTIONHISTORY_H

#include <stack>
#include <string>
#include <iostream>

using namespace std;

enum ActionType {
    ADD_ORDER,
    PROCESS_ORDER,
    DISPATCH_ORDER
};

struct ActionRecord {
    ActionType type;
    int orderId;
    int itemId;
    int quantity;
    int priority;
};

// Logs actions and enables Undo functionality using a Stack
class ActionHistory {
private:
    stack<ActionRecord> history;

public:
    void logAction(ActionRecord record) {
        history.push(record);
    }

    bool hasActions() {
        return !history.empty();
    }

    ActionRecord popLastAction() {
        if (!history.empty()) {
            ActionRecord last = history.top();
            history.pop();
            return last;
        }
        // Return dummy if empty (caller should check hasActions)
        return {ADD_ORDER, -1, -1, 0, 0};
    }

    // Peeking for display if needed
    void showHistory() {
        cout << "\n--- Recent Actions (Stack Trace) ---\n";
        // Simplified view
        cout << "(Stack size: " << history.size() << ")\n";
        cout << "------------------------------------\n";
    }
};

#endif
