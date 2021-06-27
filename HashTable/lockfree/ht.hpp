#ifndef HT_H
#define HT_H

#include <cstdint>
#include <iostream>

class Node
{
    public:
        uint64_t key;
        uint64_t data;
        Node *next;
        // Node size of Intel Cache Line (64 byte)
        uint8_t padding[40];

        Node(uint64_t k, uint64_t d, Node *n);

};

class HT
{
    private:
        Node **array;
        int size;

        long is_marked(void *n);
        long get_unmarked(void *n);
        long get_marked(void *n);
        Node * get(uint64_t k, Node **left_node);

    public:
        HT(int s);
        Node * get(uint64_t k);
        void add(uint64_t k, uint64_t d);
        void del(uint64_t k);
        void printHT();
};

#endif
