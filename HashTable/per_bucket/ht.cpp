#include "ht.hpp"
#include <stdio.h>

using namespace std;

#define NOPS 30000

Node::Node(uint64_t k, char** d, Node *n)
{
    key = k;
    data = d;
    next = n;
}

Node::~Node()
{
}

HT::HT(int s)
{
	//std::cout << s << endl;
    size = s;
	array = new Bucket_Hash[s];
    //spinlock = new std::atomic<bool>[s];
    //array = new Node*[s];
    for (int i = 0; i < s; i++)
    {
        //array[i] = new Node(0, 0, nullptr);
	    //spinlock[i].store(false);
		array[i].pointer = new Node(0, 0, nullptr);
		array[i].spinlock.store(false);
    }
	memset(htm, 0, sizeof(htm_counters) * 88);
}

inline void HT::startHTM(std::atomic<bool> &spinlock)
{
    int retry = 0;
    unsigned status;

    while(true)
    {
		int thread_num = omp_get_thread_num();
        htm[thread_num].start++;
        if ((status = _xbegin()) != _XBEGIN_STARTED) {
            htm[thread_num].abort++;
            if (status & _XABORT_EXPLICIT) 
            {
                /*
                 * Explicit abort because other threads get the lock.
                 */
                if (_XABORT_CODE(status) == 0xFF && !(status & _XABORT_NESTED))
                {
                    htm[thread_num].expl++;
                    while(spinlock.load(std::memory_order_relaxed)) _mm_pause();
                    --retry;
                }
            }

            if (status & _XABORT_CONFLICT)
            {
                htm[thread_num].conflict++;
            } 
			else if(status & _XABORT_CAPACITY) 
            {
                htm[thread_num].capacity++;
                retry = limit;
            }
            if (++retry >= limit || status == 0)
            {
                htm[thread_num].fallpath++;
                // Acquire the spinlock
                while(true)
                {
                    // Wait in an spinlock
                    while (spinlock.load(std::memory_order_relaxed)) _mm_pause();

                    //Acquire lock
                    if (!spinlock.exchange(true)) break;
                }
                return;
            }
        } else
        {
            if (!spinlock.load(std::memory_order_relaxed)) return;
            _xabort(0xFF);
        }
    }
}

inline void HT::endHTM(std::atomic<bool> &spinlock)
{
    if (_xtest())
    {
        _xend();
    	asm volatile("":::"memory");
        //htm[num_thread].commit++;
    } else
    {
        // Unlock
        spinlock.store(false);
    }
}

void HT::add(uint64_t k, char **d, bool init)
{
    int index = k % size;
    bool added = true;

    Node *idx, **prev;
    /*
     * Create the new node to avoid aborts because of: page faults or systems
     * call
     */
    Node *newNode = new Node(k, d, nullptr);

    asm volatile("":::"memory"); // Avoid gcc reorder 
    if (!init) startHTM(array[index].spinlock);
    idx = array[index].pointer; 
    prev = &(idx->next);

    // Search for the position where the new node will be added
    while(idx && idx->key < k)
    {
        prev = &(idx->next);
        idx = idx->next;
    }

    if (!idx)
    {
        *prev = newNode;
    } else if (idx->key == k) 
    {
        //idx->data = d;
    } else
    {
        newNode->next = *prev;
        *prev = newNode;
    }
    // End transaction
    if (!init) {
		asm volatile (
        	"loop_1:          \n\t"
        	"   dec %0      \n\t"
        	"   jnz loop_1    \n\t"
        	: // Output vars
        	: "r" (NOPS)); // Input vars (src == 0)
	}
	//for (volatile unsigned long long nop = 0; nop < NOPS; nop++) asm("");
    if (!init) endHTM(array[index].spinlock);
    asm volatile("":::"memory"); // Avoid gcc reorder 
}

void HT::del(uint64_t k)
{
    int index = k % size;

    Node *idx, **prev; 

    asm volatile("":::"memory"); // Avoid gcc reorder 
    startHTM(array[index].spinlock);
    idx = array[index].pointer; 
    prev = &(array[index].pointer);

    // Search for the node
    while(idx && idx->key < k) 
    {
        prev = &(idx->next);
        idx = idx->next;
    }

    if (idx && idx->key == k) 
    {   
        // Logically remove
        *prev = idx->next;
    }
    // End Transaction
	asm (
    	"loop_2:          \n\t"
    	"   dec %0      \n\t"
    	"   jnz loop_2    \n\t"
    	: // Output vars
    	: "r" (NOPS)); // Input vars (src == 0)

    //for (volatile unsigned long long nop = 0; nop < NOPS; nop++) asm("");
    endHTM(array[index].spinlock);
    asm volatile("":::"memory"); // Avoid gcc reorder 

    // The delete must be out of the transactional section to avoid aborts
    if (idx && idx->key == k) delete idx;
}

char **HT::get(uint64_t k)
{
    int index = k % size;
    Node *idx;
    bool res = false; 

    asm volatile("":::"memory"); // Avoid gcc reorder 
    startHTM(array[index].spinlock);
    idx = array[index].pointer; 

    while(idx && idx->key < k) idx = idx->next;
    if (idx && idx->key == k) res = true;

    endHTM(array[index].spinlock);
    asm volatile("":::"memory"); // Avoid gcc reorder 

    //return res;
    if (res) return idx->data;
    else return NULL;
}

void HT::printHTM()
{
    uint64_t abort = 0;
    uint64_t commit = 0;
    uint64_t fallpath = 0;
    uint64_t capacity = 0;
    uint64_t conflict = 0;
    uint64_t expl = 0;
    uint64_t start = 0;
    for (int i = 0; i < 88; i++)
    {
        start += htm[i].start;
        abort += htm[i].abort;
        commit += htm[i].commit;
        fallpath += htm[i].fallpath;
        capacity += htm[i].capacity;
        conflict += htm[i].conflict;
        expl += htm[i].expl;
    }

    cout << "HTM start: " << start << endl;
    cout << "HTM abort: " << abort << endl;
    cout << "HTM commit: " << commit << endl;
    cout << "HTM fallpath: " << fallpath << endl;
    cout << "HTM capacity: " << capacity << endl;
    cout << "HTM conflict: " << conflict << endl;
    cout << "HTM explicit: " << expl << endl;
    cout << "HTM code 0: " << abort - (expl + capacity + conflict)<< endl;
}

void HT::printHT()
{
}
