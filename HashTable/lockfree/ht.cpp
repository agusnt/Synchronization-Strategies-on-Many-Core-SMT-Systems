#include "ht.hpp"
#include <omp.h>

/*
 * https://www.microsoft.com/en-us/research/wp-content/uploads/2001/10/2001-disc.pdf
 */

using namespace std;

Node::Node(uint64_t k, uint64_t d, Node *n)
{
    key = k;
    data = d;
    next = n;
}

HT::HT(int s)
{
    size = s;
    array = new Node*[s];
}

inline long HT::is_marked(void *n)
{
    if (!n) return 0;
    return ((uintptr_t) n) & 0x1L;
}

inline long HT::get_unmarked(void *n)
{
    return ((uintptr_t) n) & (~0x1L);
}

inline long HT::get_marked(void *n)
{
    return ((uintptr_t) n) | (0x1L);
}

void HT::add(uint64_t k, uint64_t d)
{
    int index = k % size;

    Node *new_node = new Node(k, d, nullptr);
    Node *right_node, *left_node;

    do
    {
        right_node = get(k, &left_node);

        if (right_node && right_node->key == k) 
        {
            right_node->data = d;
            break;
        }

        new_node->next = right_node;

        Node **place;
        if (!left_node) place = &(array[index]);
        else place = &left_node->next;

        if (__sync_bool_compare_and_swap(place, right_node, new_node))
        {
            break;
        }
    } while(true);
}

void HT::del(uint64_t k)
{
    Node *right_node, *right_node_next, *left_node;
    do
    {
        right_node = get(k, &left_node);
        if (!right_node || right_node->key != k) return;

        right_node_next = right_node->next;
        if (!is_marked(right_node_next))
        {
            if (__sync_bool_compare_and_swap(&right_node->next, right_node_next, 
                        get_marked(right_node_next)))
            {
                break;
            }
        }
    } while(true);

    if (!__sync_bool_compare_and_swap(&left_node->next, right_node, 
                right_node_next))
    {
        right_node = get(k, &left_node);
    }
}

Node * HT::get(uint64_t k)
{
    Node *right_node, *left_node;

    right_node = get(k, &left_node);

    if (right_node && right_node->key != k) return nullptr;

    else return right_node;
}

Node * HT::get(uint64_t k, Node **left_node)
{
    int index = k % size;

search_again:
    do
    {
        Node *left_node_next = nullptr, *right_node = nullptr;
        Node *t = array[index];
        Node *t_next;
        if (t) t_next = t->next;
        else t_next = nullptr;

        do
        {
            if (!is_marked(t_next))
            {
                (*left_node) = t;
                left_node_next = t_next;
            }

            t = (Node *) get_unmarked(t_next);
            if (!t) break;

            t_next = t->next;
        } while(is_marked(t_next) || t->key < k);

        right_node = t;

        if (left_node_next == right_node)
        {
            if (right_node && is_marked(right_node->next)) 
            {
                goto search_again;
            } else 
            {
                return right_node;
            }
        }
        
        if (__sync_bool_compare_and_swap(&(*left_node)->next, left_node_next, right_node))
        {
            if (right_node && is_marked(right_node->next)) goto search_again;
            else return right_node;
        }
    } while(1);
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
