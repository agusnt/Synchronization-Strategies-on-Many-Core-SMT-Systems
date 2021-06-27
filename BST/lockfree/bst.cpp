#include "bst.hpp"

/*
 * Implementation from:
 *
 * https://dl.acm.org/doi/pdf/10.1145/2312005.2312036
 */

using namespace std;

void *FLAG(void *ptr, int state)
{
    ptr = (void *) ((uintptr_t)ptr | (uintptr_t)state);
    return ptr;
}

int GETFLAG(void *ptr)
{
    int flag = ((uintptr_t) ptr & (uintptr_t)0x3);
    return flag;
}

void *UNFLAG(void *ptr)
{
    ptr = (void *)((uintptr_t)ptr & ~(uintptr_t)0x3);
    return ptr;
}

void *SETNULL(void *ptr)
{
    ptr = (void *) ((uintptr_t)ptr | (uintptr_t)0x1);
    return ptr;
}

bool ISNULL(void *ptr)
{
    int val = ((uintptr_t)ptr & (uintptr_t)0x1);
    if(val == 1) {
        return true;
    }
    return false;
}

Node::Node(long k, Node *l, Node *r)
{
    key = k;
    op = nullptr;
    if (!l) left = (Node *) SETNULL(nullptr);
    else left = l;
    if (!r) right = (Node *) SETNULL(nullptr);
    else right = r;
}

Node::~Node()
{
}

BST::BST() 
{
    root = nullptr;
}

BST::~BST()
{
}

ChildCASOp::ChildCASOp(bool l, Node *e, Node *u)
{
    isLeft = l;
    expected = e;
    update = u;
}

RelocateOp::RelocateOp(Node *curr, Operation *currOp, long k, long rk)
{
    dest = curr;
    destOp = currOp;
    removeKey = k;
    replaceKey = rk;
    state = ONGOING;
}

void BST::help(Node *pred, Operation *predOp, Node *curr, Operation *currOp)
{
    if (GETFLAG(currOp) == CASCHILD) 
        helpChildCAS((ChildCASOp *) UNFLAG((void *) currOp), curr);
    else if (GETFLAG(currOp) == REALOCATE) 
        helpRelocate((RelocateOp *) UNFLAG((void *) currOp), pred, predOp, curr);
    else if (GETFLAG(currOp) == MARK) 
        helpMarked(pred, predOp, curr);
}

bool BST::helpRelocate(RelocateOp *op, Node *pred, Operation *predOp, Node *curr)
{
    int seenState = op->state;
    if (seenState == ONGOING)
    {
        Operation *seenOp = VCAS(&op->dest->op, op->destOp, FLAG(op, REALOCATE));
        if ((seenOp == op->destOp) || (seenOp == FLAG(op, REALOCATE)))
        {
           CAS(&op->state, ONGOING, SUCCESFUL);
           seenState = SUCCESFUL;
        } else 
        {
           seenState = VCAS(&op->state, ONGOING, FAILED);
        }
    }
    if (seenState == SUCCESFUL)
    {
        CAS(&op->dest->key, op->removeKey, op->replaceKey);
        CAS(&op->dest->op, FLAG(op, REALOCATE), FLAG(op, NONE));
    }
    bool result = (seenState == SUCCESFUL);

    if (op->dest == curr) return result;
    CAS(&curr->op, FLAG(op, REALOCATE), FLAG(op, result ? MARK : NONE));
    if (result)
    {
        if (op->dest == pred) predOp = (Operation *) FLAG((void *)op, NONE);
        helpMarked(pred, predOp, curr);
    }
    return result;
}

void BST::helpMarked(Node *pred, Operation *predOp, Node *curr)
{
    Node *newRef;
    if (ISNULL(curr->left))
    {
        if (ISNULL(curr->right))
        {
            newRef = (Node *) SETNULL(curr);
        }
        else
        {
            newRef = curr->right;
        }
    } else
    {
        newRef = curr->left;
    }

    ChildCASOp* casOp = new ChildCASOp(curr == pred->left, curr, newRef);

    if (CAS(&pred->op, predOp, FLAG(casOp, CASCHILD))) 
    {
        helpChildCAS(casOp, pred);
    } else
    {
        delete casOp;
    }
}

void BST::helpChildCAS(ChildCASOp *op, Node *dest)
{
    Node ** address = op->isLeft ? &dest->left : &dest->right;
    CAS(address, op->expected, op->update);
    CAS(&dest->op, FLAG(op, CASCHILD), FLAG(op, NONE));
}

int BST::getAux(long k, Node *& pred, Operation *& predOp, Node*& curr, 
        Operation *& currOp, Node *auxRoot)
{
    int result, currKey;
    Node *next, *lastRight;
    Operation *lastRightOp;

retry:
    result = NOTFOUND_R;
    curr = auxRoot;
    currOp = curr->op;
    if (GETFLAG(currOp) != NONE)
    {
        if (auxRoot == root)
        {
            helpChildCAS((ChildCASOp *) UNFLAG((void *)currOp), curr);
            goto retry;
        } else return ABORT; 
    }

    next = curr->right;
    lastRight = curr;
    lastRightOp = currOp;

    while(!ISNULL(next) && next)
    {
        pred = curr;
        predOp = currOp;
        curr = next;
        currOp = curr->op;

        if (GETFLAG(currOp) != NONE)
        {
            help(pred, predOp, curr, currOp);
            goto retry;
        }

        currKey = curr->key;
        if (k < currKey)
        {
            result = NOTFOUND_L;
            next = curr->left;
        } else if(k > currKey)
        {
            result = NOTFOUND_R;
            next = curr->right;
            lastRight = curr;
            lastRightOp = currOp;
        } else
        {
            result = FOUND; 
            break;
        }
    }
    if ((result != FOUND) && (lastRightOp != lastRight->op)) goto retry;
    if (curr->op != currOp) goto retry;
    return result;
}

int BST::get(long k)
{
    Node *pred, *curr;
    Operation *predOp, *currOp;
    return getAux(k, pred, predOp, curr, currOp, root);
}

void BST::add(long k)
{
    Node *pred, *curr, *newNode;
    Operation* predOp, *currOp;
    ChildCASOp *casOp;
    int result;

    while(true)
    {
        if (root)
        {
            result = getAux(k, pred, predOp, curr, currOp, root);
            if (result == FOUND) return;
            newNode = new Node(k, nullptr, nullptr);

            bool isLeft = (result == NOTFOUND_L);

            Node *old = isLeft ? curr->left : curr->right;

            casOp = new ChildCASOp(isLeft, old, newNode);

            if (CAS(&curr->op, currOp, FLAG(casOp, CASCHILD)))
            {
                helpChildCAS(casOp, curr);
                return;
            }
        } else
        {
            newNode = new Node(k, nullptr, nullptr);
            if (CAS(&root, nullptr, newNode)) return;
        }
    }
}

void BST::del(long k)
{
    Node *pred, *curr, *replace;

    Operation *predOp, *currOp, *replaceOp; 
    RelocateOp *relocOp;

    while (true)
    {
        if (getAux(k, pred, predOp, curr, currOp, root) != FOUND) return;

        if (ISNULL(curr->right) || ISNULL(curr->left))
        {
            if (CAS(&curr->op, currOp, FLAG(currOp, MARK)))
            {
                helpMarked(pred, predOp, curr);
                return;
            }
        } else{

            if ((getAux(k, pred, predOp, replace, replaceOp, curr) == ABORT)
                    || (curr->op != currOp)) 
            {
                continue;
            }

            relocOp = new RelocateOp(curr, currOp, k, replace->key);

            if (CAS(&replace->op, replaceOp, FLAG(relocOp, REALOCATE)))
            {
                if (helpRelocate(relocOp, pred, predOp, replace)) return;
            }
        }
    }
}

void BST::auxPrintBst(Node *aux)
{
    if (!ISNULL(aux))
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
