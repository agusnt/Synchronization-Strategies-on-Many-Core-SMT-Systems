#include "bst.hpp"

using namespace std;

Node::Node(long k, Node *l, Node *r)
{
    key = k;
    left = l;
    right = r;
}

BST::BST() 
{
    root = nullptr;
}

Node * BST::get(long k)
{
    Node *res;

    TM_START(0, RO);
    Node *aux = (Node *) TM_LOAD(&root);
    while(aux)
    {
        long auxK = (long) TM_LOAD(&(aux->key));
        if (k < auxK) 
        {
            aux = (Node *) TM_LOAD(&(aux->left));
        }
        else if (k > auxK) {
            aux = (Node *) TM_LOAD(&(aux->right));
        }
        else 
        {
            res = aux;
            aux = NULL;
        }
    }

    TM_COMMIT;
    return res;
}

void BST::add(long k)
{
    TM_START(1, RW);
    Node *aux = (Node *) TM_LOAD(&root), *prev = NULL;
    while(aux)
    {
        long auxK = (long) TM_LOAD(&(aux->key));
        if (k < auxK) 
        {
            prev = aux;
            aux = (Node *) TM_LOAD(&aux->left);
        }
        else if (k > auxK) 
        {
            prev = aux;
            aux = (Node *) TM_LOAD(&aux->right);
        }
        else break;
    }
        
    if (!aux)
    {
        Node * tmp = (Node *) TM_MALLOC(sizeof(Node));
        TM_STORE(&tmp->key, k);
        TM_STORE(&tmp->left, nullptr);
        TM_STORE(&tmp->right, nullptr);

        if (prev)
        {
            long auxK = (long) TM_LOAD(&(prev->key));
            if (k < auxK) TM_STORE(&prev->left, tmp);
            else if (k > auxK) TM_STORE(&prev->right, tmp);
        } else 
        {
            TM_STORE(&root, tmp);
        }
    }
    TM_COMMIT;
}

void BST::del(long k)
{
    TM_START(2, RW);
    Node *aux = (Node *) TM_LOAD(&root), *prev = NULL;
    while(aux)
    {
        long auxK = (long) TM_LOAD(&(aux->key));
        if (k < auxK) 
        {
            prev = aux;
            aux = (Node *) TM_LOAD(&aux->left);
        }
        else if (k > auxK) 
        {
            prev = aux;
            aux = (Node *) TM_LOAD(&aux->right);
        }
        else break;
    }
    
    if (aux)
    {
        if (!aux->left)
        {
            Node *tmp = aux;
            Node *auxR = (Node *) TM_LOAD(&(aux->right));
            long auxK = (long) TM_LOAD(&(prev->key));
            if (k < auxK) TM_STORE(&(prev->left), auxR);
            else TM_STORE(&(prev->right), auxR);
            TM_FREE(tmp);
        } else if (!aux->right)
        {
            Node *tmp = aux;
            Node *auxL = (Node *) TM_LOAD(&(aux->left));
            long auxK = (long) TM_LOAD(&(prev->key));
            if (k < auxK) TM_STORE(&(prev->left), auxL);
            else TM_STORE(&(prev->right), auxL);
            TM_FREE(tmp);
        } else
        {
            Node *pprevMin = aux;
            Node *prevMin = (Node *) TM_LOAD(&(aux->right));
            Node *min = (Node *) TM_LOAD(&(prevMin->left));
            while (min) 
            {
                pprevMin = prevMin;
                prevMin = min;
                min = (Node *) TM_LOAD(&(prevMin->left));
            }
            long auxK = (long) TM_LOAD(&(prevMin->key));
            TM_STORE(&(aux->key), auxK);
            
            Node *tmp = prevMin;
            Node *auxR = (Node *)TM_LOAD(&prevMin->right);
            
            long pprevK = (long) TM_LOAD(&(pprevMin->key));
            if (auxK < pprevK) TM_STORE(&(pprevMin->left), auxR);
            else TM_STORE(&(pprevMin->right), auxR);
            TM_FREE(tmp);
        }
    }
    TM_COMMIT;
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
    auxPrintBst(root);
    cout << endl;
}
