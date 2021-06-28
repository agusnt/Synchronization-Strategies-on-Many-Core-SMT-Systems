#include "ht.hpp"

using namespace std;

Node::Node(uint64_t k, uint64_t d, Node *n)
{
    key = k;
    data = d;
    next = n;
    marked = false;
}

Node::~Node()
{
}

HT::HT(int s)
{
    size = s;
    array = new Node*[s];
    for (int i = 0; i < s; i++) array[i] = new Node(0, 0, nullptr);

    // HTM vars to zero
    spinlock.store(false);
}

inline void HT::startHTM()
{
    int retry = 0;
    unsigned status;

    while(true)
    {
        htm[omp_get_thread_num()].start++;
        if ((status = _xbegin()) != _XBEGIN_STARTED) {
            htm[omp_get_thread_num()].abort++;
            if (status & _XABORT_EXPLICIT) 
            {
                /*
                 * Explicit abort because other threads get the lock.
                 */
                if (_XABORT_CODE(status) == 0xFF && !(status & _XABORT_NESTED))
                {
                    htm[omp_get_thread_num()].expl++;
                    while(spinlock.load(std::memory_order_relaxed)) _mm_pause();
                    --retry;
                }
            }

            if (status & _XABORT_CONFLICT)
            {
                htm[omp_get_thread_num()].conflict++;
            } else if(status & _XABORT_CAPACITY) 
            {
                htm[omp_get_thread_num()].capacity++;
                retry = RETRY_LIMIT;
            }
            if (++retry >= RETRY_LIMIT)
            {
                htm[omp_get_thread_num()].fallpath++;
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

#pragma GCC push_options
#pragma GCC optimize("O0")
inline void HT::endHTM()
{
    if (_xtest())
    {
        _xend();
        htm[omp_get_thread_num()].commit++;
    } else
    {
        // Unlock
        spinlock.store(false);
    }
}
#pragma GCC pop_options

void HT::add(uint64_t k, uint64_t d)
{
    int index = k % size;
    bool added = true;

    Node *idx, **prev;
    /*
     * Create the new node to avoid aborts because of: page faults or systems
     * call
     */
    Node *newNode = new Node(k, d, nullptr);

findPos:
    idx = array[index];
    prev = &(idx->next);

    // Search for the position where the new node will be added
    while(idx && idx->key < k)
    {
        prev = &(idx->next);
        idx = idx->next;
    }

    // Start transaction
    startHTM();
    if (!idx)
    {
        // Test that nobody change the pointer before us
        if (*prev == idx) *prev = newNode;
    } else if (idx->key == k) 
    {
        idx->data = d;
        added = false;
    } else
    {
        if (*prev != idx)
        {
            // Someone change the pointer before us, so we have to look again
            // for the position to add the new node
            endHTM();
            goto findPos;
        }
        newNode->next = *prev;
        *prev = newNode;
    }
    // End transaction
    endHTM();

    if (!added) delete newNode;
}

void HT::del(uint64_t k)
{

    int index = k % size;

    Node *idx, **prev; 

findPos:
    idx = array[index]; 
    prev = &(array[index]);

    // Search for the node
    while(idx && idx->key < k) 
    {
        prev = &(idx->next);
        idx = idx->next;
    }

    // Start Transaction
    startHTM();
    if (idx && idx->key == k && !idx->marked) 
    {   
        if (*prev != idx)
        {
            // Someone change the pointer before us, so we have to look again
            // for the position to add the new node
            endHTM();
            goto findPos;
        }
        // Logically remove
        *prev = idx->next;
        idx->marked = true;
    }
    // End Transaction
    endHTM();

    // The delete must be out of the transactional section to avoid aborts
    //if (idx && idx->key == k) delete idx;
}

bool HT::get(uint64_t k)
{
    int index = k % size;
    Node *idx;
    bool res = false; 

    //startHTM();
    idx = array[index]; 

    while(idx && idx->key < k) idx = idx->next;
    if (idx && idx->key == k && !idx->marked) res = true;

    //endHTM();

    return res;
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
}

void HT::printHT()
{
    long long total = 0;
    for (int i = 0; i < size; i++)
    {
        Node *n = array[i];
        while(n)
        {
            cout << n->key << " ";
            n = n->next;
            total++;
        }
        cout << endl;
    }
    cout << "Total: " << total << endl;
}
