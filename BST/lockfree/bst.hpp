#ifndef BST_H
#define BST_H

#include <iostream>
#include <cstdint>

// Macros
#define NONE 0x0
#define MARK 0x1
#define CASCHILD 0x2
#define REALOCATE 0x3
#define CAS(a, b, c) __sync_bool_compare_and_swap(a, b, c)
#define VCAS(a, b, c) __sync_val_compare_and_swap(a, b, c)

#define ONGOING 1
#define FAILED 3
#define SUCCESFUL 2

#define NOTFOUND_R 0
#define NOTFOUND_L 1
#define FOUND 2
#define ABORT 3

class Operation {};

class Node
{
    public:
        long key;
        Operation * op;
        Node *left;
        Node *right;
        // Make the node size equal to the cache line size
        uint8_t padding[40];
        
        Node(long k, Node *l, Node *r);
        ~Node();
};

class ChildCASOp : Operation 
{
    public:
        bool isLeft;
        Node *expected;
        Node *update;
        ChildCASOp(bool l, Node *e, Node *u);
};

class RelocateOp : Operation 
{
    public:
        int state = ONGOING;
        Node *dest;
        Operation *destOp;
        long removeKey;
        long replaceKey;

        RelocateOp(Node *curr, Operation *currOp, long k, long rk);
};

class BST
{
    private:
        Node *root;

        void auxPrintBst(Node *aux);
        long is_marked(void *n);
        long get_unmarked(void *n);
        long get_marked(void *n);

        int getAux(long k, Node *& pred, Operation *& predOp, Node*& curr, 
                Operation *& currOp, Node *auxRoot);
        void helpChildCAS(ChildCASOp *op, Node *dest);
        void help(Node *pred, Operation *predOp, Node *curr, Operation *currOp);
        void helpMarked(Node *pred, Operation *predOp, Node *curr);
        bool helpRelocate(RelocateOp *op, Node *pred, Operation *predOp, Node *curr);

    public:
        BST();
        ~BST();
        int get(long k);
        void add(long k);
        void del(long k);
        void printBST();
};
#endif
