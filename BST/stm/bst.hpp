#ifndef BST_H
#define BST_H

#include <iostream>
#include <cstdint>

#include "../../include/stm.h"
#include "../../include/mod_ab.h"
#include "../../include/mod_cb.h"
#include "../../include/mod_log.h"
#include "../../include/mod_mem.h"
#include "../../include/mod_order.h"
#include "../../include/mod_print.h"
#include "../../include/mod_stats.h"
#include "../../include/wrappers.h"

// Macros for TinySTM
#define RO                    1
#define RW                    0
#define TM_START(tid, ro)     { stm_tx_attr_t _a = {{.id = tid, .read_only = ro}}; sigjmp_buf *_e = stm_start(_a); if (_e != NULL) sigsetjmp(*_e, 0)
#define TM_LOAD(addr)         stm_load((stm_word_t *)addr)
#define TM_STORE(addr, value) stm_store((stm_word_t *)addr, (stm_word_t)value)
#define TM_MALLOC(size)       stm_malloc(size)
#define TM_COMMIT             stm_commit(); }
#define TM_INIT               stm_init(); mod_mem_init(0); mod_ab_init(0, NULL)
#define TM_EXIT               stm_exit()
#define TM_INIT_THREAD        stm_init_thread()
#define TM_EXIT_THREAD        stm_exit_thread()
#define TM_FREE(addr)         stm_free(addr, sizeof(*addr))
#define TM_FREE2(addr, size)  stm_free(addr, size)

class Node
{
    public:
        long key;
        Node *left, *right;
        // Make the node size equal to the cache line size
        uint8_t padding[40];
        
        Node(long k, Node *l, Node *r);
};

class BST
{
    private:
        Node *root;
        void auxPrintBst(Node *aux);

    public:
        BST();
        Node * get(long k);
        void add(long k);
        void del(long k);
        void printBST();
};
#endif
