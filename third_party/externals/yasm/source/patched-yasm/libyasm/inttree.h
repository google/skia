#ifndef YASM_INTTREE_H
#define YASM_INTTREE_H

#ifndef YASM_LIB_DECL
#define YASM_LIB_DECL
#endif

/* The interval_tree.h and interval_tree.cc files contain code for 
 * interval trees implemented using red-black-trees as described in
 * the book _Introduction_To_Algorithms_ by Cormen, Leisserson, 
 * and Rivest.
 */

typedef struct IntervalTreeNode {
    struct IntervalTreeNode *left, *right, *parent;
    void *data;
    long low;
    long high;
    long maxHigh;
    int red; /* if red=0 then the node is black */
} IntervalTreeNode;

typedef struct it_recursion_node {
    /* This structure stores the information needed when we take the
     * right branch in searching for intervals but possibly come back
     * and check the left branch as well.
     */
    IntervalTreeNode *start_node;
    unsigned int parentIndex;
    int tryRightBranch;
} it_recursion_node;

typedef struct IntervalTree {
    /* A sentinel is used for root and for nil.  These sentinels are
     * created when ITTreeCreate is called.  root->left should always
     * point to the node which is the root of the tree.  nil points to a
     * node which should always be black but has aribtrary children and
     * parent and no key or info.  The point of using these sentinels is so
     * that the root and nil nodes do not require special cases in the code
     */
    IntervalTreeNode *root;
    IntervalTreeNode *nil;

/*private:*/
    unsigned int recursionNodeStackSize;
    it_recursion_node * recursionNodeStack;
    unsigned int currentParent;
    unsigned int recursionNodeStackTop;
} IntervalTree;

YASM_LIB_DECL
IntervalTree *IT_create(void);
YASM_LIB_DECL
void IT_destroy(IntervalTree *);
YASM_LIB_DECL
void IT_print(const IntervalTree *);
YASM_LIB_DECL
void *IT_delete_node(IntervalTree *, IntervalTreeNode *, long *low,
                     long *high);
YASM_LIB_DECL
IntervalTreeNode *IT_insert(IntervalTree *, long low, long high, void *data);
YASM_LIB_DECL
IntervalTreeNode *IT_get_predecessor(const IntervalTree *, IntervalTreeNode *);
YASM_LIB_DECL
IntervalTreeNode *IT_get_successor(const IntervalTree *, IntervalTreeNode *);
YASM_LIB_DECL
void IT_enumerate(IntervalTree *, long low, long high, void *cbd,
                  void (*callback) (IntervalTreeNode *node, void *cbd));

#endif
