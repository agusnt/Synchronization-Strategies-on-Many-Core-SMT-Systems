#include "ht.hpp"

#include <omp.h>

/*
 * Source:
 * https://people.csail.mit.edu/shanir/publications/Lazy_Concurrent.pdf
 */

using namespace std;

Node::Node(uint64_t k, uint64_t d, Node *n)
{
    key = k;
    data = d;
    next = n;
    marked.store(false);
}

HT::HT(int s)
{
    size = s;
    array = new Node*[s];
    for (int i = 0; i < s; i++) array[i] = new Node(INT_MIN, INT_MIN, nullptr);
}

bool HT::validate(Node *pred, Node *curr)
{
    return (!pred->marked.load() && !curr) 
        || (!pred->marked.load() && !curr->marked.load() && pred->next == curr);
}

void HT::add(uint64_t k, uint64_t d)
{
    int index = k % size;
    bool added = false;

    while (true)
    {
        Node *pred = array[index];
        Node *curr = array[index]->next;

        while (curr && curr->key < k)
        {
            // Find the key
            pred = curr;
            curr = curr->next;
        }


        pred->lock.lock();
        // First try
        if (curr) curr->lock.lock();
        // Second try
        if (validate(pred, curr))
        {
            if (curr && curr->key == k) 
            {
                added = true;
            }
            else 
            {
                Node *newNode = new Node(k, d, curr);
                pred->next = newNode;

                added = true;
            }
        }

        if (curr) curr->lock.unlock();
        pred->lock.unlock();
        if (added) return;
    }
}

void HT::del(uint64_t k)
{
    int index = k % size;
    bool remove = false;
    
    while(true)
    {
        Node *pred = array[index];
        Node *curr = array[index]->next;

        while (curr && curr->key < k)
        {
            // Find the key
            pred = curr;
            curr = curr->next;
        }

        if (!curr) return;
        
        pred->lock.lock();
        // First try
        curr->lock.lock();
        // Second try
        if (validate(pred, curr))
        {
            if (!curr || curr->key != k) remove = true;
            else
            {
                curr->marked.store(true); // Logical remove
                pred->next = curr->next;
                remove = true;
                //delete curr;
            }
        }
        curr->lock.unlock();
        pred->lock.unlock();
        if (remove) return;
    }
}

bool HT::get(uint64_t k)
{
    int index = k % size;

    Node *curr = array[index]->next; 
    while (curr && curr->key < k) curr = curr->next;
    return curr && curr->key == k && !curr->marked.load();
}

void HT::printHT()
{
    long total = 0;
    for (int i = 0; i < size; i++)
    {
        Node *n = array[i]->next;
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

