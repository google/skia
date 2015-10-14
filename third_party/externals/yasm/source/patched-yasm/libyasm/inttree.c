#include "util.h"

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include "coretype.h"
#include "inttree.h"

#define VERIFY(condition) \
if (!(condition)) { \
fprintf(stderr, "Assumption \"%s\"\nFailed in file %s: at line:%i\n", \
#condition,__FILE__,__LINE__); \
abort();}

/*#define DEBUG_ASSERT 1*/

#ifdef DEBUG_ASSERT
static void Assert(int assertion, const char *error)
{
    if (!assertion) {
        fprintf(stderr, "Assertion Failed: %s\n", error);
        abort();
    }
}
#endif

/* If the symbol CHECK_INTERVAL_TREE_ASSUMPTIONS is defined then the
 * code does a lot of extra checking to make sure certain assumptions
 * are satisfied.  This only needs to be done if you suspect bugs are
 * present or if you make significant changes and want to make sure
 * your changes didn't mess anything up.
 */
/*#define CHECK_INTERVAL_TREE_ASSUMPTIONS 1*/

static IntervalTreeNode *ITN_create(long low, long high, void *data);

static void LeftRotate(IntervalTree *, IntervalTreeNode *);
static void RightRotate(IntervalTree *, IntervalTreeNode *);
static void TreeInsertHelp(IntervalTree *, IntervalTreeNode *);
static void TreePrintHelper(const IntervalTree *, IntervalTreeNode *);
static void FixUpMaxHigh(IntervalTree *, IntervalTreeNode *);
static void DeleteFixUp(IntervalTree *, IntervalTreeNode *);
#ifdef CHECK_INTERVAL_TREE_ASSUMPTIONS
static void CheckMaxHighFields(const IntervalTree *, IntervalTreeNode *);
static int CheckMaxHighFieldsHelper(const IntervalTree *, IntervalTreeNode *y, 
                                    const int currentHigh, int match);
static void IT_CheckAssumptions(const IntervalTree *);
#endif

/* define a function to find the maximum of two objects. */
#define ITMax(a, b) ( (a > b) ? a : b )

IntervalTreeNode *
ITN_create(long low, long high, void *data)
{
    IntervalTreeNode *itn = yasm_xmalloc(sizeof(IntervalTreeNode));
    itn->data = data;
    if (low < high) {
        itn->low = low;
        itn->high = high;
    } else {
        itn->low = high;
        itn->high = low;
    }
    itn->maxHigh = high;
    return itn;
}

IntervalTree *
IT_create(void)
{
    IntervalTree *it = yasm_xmalloc(sizeof(IntervalTree));

    it->nil = ITN_create(LONG_MIN, LONG_MIN, NULL);
    it->nil->left = it->nil;
    it->nil->right = it->nil;
    it->nil->parent = it->nil;
    it->nil->red = 0;
  
    it->root = ITN_create(LONG_MAX, LONG_MAX, NULL);
    it->root->left = it->nil;
    it->root->right = it->nil;
    it->root->parent = it->nil;
    it->root->red = 0;

    /* the following are used for the Enumerate function */
    it->recursionNodeStackSize = 128;
    it->recursionNodeStack = (it_recursion_node *) 
        yasm_xmalloc(it->recursionNodeStackSize*sizeof(it_recursion_node));
    it->recursionNodeStackTop = 1;
    it->recursionNodeStack[0].start_node = NULL;

    return it;
}

/***********************************************************************/
/*  FUNCTION:  LeftRotate */
/**/
/*  INPUTS:  the node to rotate on */
/**/
/*  OUTPUT:  None */
/**/
/*  Modifies Input: this, x */
/**/
/*  EFFECTS:  Rotates as described in _Introduction_To_Algorithms by */
/*            Cormen, Leiserson, Rivest (Chapter 14).  Basically this */
/*            makes the parent of x be to the left of x, x the parent of */
/*            its parent before the rotation and fixes other pointers */
/*            accordingly. Also updates the maxHigh fields of x and y */
/*            after rotation. */
/***********************************************************************/

static void
LeftRotate(IntervalTree *it, IntervalTreeNode *x)
{
    IntervalTreeNode *y;
 
    /* I originally wrote this function to use the sentinel for
     * nil to avoid checking for nil.  However this introduces a
     * very subtle bug because sometimes this function modifies
     * the parent pointer of nil.  This can be a problem if a
     * function which calls LeftRotate also uses the nil sentinel
     * and expects the nil sentinel's parent pointer to be unchanged
     * after calling this function.  For example, when DeleteFixUP
     * calls LeftRotate it expects the parent pointer of nil to be
     * unchanged.
     */

    y=x->right;
    x->right=y->left;

    if (y->left != it->nil)
        y->left->parent=x; /* used to use sentinel here */
    /* and do an unconditional assignment instead of testing for nil */
  
    y->parent=x->parent;   

    /* Instead of checking if x->parent is the root as in the book, we
     * count on the root sentinel to implicitly take care of this case
     */
    if (x == x->parent->left)
        x->parent->left=y;
    else
        x->parent->right=y;
    y->left=x;
    x->parent=y;

    x->maxHigh=ITMax(x->left->maxHigh,ITMax(x->right->maxHigh,x->high));
    y->maxHigh=ITMax(x->maxHigh,ITMax(y->right->maxHigh,y->high));
#ifdef CHECK_INTERVAL_TREE_ASSUMPTIONS
    IT_CheckAssumptions(it);
#elif defined(DEBUG_ASSERT)
    Assert(!it->nil->red,"nil not red in ITLeftRotate");
    Assert((it->nil->maxHigh=LONG_MIN),
           "nil->maxHigh != LONG_MIN in ITLeftRotate");
#endif
}


/***********************************************************************/
/*  FUNCTION:  RightRotate */
/**/
/*  INPUTS:  node to rotate on */
/**/
/*  OUTPUT:  None */
/**/
/*  Modifies Input?: this, y */
/**/
/*  EFFECTS:  Rotates as described in _Introduction_To_Algorithms by */
/*            Cormen, Leiserson, Rivest (Chapter 14).  Basically this */
/*            makes the parent of x be to the left of x, x the parent of */
/*            its parent before the rotation and fixes other pointers */
/*            accordingly. Also updates the maxHigh fields of x and y */
/*            after rotation. */
/***********************************************************************/


static void
RightRotate(IntervalTree *it, IntervalTreeNode *y)
{
    IntervalTreeNode *x;

    /* I originally wrote this function to use the sentinel for
     * nil to avoid checking for nil.  However this introduces a
     * very subtle bug because sometimes this function modifies
     * the parent pointer of nil.  This can be a problem if a
     * function which calls LeftRotate also uses the nil sentinel
     * and expects the nil sentinel's parent pointer to be unchanged
     * after calling this function.  For example, when DeleteFixUP
     * calls LeftRotate it expects the parent pointer of nil to be
     * unchanged.
     */

    x=y->left;
    y->left=x->right;

    if (it->nil != x->right)
        x->right->parent=y; /*used to use sentinel here */
    /* and do an unconditional assignment instead of testing for nil */

    /* Instead of checking if x->parent is the root as in the book, we
     * count on the root sentinel to implicitly take care of this case
     */
    x->parent=y->parent;
    if (y == y->parent->left)
        y->parent->left=x;
    else
        y->parent->right=x;
    x->right=y;
    y->parent=x;

    y->maxHigh=ITMax(y->left->maxHigh,ITMax(y->right->maxHigh,y->high));
    x->maxHigh=ITMax(x->left->maxHigh,ITMax(y->maxHigh,x->high));
#ifdef CHECK_INTERVAL_TREE_ASSUMPTIONS
    IT_CheckAssumptions(it);
#elif defined(DEBUG_ASSERT)
    Assert(!it->nil->red,"nil not red in ITRightRotate");
    Assert((it->nil->maxHigh=LONG_MIN),
           "nil->maxHigh != LONG_MIN in ITRightRotate");
#endif
}

/***********************************************************************/
/*  FUNCTION:  TreeInsertHelp  */
/**/
/*  INPUTS:  z is the node to insert */
/**/
/*  OUTPUT:  none */
/**/
/*  Modifies Input:  this, z */
/**/
/*  EFFECTS:  Inserts z into the tree as if it were a regular binary tree */
/*            using the algorithm described in _Introduction_To_Algorithms_ */
/*            by Cormen et al.  This funciton is only intended to be called */
/*            by the InsertTree function and not by the user */
/***********************************************************************/

static void
TreeInsertHelp(IntervalTree *it, IntervalTreeNode *z)
{
    /*  This function should only be called by InsertITTree (see above) */
    IntervalTreeNode* x;
    IntervalTreeNode* y;
    
    z->left=z->right=it->nil;
    y=it->root;
    x=it->root->left;
    while( x != it->nil) {
        y=x;
        if (x->low > z->low)
            x=x->left;
        else /* x->low <= z->low */
            x=x->right;
    }
    z->parent=y;
    if ((y == it->root) || (y->low > z->low))
        y->left=z;
    else
        y->right=z;

#if defined(DEBUG_ASSERT)
    Assert(!it->nil->red,"nil not red in ITTreeInsertHelp");
    Assert((it->nil->maxHigh=INT_MIN),
           "nil->maxHigh != INT_MIN in ITTreeInsertHelp");
#endif
}


/***********************************************************************/
/*  FUNCTION:  FixUpMaxHigh  */
/**/
/*  INPUTS:  x is the node to start from*/
/**/
/*  OUTPUT:  none */
/**/
/*  Modifies Input:  this */
/**/
/*  EFFECTS:  Travels up to the root fixing the maxHigh fields after */
/*            an insertion or deletion */
/***********************************************************************/

static void
FixUpMaxHigh(IntervalTree *it, IntervalTreeNode *x)
{
    while(x != it->root) {
        x->maxHigh=ITMax(x->high,ITMax(x->left->maxHigh,x->right->maxHigh));
        x=x->parent;
    }
#ifdef CHECK_INTERVAL_TREE_ASSUMPTIONS
    IT_CheckAssumptions(it);
#endif
}

/*  Before calling InsertNode  the node x should have its key set */

/***********************************************************************/
/*  FUNCTION:  InsertNode */
/**/
/*  INPUTS:  newInterval is the interval to insert*/
/**/
/*  OUTPUT:  This function returns a pointer to the newly inserted node */
/*           which is guarunteed to be valid until this node is deleted. */
/*           What this means is if another data structure stores this */
/*           pointer then the tree does not need to be searched when this */
/*           is to be deleted. */
/**/
/*  Modifies Input: tree */
/**/
/*  EFFECTS:  Creates a node node which contains the appropriate key and */
/*            info pointers and inserts it into the tree. */
/***********************************************************************/

IntervalTreeNode *
IT_insert(IntervalTree *it, long low, long high, void *data)
{
    IntervalTreeNode *x, *y, *newNode;

    x = ITN_create(low, high, data);
    TreeInsertHelp(it, x);
    FixUpMaxHigh(it, x->parent);
    newNode = x;
    x->red=1;
    while(x->parent->red) { /* use sentinel instead of checking for root */
        if (x->parent == x->parent->parent->left) {
            y=x->parent->parent->right;
            if (y->red) {
                x->parent->red=0;
                y->red=0;
                x->parent->parent->red=1;
                x=x->parent->parent;
            } else {
                if (x == x->parent->right) {
                    x=x->parent;
                    LeftRotate(it, x);
                }
                x->parent->red=0;
                x->parent->parent->red=1;
                RightRotate(it, x->parent->parent);
            } 
        } else { /* case for x->parent == x->parent->parent->right */
             /* this part is just like the section above with */
             /* left and right interchanged */
            y=x->parent->parent->left;
            if (y->red) {
                x->parent->red=0;
                y->red=0;
                x->parent->parent->red=1;
                x=x->parent->parent;
            } else {
                if (x == x->parent->left) {
                    x=x->parent;
                    RightRotate(it, x);
                }
                x->parent->red=0;
                x->parent->parent->red=1;
                LeftRotate(it, x->parent->parent);
            } 
        }
    }
    it->root->left->red=0;

#ifdef CHECK_INTERVAL_TREE_ASSUMPTIONS
    IT_CheckAssumptions(it);
#elif defined(DEBUG_ASSERT)
    Assert(!it->nil->red,"nil not red in ITTreeInsert");
    Assert(!it->root->red,"root not red in ITTreeInsert");
    Assert((it->nil->maxHigh=LONG_MIN),
           "nil->maxHigh != LONG_MIN in ITTreeInsert");
#endif
    return newNode;
}

/***********************************************************************/
/*  FUNCTION:  GetSuccessorOf  */
/**/
/*    INPUTS:  x is the node we want the succesor of */
/**/
/*    OUTPUT:  This function returns the successor of x or NULL if no */
/*             successor exists. */
/**/
/*    Modifies Input: none */
/**/
/*    Note:  uses the algorithm in _Introduction_To_Algorithms_ */
/***********************************************************************/
  
IntervalTreeNode *
IT_get_successor(const IntervalTree *it, IntervalTreeNode *x)
{ 
    IntervalTreeNode *y;

    if (it->nil != (y = x->right)) { /* assignment to y is intentional */
        while(y->left != it->nil) /* returns the minium of the right subtree of x */
            y=y->left;
        return y;
    } else {
        y=x->parent;
        while(x == y->right) { /* sentinel used instead of checking for nil */
            x=y;
            y=y->parent;
        }
        if (y == it->root)
            return(it->nil);
        return y;
    }
}

/***********************************************************************/
/*  FUNCTION:  GetPredecessorOf  */
/**/
/*    INPUTS:  x is the node to get predecessor of */
/**/
/*    OUTPUT:  This function returns the predecessor of x or NULL if no */
/*             predecessor exists. */
/**/
/*    Modifies Input: none */
/**/
/*    Note:  uses the algorithm in _Introduction_To_Algorithms_ */
/***********************************************************************/

IntervalTreeNode *
IT_get_predecessor(const IntervalTree *it, IntervalTreeNode *x)
{
    IntervalTreeNode *y;

    if (it->nil != (y = x->left)) { /* assignment to y is intentional */
        while(y->right != it->nil) /* returns the maximum of the left subtree of x */
            y=y->right;
        return y;
    } else {
        y=x->parent;
        while(x == y->left) { 
            if (y == it->root)
                return(it->nil); 
            x=y;
            y=y->parent;
        }
        return y;
    }
}

/***********************************************************************/
/*  FUNCTION:  Print */
/**/
/*    INPUTS:  none */
/**/
/*    OUTPUT:  none  */
/**/
/*    EFFECTS:  This function recursively prints the nodes of the tree */
/*              inorder. */
/**/
/*    Modifies Input: none */
/**/
/*    Note:    This function should only be called from ITTreePrint */
/***********************************************************************/

static void
ITN_print(const IntervalTreeNode *itn, IntervalTreeNode *nil,
          IntervalTreeNode *root)
{
    printf(", l=%li, h=%li, mH=%li", itn->low, itn->high, itn->maxHigh);
    printf("  l->low=");
    if (itn->left == nil)
        printf("NULL");
    else
        printf("%li", itn->left->low);
    printf("  r->low=");
    if (itn->right == nil)
        printf("NULL");
    else
        printf("%li", itn->right->low);
    printf("  p->low=");
    if (itn->parent == root)
        printf("NULL");
    else
        printf("%li", itn->parent->low);
    printf("  red=%i\n", itn->red);
}

static void
TreePrintHelper(const IntervalTree *it, IntervalTreeNode *x)
{
    if (x != it->nil) {
        TreePrintHelper(it, x->left);
        ITN_print(x, it->nil, it->root);
        TreePrintHelper(it, x->right);
    }
}

void
IT_destroy(IntervalTree *it)
{
    IntervalTreeNode *x = it->root->left;
    SLIST_HEAD(node_head, nodeent)
        stuffToFree = SLIST_HEAD_INITIALIZER(stuffToFree);
    struct nodeent {
        SLIST_ENTRY(nodeent) link;
        struct IntervalTreeNode *node;
    } *np;

    if (x != it->nil) {
        if (x->left != it->nil) {
            np = yasm_xmalloc(sizeof(struct nodeent));
            np->node = x->left;
            SLIST_INSERT_HEAD(&stuffToFree, np, link);
        }
        if (x->right != it->nil) {
            np = yasm_xmalloc(sizeof(struct nodeent));
            np->node = x->right;
            SLIST_INSERT_HEAD(&stuffToFree, np, link);
        }
        yasm_xfree(x);
        while (!SLIST_EMPTY(&stuffToFree)) {
            np = SLIST_FIRST(&stuffToFree);
            x = np->node;
            SLIST_REMOVE_HEAD(&stuffToFree, link);
            yasm_xfree(np);

            if (x->left != it->nil) {
                np = yasm_xmalloc(sizeof(struct nodeent));
                np->node = x->left;
                SLIST_INSERT_HEAD(&stuffToFree, np, link);
            }
            if (x->right != it->nil) {
                np = yasm_xmalloc(sizeof(struct nodeent));
                np->node = x->right;
                SLIST_INSERT_HEAD(&stuffToFree, np, link);
            }
            yasm_xfree(x);
        }
    }

    yasm_xfree(it->nil);
    yasm_xfree(it->root);
    yasm_xfree(it->recursionNodeStack);
    yasm_xfree(it);
}


/***********************************************************************/
/*  FUNCTION:  Print */
/**/
/*    INPUTS:  none */
/**/
/*    OUTPUT:  none */
/**/
/*    EFFECT:  This function recursively prints the nodes of the tree */
/*             inorder. */
/**/
/*    Modifies Input: none */
/**/
/***********************************************************************/

void
IT_print(const IntervalTree *it)
{
    TreePrintHelper(it, it->root->left);
}

/***********************************************************************/
/*  FUNCTION:  DeleteFixUp */
/**/
/*    INPUTS:  x is the child of the spliced */
/*             out node in DeleteNode. */
/**/
/*    OUTPUT:  none */
/**/
/*    EFFECT:  Performs rotations and changes colors to restore red-black */
/*             properties after a node is deleted */
/**/
/*    Modifies Input: this, x */
/**/
/*    The algorithm from this function is from _Introduction_To_Algorithms_ */
/***********************************************************************/

static void
DeleteFixUp(IntervalTree *it, IntervalTreeNode *x)
{
    IntervalTreeNode *w;
    IntervalTreeNode *rootLeft = it->root->left;

    while ((!x->red) && (rootLeft != x)) {
        if (x == x->parent->left) {
            w=x->parent->right;
            if (w->red) {
                w->red=0;
                x->parent->red=1;
                LeftRotate(it, x->parent);
                w=x->parent->right;
            }
            if ( (!w->right->red) && (!w->left->red) ) { 
                w->red=1;
                x=x->parent;
            } else {
                if (!w->right->red) {
                    w->left->red=0;
                    w->red=1;
                    RightRotate(it, w);
                    w=x->parent->right;
                }
                w->red=x->parent->red;
                x->parent->red=0;
                w->right->red=0;
                LeftRotate(it, x->parent);
                x=rootLeft; /* this is to exit while loop */
            }
        } else { /* the code below is has left and right switched from above */
            w=x->parent->left;
            if (w->red) {
                w->red=0;
                x->parent->red=1;
                RightRotate(it, x->parent);
                w=x->parent->left;
            }
            if ((!w->right->red) && (!w->left->red)) { 
                w->red=1;
                x=x->parent;
            } else {
                if (!w->left->red) {
                    w->right->red=0;
                    w->red=1;
                    LeftRotate(it, w);
                    w=x->parent->left;
                }
                w->red=x->parent->red;
                x->parent->red=0;
                w->left->red=0;
                RightRotate(it, x->parent);
                x=rootLeft; /* this is to exit while loop */
            }
        }
    }
    x->red=0;

#ifdef CHECK_INTERVAL_TREE_ASSUMPTIONS
    IT_CheckAssumptions(it);
#elif defined(DEBUG_ASSERT)
    Assert(!it->nil->red,"nil not black in ITDeleteFixUp");
    Assert((it->nil->maxHigh=LONG_MIN),
           "nil->maxHigh != LONG_MIN in ITDeleteFixUp");
#endif
}


/***********************************************************************/
/*  FUNCTION:  DeleteNode */
/**/
/*    INPUTS:  tree is the tree to delete node z from */
/**/
/*    OUTPUT:  returns the Interval stored at deleted node */
/**/
/*    EFFECT:  Deletes z from tree and but don't call destructor */
/*             Then calls FixUpMaxHigh to fix maxHigh fields then calls */
/*             ITDeleteFixUp to restore red-black properties */
/**/
/*    Modifies Input:  z */
/**/
/*    The algorithm from this function is from _Introduction_To_Algorithms_ */
/***********************************************************************/

void *
IT_delete_node(IntervalTree *it, IntervalTreeNode *z, long *low, long *high)
{
    IntervalTreeNode *x, *y;
    void *returnValue = z->data;
    if (low)
        *low = z->low;
    if (high)
        *high = z->high;

    y= ((z->left == it->nil) || (z->right == it->nil)) ?
        z : IT_get_successor(it, z);
    x= (y->left == it->nil) ? y->right : y->left;
    if (it->root == (x->parent = y->parent))
        /* assignment of y->p to x->p is intentional */
        it->root->left=x;
    else {
        if (y == y->parent->left)
            y->parent->left=x;
        else
            y->parent->right=x;
    }
    if (y != z) { /* y should not be nil in this case */

#ifdef DEBUG_ASSERT
        Assert( (y!=it->nil),"y is nil in DeleteNode \n");
#endif
        /* y is the node to splice out and x is its child */
  
        y->maxHigh = INT_MIN;
        y->left=z->left;
        y->right=z->right;
        y->parent=z->parent;
        z->left->parent=z->right->parent=y;
        if (z == z->parent->left)
            z->parent->left=y; 
        else
            z->parent->right=y;
        FixUpMaxHigh(it, x->parent); 
        if (!(y->red)) {
            y->red = z->red;
            DeleteFixUp(it, x);
        } else
            y->red = z->red; 
        yasm_xfree(z);
#ifdef CHECK_INTERVAL_TREE_ASSUMPTIONS
        IT_CheckAssumptions(it);
#elif defined(DEBUG_ASSERT)
        Assert(!it->nil->red,"nil not black in ITDelete");
        Assert((it->nil->maxHigh=LONG_MIN),"nil->maxHigh != LONG_MIN in ITDelete");
#endif
    } else {
        FixUpMaxHigh(it, x->parent);
        if (!(y->red))
            DeleteFixUp(it, x);
        yasm_xfree(y);
#ifdef CHECK_INTERVAL_TREE_ASSUMPTIONS
        IT_CheckAssumptions(it);
#elif defined(DEBUG_ASSERT)
        Assert(!it->nil->red,"nil not black in ITDelete");
        Assert((it->nil->maxHigh=LONG_MIN),"nil->maxHigh != LONG_MIN in ITDelete");
#endif
    }
    return returnValue;
}


/***********************************************************************/
/*  FUNCTION:  Overlap */
/**/
/*    INPUTS:  [a1,a2] and [b1,b2] are the low and high endpoints of two */
/*             closed intervals.  */
/**/
/*    OUTPUT:  stack containing pointers to the nodes between [low,high] */
/**/
/*    Modifies Input: none */
/**/
/*    EFFECT:  returns 1 if the intervals overlap, and 0 otherwise */
/***********************************************************************/

static int
Overlap(int a1, int a2, int b1, int b2)
{
    if (a1 <= b1)
        return (b1 <= a2);
    else
        return (a1 <= b2);
}


/***********************************************************************/
/*  FUNCTION:  Enumerate */
/**/
/*    INPUTS:  tree is the tree to look for intervals overlapping the */
/*             closed interval [low,high]  */
/**/
/*    OUTPUT:  stack containing pointers to the nodes overlapping */
/*             [low,high] */
/**/
/*    Modifies Input: none */
/**/
/*    EFFECT:  Returns a stack containing pointers to nodes containing */
/*             intervals which overlap [low,high] in O(max(N,k*log(N))) */
/*             where N is the number of intervals in the tree and k is  */
/*             the number of overlapping intervals                      */
/**/
/*    Note:    This basic idea for this function comes from the  */
/*              _Introduction_To_Algorithms_ book by Cormen et al, but */
/*             modifications were made to return all overlapping intervals */
/*             instead of just the first overlapping interval as in the */
/*             book.  The natural way to do this would require recursive */
/*             calls of a basic search function.  I translated the */
/*             recursive version into an interative version with a stack */
/*             as described below. */
/***********************************************************************/



/* The basic idea for the function below is to take the IntervalSearch
 * function from the book and modify to find all overlapping intervals
 * instead of just one.  This means that any time we take the left
 * branch down the tree we must also check the right branch if and only if
 * we find an overlapping interval in that left branch.  Note this is a
 * recursive condition because if we go left at the root then go left
 * again at the first left child and find an overlap in the left subtree
 * of the left child of root we must recursively check the right subtree
 * of the left child of root as well as the right child of root.
 */
void
IT_enumerate(IntervalTree *it, long low, long high, void *cbd,
             void (*callback) (IntervalTreeNode *node, void *cbd))
{
    IntervalTreeNode *x=it->root->left;
    int stuffToDo = (x != it->nil);
  
    /* Possible speed up: add min field to prune right searches */

#ifdef DEBUG_ASSERT
    Assert((it->recursionNodeStackTop == 1),
           "recursionStack not empty when entering IntervalTree::Enumerate");
#endif
    it->currentParent = 0;

    while (stuffToDo) {
        if (Overlap(low,high,x->low,x->high) ) {
            callback(x, cbd);
            it->recursionNodeStack[it->currentParent].tryRightBranch=1;
        }
        if(x->left->maxHigh >= low) { /* implies x != nil */
            if (it->recursionNodeStackTop == it->recursionNodeStackSize) {
                it->recursionNodeStackSize *= 2;
                it->recursionNodeStack = (it_recursion_node *) 
                    yasm_xrealloc(it->recursionNodeStack,
                                  it->recursionNodeStackSize * sizeof(it_recursion_node));
            }
            it->recursionNodeStack[it->recursionNodeStackTop].start_node = x;
            it->recursionNodeStack[it->recursionNodeStackTop].tryRightBranch = 0;
            it->recursionNodeStack[it->recursionNodeStackTop].parentIndex = it->currentParent;
            it->currentParent = it->recursionNodeStackTop++;
            x = x->left;
        } else {
            x = x->right;
        }
        stuffToDo = (x != it->nil);
        while (!stuffToDo && (it->recursionNodeStackTop > 1)) {
            if (it->recursionNodeStack[--it->recursionNodeStackTop].tryRightBranch) {
                x=it->recursionNodeStack[it->recursionNodeStackTop].start_node->right;
                it->currentParent=it->recursionNodeStack[it->recursionNodeStackTop].parentIndex;
                it->recursionNodeStack[it->currentParent].tryRightBranch=1;
                stuffToDo = (x != it->nil);
            }
        }
    }
#ifdef DEBUG_ASSERT
    Assert((it->recursionNodeStackTop == 1),
           "recursionStack not empty when exiting IntervalTree::Enumerate");
#endif
}

#ifdef CHECK_INTERVAL_TREE_ASSUMPTIONS

static int
CheckMaxHighFieldsHelper(const IntervalTree *it, IntervalTreeNode *y, 
                         int currentHigh, int match)
{
    if (y != it->nil) {
        match = CheckMaxHighFieldsHelper(it, y->left, currentHigh, match) ?
            1 : match;
        VERIFY(y->high <= currentHigh);
        if (y->high == currentHigh)
            match = 1;
        match = CheckMaxHighFieldsHelper(it, y->right, currentHigh, match) ?
            1 : match;
    }
    return match;
}

          

/* Make sure the maxHigh fields for everything makes sense. *
 * If something is wrong, print a warning and exit */
static void
CheckMaxHighFields(const IntervalTree *it, IntervalTreeNode *x)
{
    if (x != it->nil) {
        CheckMaxHighFields(it, x->left);
        if(!(CheckMaxHighFieldsHelper(it, x, x->maxHigh, 0) > 0)) {
            fprintf(stderr, "error found in CheckMaxHighFields.\n");
            abort();
        }
        CheckMaxHighFields(it, x->right);
    }
}

static void
IT_CheckAssumptions(const IntervalTree *it)
{
    VERIFY(it->nil->low == INT_MIN);
    VERIFY(it->nil->high == INT_MIN);
    VERIFY(it->nil->maxHigh == INT_MIN);
    VERIFY(it->root->low == INT_MAX);
    VERIFY(it->root->high == INT_MAX);
    VERIFY(it->root->maxHigh == INT_MAX);
    VERIFY(it->nil->data == NULL);
    VERIFY(it->root->data == NULL);
    VERIFY(it->nil->red == 0);
    VERIFY(it->root->red == 0);
    CheckMaxHighFields(it, it->root->left);
}
#endif

