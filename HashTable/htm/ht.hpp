#ifndef HT_H
#define HT_H

#include <omp.h>
#include <atomic>
#include <cstdint>
#include <string.h>
#include <iostream>
#include <immintrin.h>

#define MAX_THREADS 88

class Node
{
    public:
        uint64_t key;
        uint64_t data;
        // Is the node logically deleted
        bool marked;
        Node *next;
        // Node size of Intel Cache Line (64 byte)
        uint8_t padding[30];

        Node(uint64_t k, uint64_t d, Node *n);
        ~Node();

};

class HT
{
    private:
        alignas(64) Node **array;
        alignas(64) int size;
        // Vars for HTM purpose
        // 15-Pereira-HTM_characteristics_seralization_Haswell -> This paper give about 10
        // as the best number or retries
        alignas(64) const int limit = 10;
        alignas(64) std::atomic<bool> spinlock;
        alignas(64) struct htm_counters {
            uint64_t abort = 0;
            uint64_t commit = 0;
            uint64_t fallpath = 0;
            uint64_t capacity = 0;
            uint64_t conflict = 0;
            uint64_t expl = 0;
            uint64_t start = 0;
            uint8_t padding[8];
        } htm[88];

        inline void startHTM();
        inline void endHTM();

    public:
        HT(int s);
        ~HT();
        bool get(uint64_t k);
        void add(uint64_t k, uint64_t d);
        void del(uint64_t k);
        void printHT();
        void printHTM();
};

#endif
