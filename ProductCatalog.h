#ifndef PRODUCTCATALOG_H
#define PRODUCTCATALOG_H

#include <iostream>
#include <string>

using namespace std;

struct BSTNode {
    int productId;
    string productName;
    string category;
    double price;
    BSTNode* left;
    BSTNode* right;

    BSTNode(int id, string name, string cat, double p) 
        : productId(id), productName(name), category(cat), price(p), left(nullptr), right(nullptr) {}
};

// Manages product data using a Binary Search Tree (BST) for sorted storage and efficient search
class ProductCatalog {
private:
    BSTNode* root;

    // Helper: Recursive Insert
    BSTNode* insert(BSTNode* node, int id, string name, string cat, double price) {
        if (node == nullptr) {
            return new BSTNode(id, name, cat, price);
        }
        if (id < node->productId) {
            node->left = insert(node->left, id, name, cat, price);
        } else if (id > node->productId) {
            node->right = insert(node->right, id, name, cat, price);
        }
        return node;
    }

    // Helper: Recursive Search
    BSTNode* search(BSTNode* node, int id) {
        if (node == nullptr || node->productId == id) {
            return node;
        }
        if (id < node->productId) {
            return search(node->left, id);
        }
        return search(node->right, id);
    }

    // Helper: In-order traversal
    void inorder(BSTNode* node) {
        if (node != nullptr) {
            inorder(node->left);
            cout << "ID: " << node->productId << " | Name: " << node->productName 
                 << " | Category: " << node->category << " | Price: $" << node->price << endl;
            inorder(node->right);
        }
    }

public:
    ProductCatalog() : root(nullptr) {}

    void addProduct(int id, string name, string cat, double price) {
        root = insert(root, id, name, cat, price);
    }

    BSTNode* findProduct(int id) {
        return search(root, id);
    }

    void displayCatalog() {
        cout << "\n--- Product Catalog (BST In-Order Traversal) ---\n";
        inorder(root);
        cout << "------------------------------------------------\n";
    }

    // --- Accessor for GUI ---
    void collectNodes(BSTNode* node, vector<BSTNode>& list) {
        if (node != nullptr) {
            collectNodes(node->left, list);
            list.push_back(*node); // Copy node data
            collectNodes(node->right, list);
        }
    }

    vector<BSTNode> getCatalog() {
        vector<BSTNode> list;
        collectNodes(root, list);
        return list;
    }
};

#endif
