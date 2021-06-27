#ifndef HT_H
#define HT_H

#include <omp.h>
#include <atomic>
#include <cstdint>
#include <string.h>
#include <iostream>
#include <immintrin.h>

#define MAX_THREADS 88

class __attribute__((aligned(64))) Node
{
    public:
        uint64_t key;
        char** data;
        // Is the node logically deleted
        Node *next;
        // Node size of Intel Cache Line (64 byte)
        uint8_t padding[40];

        Node(uint64_t k, char** d, Node *n);
        ~Node();

};

class __attribute__((aligned(64))) Bucket_Hash
{
	public:
		Node * pointer;
		std::atomic<bool> spinlock;	
		uint8_t padding[48];
};

class HT
{
    private:
		
        //Node **array; 
		Bucket_Hash *array;
        int size;
        // Vars for HTM purpose
        // 15-Pereira-HTM_characteristics_seralization_Haswell -> This paper give 20
        // as the best number or retries
        const int limit = 20;

        //std::atomic<bool> *spinlock;

        struct htm_counters {
            uint64_t abort = 0;
            uint64_t commit = 0;
            uint64_t fallpath = 0;
            uint64_t capacity = 0;
            uint64_t conflict = 0;
            uint64_t expl = 0;
            uint64_t start = 0;
            uint8_t padding[8];
        } htm[88];

        inline void startHTM(std::atomic<bool> &spinlock);
        inline void endHTM(std::atomic<bool> &spinlock);

    public:

        HT(int s);
        ~HT();
        char** get(uint64_t k);
        void add(uint64_t k, char **d, bool init);
        void del(uint64_t k);
        void printHT();
        void printHTM();
};
#endif
