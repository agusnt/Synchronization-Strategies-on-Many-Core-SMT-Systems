#ifndef BST_H
#define BST_H

#include <immintrin.h>
#include <iostream>
#include <climits>
#include <cstdint>
#include <atomic>
#include <omp.h>

class alignas(64) Node
{
    public:
        long key;
        Node *left, *right;
        bool marked;
        // Make the node size equal to the cache line size
        uint8_t padding[40];
        
        Node(long k, Node *l, Node *r);
        ~Node();
};

class BST
{
    private:
        alignas(64) Node *root;
        alignas(64) const int limit = 10;
        alignas(64) std::atomic<bool> spinlock;
        alignas(64) struct htm_counters {
            uint64_t abort = 0;
            uint64_t commit = 0;
            uint64_t fallpath = 0;
            uint64_t capacity = 0;
            uint64_t conflict = 0;
            uint64_t expl = 0;
            uint8_t padding[16];
        } htm[88];


        void auxPrintBst(Node *aux);
        inline void startHTM();
        inline void endHTM();

    public:
        BST();
        ~BST();
        Node * get(long k);
        void add(long k);
        void del(long k);
        void printBST();
        void printHTM();
};
#endif
