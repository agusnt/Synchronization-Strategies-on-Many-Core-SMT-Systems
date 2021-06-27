#ifndef HT_H
#define HT_H

#include <cstdint>
#include <iostream>
#include <climits>
#include <atomic>
#include <mutex>
#include <omp.h>

class Node
{
    public:
        uint64_t key;
        uint64_t data;
        std::atomic<bool> marked;
        std::mutex lock;
        Node *next;
        // Node size of Intel Cache Line (64 byte)
        uint8_t padding[56];

        Node(uint64_t k, uint64_t d, Node *n);
};

// Structure that keep 
typedef struct free_list
{
    std::atomic<Node*> to_free;
    std::atomic<int> reset_threads;
} free_list_t;

class HT
{
    private:
        Node **array;
        int size;
        // Array that keep the index of the free_list for every thread
        long th_idx[88];

        bool validate(Node *pred, Node *curr);
    public:
        HT(int s);
        bool get(uint64_t k);
        void add(uint64_t k, uint64_t d);
        void del(uint64_t k);
        void printHT();
};
#endif
