#pragma once
#include <iostream>
#include <string>
#include<sstream>
using namespace std;

struct Theme {
    int themeID;
    string name;
    string description;
    string imageString;
    string musicString;
    int tilecolor;

    Theme(int id = -1, const string& n = "", const string& des="", const string& image = "", const string& str = "", int tile = 0)
        : themeID(id), name(n), imageString(image), musicString(str), tilecolor(tile), description(des){}
};

struct treeNode {
    Theme data;
    treeNode* left;
    treeNode* right;
    int height;

    treeNode(const Theme& theme)
        : data(theme), left(nullptr), right(nullptr), height(1) {
    }
};


class Inventory {
    treeNode* root;

    int height(treeNode* node) {
        return node ? node->height : 0;
    }

    int getBalance(treeNode* node) {
        if (!node) return 0;
        return height(node->left) - height(node->right);
    }

    int max(int a, int b) {
        return (a > b) ? a : b;
    }

    treeNode* rightRotate(treeNode* y) {
        treeNode* x = y->left;
        treeNode* T2 = x->right;

        // Perform rotation
        x->right = y;
        y->left = T2;

        // Update heights
        y->height = max(height(y->left), height(y->right)) + 1;
        x->height = max(height(x->left), height(x->right)) + 1;

        return x;
    }

    treeNode* leftRotate(treeNode* x) {
        treeNode* y = x->right;
        treeNode* T2 = y->left;

        // Perform rotation
        y->left = x;
        x->right = T2;

        // Update heights
        x->height = max(height(x->left), height(x->right)) + 1;
        y->height = max(height(y->left), height(y->right)) + 1;

        return y;
    }

    treeNode* insert(treeNode* node, const Theme& theme) {
        if (!node)
            return new treeNode(theme);

        if (theme.themeID < node->data.themeID)
            node->left = insert(node->left, theme);
        else if (theme.themeID > node->data.themeID)
            node->right = insert(node->right, theme);
        else
            return node;  // Duplicate themeID not allowed

        // Update height
        node->height = 1 + max(height(node->left), height(node->right));

        // Balance the node
        int balance = getBalance(node);

        // Left Left
        if (balance > 1 && theme.themeID < node->left->data.themeID)
            return rightRotate(node);

        // Right Right
        if (balance < -1 && theme.themeID > node->right->data.themeID)
            return leftRotate(node);

        // Left Right
        if (balance > 1 && theme.themeID > node->left->data.themeID) {
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }

        // Right Left
        if (balance < -1 && theme.themeID < node->right->data.themeID) {
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }

        return node;
    }

    void inOrderTraversal(treeNode* node) {
        if (node) {
            inOrderTraversal(node->left);
            cout << "ID: " << node->data.themeID
                << ", Name: " << node->data.name;
            inOrderTraversal(node->right);
        }
    }

    void freeTree(treeNode* node) {
        if (node) {
            freeTree(node->left);
            freeTree(node->right);
            delete node;
        }
    }

public:
    Inventory() : root(nullptr) {}

    ~Inventory() {
        freeTree(root);
    }

    treeNode* searchByName(treeNode* node, const string& name) {
        if (!node)
            return nullptr;

        if (node->data.name == name)
            return node;

        treeNode* leftResult = searchByName(node->left, name);
        if (leftResult)
            return leftResult;

        return searchByName(node->right, name);
    }
 
    void insertTheme(const Theme& theme) {
        root = insert(root, theme);
    }

    treeNode* searchByID(treeNode* node, int themeID) {
        if (!node || node->data.themeID == themeID)
            return node;

        if (themeID < node->data.themeID)
            return searchByID(node->left, themeID);
        else
            return searchByID(node->right, themeID);
    }
   
    void displayInOrder() {
        inOrderTraversal(root);
    }

    treeNode* getRoot() {
        return root;
    }

    void findByID(int themeID) {
        treeNode* result = searchByID(root, themeID);
        if (result) {
            cout << "Found: ID: " << result->data.themeID
                << ", Name: " << result->data.name;
        }
        else {
            cout << "Theme with ID " << themeID << " not found." << endl;
        }
    }

    void findByName(const string& name) {
        cout << "Searching for name: " << name << endl;
        searchByName(root, name);
    }

    void displayInOrderToStreamHelper(treeNode* node, stringstream& ss) {
        if (node != NULL) {
            displayInOrderToStreamHelper(node->left, ss);
            ss << "ID: " << node->data.themeID << " | "
                << node->data.name << " - "
                << node->data.description << endl;
            displayInOrderToStreamHelper(node->right, ss);
        }
    }

    void displayInOrderToStream(stringstream& ss) {
        displayInOrderToStreamHelper(root, ss);
    }

    void findByIDToStream(int themeID, stringstream& ss) {
        treeNode* result = searchByID(root, themeID);
        if (result) {
            ss << "Found Theme:\n";
            ss << "ID: " << result->data.themeID << "\n";
            ss << "Name: " << result->data.name << "\n";
            ss << "Description: " << result->data.description << "\n";
        }
        else {
            ss << "Theme with ID " << themeID << " not found!\n";
        }
    }
   
};
