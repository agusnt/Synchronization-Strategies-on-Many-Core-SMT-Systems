#include "bst.hpp"

using namespace std;

Node::Node(long k, Node *l, Node *r)
{
    key = k;
    left = l;
    right = r;
    marked = false;
}

Node::~Node()
{
}

BST::BST() 
{
    root = new Node(INT_MAX, nullptr, nullptr);

    // HTM vars
    spinlock.store(false);
}

BST::~BST()
{
}

inline void BST::startHTM()
{
    int retry = 0;
    unsigned status;
    while(true)
    {
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
            } else if (status & _XABORT_CONFLICT)
            {
                htm[omp_get_thread_num()].conflict++;
            } else if (status & _XABORT_CAPACITY) 
            {
                htm[omp_get_thread_num()].capacity++;
                retry = limit;
            }
            if (++retry >= limit)
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

inline void BST::endHTM()
{
    if (_xtest())
    {
        // Can start the transaction, test the spinlock
        _xend();
        htm[omp_get_thread_num()].commit++;
    } else
    {
        // Unlock
        spinlock.store(false);
    }
}

Node * BST::get(long k)
{
    Node *curr;

    // Start transaction
    //startHTM();

    curr = root->left;

    while(curr)
    {
        if (k < curr->key) curr = curr->left;
        else if (k > curr->key) curr = curr->right;
        else break;
    }

    // End transaction
    //endHTM();
    return curr;
}

void BST::add(long k)
{
    // New nodes in trasaction cause aborts (System call)
    Node *curr, *pred, *newNode = new Node(k, nullptr, nullptr);

    // Start transaction
findPos:
    bool find = false;
    pred = root;
    curr = root->left;

    while(curr)
    {
        if (k < curr->key) 
        {
            pred = curr;
            curr = curr->left;
        }
        else if (k > curr->key) 
        {
            pred = curr;
            curr = curr->right;
        }
        else 
        {
            find = true;
            break;
        }
    }

    // Start transaction
    startHTM();
    if (!curr) 
    {
        if (pred->marked)
        {
            // Previous node was logically remove
            endHTM();
            goto findPos;
        }
        if (k < pred->key) 
        {
            if (pred->left)
            {
                // Other threads add a new node in this position, so we will
                // to end the transaction and look again for the node
                endHTM();
                goto findPos;
            }
            pred->left = newNode;
        } else 
        {
            if (pred->right)
            {
                // Other threads add a new node in this position, so we will
                // to end the transaction and look again for the node
                endHTM();
                goto findPos;
            }
            pred->right = newNode;
        }
    }
    // End transaction
    endHTM();

    if (find) delete newNode;
}

void BST::del(long k)
{
    Node *curr, *pred, *fnode;
    fnode = nullptr;

    // Start transaction

findPos:
    pred = root;
    curr = root->left;

    // Find the key
    while(curr)
    {
        if (k < curr->key) 
        {
            pred = curr;
            curr = curr->left;
        } else if (k > curr->key) 
        {
            pred = curr;
            curr = curr->right;
        } else 
        {
            fnode = curr;
            break;
        }
    }
    if (fnode == nullptr) return;

    // Start transaction
    startHTM();
    if (fnode && !fnode->marked && curr) {
        if (pred->marked)
        {
            // Previous node was logically remove
            endHTM();
            goto findPos;
        }

        curr->marked = true;
        if (!curr->left)
        {
            if (pred->left == curr) pred->left = curr->right;
            else pred->right = curr->right;
        } else if (!curr->right)
        {
            if (pred->left == curr) pred->left = curr->left;
            else pred->right = curr->left;
        } else
        {
            Node *min;

            min = curr->right;
            pred = curr;

            while (min->left) 
            {
                pred = min;
                min = min->left;
            }

            if (min->marked) 
            {
                // Min node is marked to be logically remove
                endHTM();
                goto findPos;
            }

            curr->key = min->key;
            curr->marked = false;
            min->marked = true;
            
            fnode = min;

            if (min->right && min->right->marked) 
            {
                // Min right node is marked to be logically remove
                endHTM();
                goto findPos;
            }
            if (pred->right == min) pred->right = min->right;
            else pred->left = min->right;
        }
    }
    // End transaction
    endHTM();

    // Deletion in transaction cause aborts
    //if (find) delete fnode;
}

void BST::auxPrintBst(Node *aux)
{
    if (aux)
    {
        auxPrintBst(aux->left);
        cout << aux->key << " ";
        auxPrintBst(aux->right);
    }
}

void BST::printHTM()
{
    uint64_t abort = 0;
    uint64_t commit = 0;
    uint64_t fallpath = 0;
    uint64_t capacity = 0;
    uint64_t conflict = 0;
    uint64_t expl = 0;
    for (int i = 0; i < 88; i++)
    {
        abort += htm[i].abort;
        commit += htm[i].commit;
        fallpath += htm[i].fallpath;
        capacity += htm[i].capacity;
        conflict += htm[i].conflict;
        expl += htm[i].expl;
    }

    cout << "HTM abort: " << abort << endl;
    cout << "HTM commit: " << commit << endl;
    cout << "HTM fallpath: " << fallpath << endl;
    cout << "HTM capacity: " << capacity << endl;
    cout << "HTM conflict: " << conflict << endl;
    cout << "HTM explicit: " << expl << endl;
}

void BST::printBST()
{
    auxPrintBst(root->left);
    cout << endl;
}
