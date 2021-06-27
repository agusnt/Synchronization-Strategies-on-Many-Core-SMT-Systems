#ifndef BST_H
#define BST_H

#include <iostream>
#include <cstdint>
#include <climits>
#include <atomic>
#include <vector>
#include <mutex>

class Node
{
    public:
        long key;
        std::atomic<bool> marked;
        std::mutex lock;
        Node *left, *right;
        // Make the node size equal to the cache line size
        uint8_t padding[48];
        
        Node(long k, Node *l, Node *r);
        ~Node();
};

class BST
{
    private:
        Node *root;

        // Auxiliar function to print the node
        void auxPrintBst(Node *aux);
        // Get the next BST node
        Node * getNext(Node *x, long k);

    public:
        // Constructor
        BST();
        // Delete
        ~BST();
        // Get a key
        bool get(long k);
        // Add new key
        void add(long k);
        // Delete a key, can exist or not
        void del(long k);
        // Mark node to delete
        Node * delNoMark(Node *root, long k);
        // Print all tree
        void printBST();
};
#endif
