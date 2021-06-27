#include "ht.hpp"

using namespace std;

Node::Node(uint64_t k, uint64_t d, Node *n)
{
    TM_STORE(&key, k);
    TM_STORE(&data, d);
    TM_STORE(&next, n);
}

HT::HT(int s)
{
    size = s;
    array = new Node*[s];
}

void HT::add(uint64_t k, uint64_t d)
{
    int index = k % size;

    Node *idx, *prev;
    TM_START(2, RW);
    idx = (Node *) TM_LOAD(&(array[index]));
    prev = nullptr;

    uint64_t auxK;
    if (idx) auxK = (uint64_t) TM_LOAD(&(idx->key));
    while(idx && auxK < k) 
    {
        Node *aux;
        prev = idx;
        idx = (Node *) TM_LOAD(&(idx->next));
        if (idx) auxK = (uint64_t) TM_LOAD(&(idx->key));
    }

    if (!idx) 
    {
        // New is equal to Malloc + constructor
        Node *aux = (Node *) TM_MALLOC(sizeof(Node));
        TM_STORE(&(aux->key), k);
        TM_STORE(&(aux->data), d);
        TM_STORE(&(aux->next), nullptr);

        if (prev) TM_STORE(&(prev->next), aux);
        else TM_STORE(&(array[index]), aux);
    } else if (auxK == k)
    {
        TM_STORE(&(idx->data), d);
    } else
    {
        Node *aux = (Node *) TM_MALLOC(sizeof(Node));
        TM_STORE(&(aux->key), k);
        TM_STORE(&(aux->data), d);
        TM_STORE(&(aux->next), idx);

        if (prev) TM_STORE(&(prev->next), aux);
        else TM_STORE(&(array[index]), aux);
    }
    TM_COMMIT;
}

void HT::del(uint64_t k)
{
    int index = k % size;

    Node *idx, *prev;
    TM_START(1, RW);
    idx = (Node *) TM_LOAD(&(array[index]));
    prev = nullptr;

    uint64_t auxK;
    if (idx) auxK = (uint64_t) TM_LOAD(&(idx->key));
    while(idx && auxK != k) 
    {
        Node *aux;
        prev = idx;
        idx = (Node *) TM_LOAD(&(idx->next));
        if (idx) auxK = (uint64_t) TM_LOAD(&(idx->key));
    }

    if (idx) 
    {
        // New is equal to Malloc + constructor
        if (prev) 
        {
            TM_STORE(&prev->next, idx->next);
        } else 
        {
            TM_STORE(&array[index], idx->next);
        }
        TM_FREE(idx);
    }
    TM_COMMIT;
}

Node * HT::get(uint64_t k)
{

    int index = k % size;

    Node *idx, *res = nullptr;
    TM_START(0, RO);
    idx = (Node *) TM_LOAD(&(array[index]));

    uint64_t auxK;
    if (idx) auxK = (uint64_t) TM_LOAD(&(idx->key));
    while(idx && auxK != k) 
    {
        Node *aux;
        idx = (Node *) TM_LOAD(&(idx->next));
        if (idx) auxK = (uint64_t) TM_LOAD(&(idx->key));
    }
    if (idx) res = idx;

    TM_COMMIT;
    return res;
}

void HT::printHT()
{
    for (int i = 0; i < size; i++)
    {
        Node *n = array[i];
        while(n)
        {
            cout << n->key << " (" << n->data << ") ";
            n = n->next;
        }
        cout << endl;
    }
}
