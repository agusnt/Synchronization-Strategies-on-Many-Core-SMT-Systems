#include "bst.hpp"

using namespace std;

Node::Node(long k, Node *l, Node *r)
{
    key = k;
    left = l;
    right = r;
    marked.store(false);
}

Node::~Node()
{
}

BST::BST() 
{
    // We must have at least one node
    root = new Node(LONG_MAX, nullptr, nullptr);
}

BST::~BST()
{
}

bool BST::get(long k)
{
    Node *x = root;
    x = getNext(x, k);
    // Search node
    while (x && x->key != k) x = getNext(x, k);

    // Key found and node is not marked to be delete
    return (x && !x->marked.load());
}

Node * BST::getNext(Node *x, long k)
{
    // Get next element of the node
    if (k < x->key) return x->left;
    else if (k > x->key) return x->right;
    else return x;
}

void BST::add(long k)
{
    while(true)
    {
        Node *x = root, *next;
        next = getNext(x, k);
        while(next && next->key != k)
        {
            // Search node
            x = next;
            next = getNext(x, k);
        }

        // Node find but is marked to be deleted
        if (next && next->marked.load()) continue;
        
        // Acquire lock
        x->lock.lock();
        if (!x->marked.load())
        {
            // Mark node to be delete
            Node *newNode = new Node(k, nullptr, nullptr);
            // Fix pointers
            if (k < x->key) x->left = newNode;
            else x->right = newNode;

            // Release lock and return
            x->lock.unlock();
            return ;
        }
        // Release lock
        x->lock.unlock();
    }
}

void BST::del(long k)
{
    while(true)
    {
        Node *x = root, *next;
        next = getNext(x, k);
        while(next && next->key != k)
        {
            // Search node
            x = next;
            next = getNext(x, k);
        }

        // This node is already logically removed
        if (!next || next->marked.load()) return;
        
        // Lock nodes
        x->lock.lock();
        next->lock.lock();
        if (!x->marked.load())
        {
            // Logically remove the node
            next->marked.store(true);
            if (!next->left && !next->right)
            {
                // Case 1: No childs
                if (x->left == next) x->left = nullptr;
                else x->right = nullptr;
            } else if (!next->left && next->right)
            {
                // Case 2: One child right
                if (x->left == next) x->left = next->right;
                else x->right = next->right;
            } else if (next->left && !next->right)
            {
                // Case 3: One child left
                if (x->left == next) x->left = next->left;
                else x->right = next->left;
            } else
            {
                // Case 4: Both child
                vector<mutex *> vmtx;
                Node **min = &(next->right);

                (*min)->lock.lock();
                vmtx.push_back(&(*min)->lock);

                while((*min)->left)
                {
                    min = &(*min)->left;
                    (*min)->lock.lock();
                    vmtx.push_back(&(*min)->lock);
                }

                if ((*min)->marked.load())
                {
                    // Min is logically remove
                    for (auto i: vmtx) i->unlock();
                    next->lock.unlock();
                    x->lock.unlock();
                    continue;
                }

                next->key = (*min)->key;
                next->marked.store(false);

                if (!(*min)->marked.load()) 
                {
                    (*min) = (*min)->right;
                }

                for (auto i: vmtx) i->unlock();
            }

            // Logically remove complete
            next->lock.unlock();
            x->lock.unlock();
            return;
        }
        next->lock.unlock();
        x->lock.unlock();
    }
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

void BST::printBST()
{
    auxPrintBst(root->left);
    cout << endl;
}
