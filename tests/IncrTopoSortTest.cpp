/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTDArray.h"
#include "src/base/SkTSort.h"
#include "tests/Test.h"

#include <algorithm>
#include <cstdint>

using namespace skia_private;

// A node in the graph. This corresponds to an opsTask in the MDB world.
class Node : public SkRefCnt {
public:
    char id() const { return fID; }
    int indexInSort() const { SkASSERT(fIndexInSort >= 0); return fIndexInSort; }
    bool visited() const { return fVisited; }

#ifdef SK_DEBUG
    void print() const {
        SkDebugf("%d: id %c", fIndexInSort, fID);
        if (fNodesIDependOn.size()) {
            SkDebugf(" I depend on (%d): ", fNodesIDependOn.size());
            for (Node* tmp : fNodesIDependOn) {
                SkDebugf("%c, ", tmp->id());
            }
        }
        if (fNodesThatDependOnMe.size()) {
            SkDebugf(" (%d) depend on me: ", fNodesThatDependOnMe.size());
            for (Node* tmp : fNodesThatDependOnMe) {
                SkDebugf("%c, ", tmp->id());
            }
        }
        SkDebugf("\n");
    }
#endif

    void validate(skiatest::Reporter* reporter) const {
        for (Node* dependedOn : fNodesIDependOn) {
            REPORTER_ASSERT(reporter, dependedOn->indexInSort() < this->indexInSort());
        }
        for (Node* dependent : fNodesThatDependOnMe) {
            REPORTER_ASSERT(reporter, this->indexInSort() < dependent->indexInSort());
        }
        REPORTER_ASSERT(reporter, !fVisited); // this flag should only be true w/in depEdges()
    }

    static bool CompareIndicesGT(Node* const& a, Node* const& b) {
        return a->indexInSort() > b->indexInSort();
    }

    int numDependents() const { return fNodesThatDependOnMe.size(); }
    Node* dependent(int index) const {
        SkASSERT(0 <= index && index < fNodesThatDependOnMe.size());
        return fNodesThatDependOnMe[index];
    }

private:
    friend class Graph;

    explicit Node(char id) : fID(id), fIndexInSort(-1), fVisited(false) {}

    void setIndexInSort(int indexInSort) { fIndexInSort = indexInSort; }
    void setVisited(bool visited) { fVisited = visited; }
    void addDependency(Node* dependedOn) {
        fNodesIDependOn.push_back(dependedOn);

        dependedOn->addDependent(this);
    }
    void addDependent(Node* dependent) {
        fNodesThatDependOnMe.push_back(dependent);
    }

    char             fID;
    SkTDArray<Node*> fNodesIDependOn;      // These nodes must appear before this one in the sort
    SkTDArray<Node*> fNodesThatDependOnMe; // These ones must appear after this one
    int              fIndexInSort;
    bool             fVisited;             // only used in addEdges()
};

// The DAG driving the incremental topological sort. This corresponds to the opsTask DAG in
// the MDB world.
class Graph {
public:
    Graph(int numNodesToReserve, skiatest::Reporter* reporter)
            : fNodes(numNodesToReserve)
            , fReporter(reporter) {
    }

    Node* addNode(uint32_t id) {
        this->validate();
        sk_sp<Node> tmp(new Node(id));

        fNodes.push_back(tmp);       // The graph gets the creation ref
        tmp->setIndexInSort(fNodes.size()-1);
        this->validate();
        return tmp.get();
    }

    // 'dependedOn' must appear before 'dependent' in the sort
    void addEdge(Node* dependedOn, Node* dependent) {
        // TODO: this would be faster if all the SkTDArray code was stripped out of
        // addEdges but, when used in MDB sorting, this entry point will never be used.
        SkTDArray<Node*> tmp(&dependedOn, 1);
        this->addEdges(&tmp, dependent);
    }

    // All the nodes in 'dependedOn' must appear before 'dependent' in the sort.
    // This is O(v + e + cost_of_sorting(b)) where:
    //    v: number of nodes
    //    e: number of edges
    //    b: number of new edges in 'dependedOn'
    //
    // The algorithm works by first finding the "affected region" that contains all the
    // nodes whose position in the topological sort is invalidated by the addition of the new
    // edges. It then traverses the affected region from left to right, temporarily removing
    // invalid nodes from 'fNodes' and shifting valid nodes left to fill in the gaps. In this
    // left to right traversal, when a node is shifted to the left the current set of invalid
    // nodes is examined to see if any needed to be moved to the right of that node. If so,
    // they are reintroduced to the 'fNodes' array but now in the appropriate position. The
    // separation of the algorithm into search (the dfs method) and readjustment (the shift
    // method) means that each node affected by the new edges is only ever moved once.
    void addEdges(SkTDArray<Node*>* dependedOn, Node* dependent) {
        this->validate();

        // remove any of the new dependencies that are already satisfied
        for (int i = 0; i < dependedOn->size(); ++i) {
            if ((*dependedOn)[i]->indexInSort() < dependent->indexInSort()) {
                dependent->addDependency((*dependedOn)[i]);
                dependedOn->removeShuffle(i);
                i--;
            } else {
                dependent->addDependency((*dependedOn)[i]);
            }
        }

        if (dependedOn->empty()) {
            return;
        }

        // Sort the remaining dependencies into descending order based on their indices in the
        // sort. This means that we will be proceeding from right to left in the sort when
        // correcting the order.
        // TODO: QSort is waaay overkill here!
        SkTQSort<Node*>(dependedOn->begin(), dependedOn->end(), Node::CompareIndicesGT);

        // TODO: although this is the general algorithm, I think this can be simplified for our
        // use case (i.e., the same dependent for all the new edges).

        int lowerBound = fNodes.size();  // 'lowerBound' tracks the left of the affected region
        for (int i = 0; i < dependedOn->size(); ++i) {
            if ((*dependedOn)[i]->indexInSort() < lowerBound) {
                this->shift(lowerBound);
            }

            if (!dependent->visited()) {
                this->dfs(dependent, (*dependedOn)[i]->indexInSort());
            }

            lowerBound = std::min(dependent->indexInSort(), lowerBound);
        }

        this->shift(lowerBound);

        this->validate();
    }

    // Get the list of node ids in the current sorted order
    void getActual(SkString* actual) const {
        this->validate();

        for (int i = 0; i < fNodes.size(); ++i) {
            (*actual) += fNodes[i]->id();
            if (i < fNodes.size()-1) {
                (*actual) += ',';
            }
        }
    }

#ifdef SK_DEBUG
    void print() const {
        SkDebugf("-------------------\n");
        for (int i = 0; i < fNodes.size(); ++i) {
            if (fNodes[i]) {
                SkDebugf("%c ", fNodes[i]->id());
            } else {
                SkDebugf("0 ");
            }
        }
        SkDebugf("\n");

        for (int i = 0; i < fNodes.size(); ++i) {
            if (fNodes[i]) {
                fNodes[i]->print();
            }
        }

        SkDebugf("Stack: ");
        for (int i = 0; i < fStack.size(); ++i) {
           SkDebugf("%c/%c ", fStack[i].fNode->id(), fStack[i].fDest->id());
        }
        SkDebugf("\n");
    }
#endif

private:
    void validate() const {
        REPORTER_ASSERT(fReporter, fStack.empty());

        for (int i = 0; i < fNodes.size(); ++i) {
            REPORTER_ASSERT(fReporter, fNodes[i]->indexInSort() == i);

            fNodes[i]->validate(fReporter);
        }

        // All the nodes in the Queue had better have been marked as visited
        for (int i = 0; i < fStack.size(); ++i) {
            SkASSERT(fStack[i].fNode->visited());
        }
    }

    // Collect the nodes that need to be moved within the affected region. All the nodes found
    // to be in violation of the topological constraints are placed in 'fStack'.
    void dfs(Node* node, int upperBound) {
        node->setVisited(true);

        for (int i = 0; i < node->numDependents(); ++i) {
            Node* dependent = node->dependent(i);

            SkASSERT(dependent->indexInSort() != upperBound); // this would be a cycle

            if (!dependent->visited() && dependent->indexInSort() < upperBound) {
                this->dfs(dependent, upperBound);
            }
        }

        fStack.push_back({ sk_ref_sp(node), fNodes[upperBound].get() });
    }

    // Move 'node' to the index-th slot of the sort. The index-th slot should not have a current
    // occupant.
    void moveNodeInSort(const sk_sp<Node>& node, int index) {
        SkASSERT(!fNodes[index]);
        fNodes[index] = node;
        node->setIndexInSort(index);
    }

#ifdef SK_DEBUG
    // Does 'fStack' have 'node'? That is, was 'node' discovered to be in violation of the
    // topological constraints?
    bool stackContains(Node* node) {
        for (int i = 0; i < fStack.size(); ++i) {
            if (node == fStack[i].fNode.get()) {
                return true;
            }
        }
        return false;
    }
#endif

    // The 'shift' method iterates through the affected area from left to right moving Nodes that
    // were found to be in violation of the topological order (i.e., in 'fStack') to their correct
    // locations and shifting the non-violating nodes left, into the holes the violating nodes left
    // behind.
    void shift(int index) {
        int numRemoved = 0;
        while (!fStack.empty()) {
            sk_sp<Node> node = fNodes[index];

            if (node->visited()) {
                // This node is in 'fStack' and was found to be in violation of the topological
                // constraints. Remove it from 'fNodes' so non-violating nodes can be shifted
                // left.
                SkASSERT(this->stackContains(node.get()));
                node->setVisited(false);  // reset for future use
                fNodes[index] = nullptr;
                numRemoved++;
            } else {
                // This node was found to not be in violation of any topological constraints but
                // must be moved left to fill in for those that were.
                SkASSERT(!this->stackContains(node.get()));
                SkASSERT(numRemoved); // should be moving left

                this->moveNodeInSort(node, index - numRemoved);
                fNodes[index] = nullptr;
            }

            while (!fStack.empty() && node.get() == fStack.back().fDest) {
                // The left to right loop has finally encountered the destination for one or more
                // of the nodes in 'fStack'. Place them to the right of 'node' in the sort. Note
                // that because the violating nodes were already removed from 'fNodes' there
                // should be enough empty space for them to be placed now.
                numRemoved--;

                this->moveNodeInSort(fStack.back().fNode, index - numRemoved);

                fStack.pop_back();
            }

            index++;
        }
    }

    TArray<sk_sp<Node>> fNodes;

    struct StackInfo {
        sk_sp<Node> fNode;  // This gets a ref bc, in 'shift' it will be pulled out of 'fNodes'
        Node*       fDest;
    };

    TArray<StackInfo>   fStack;     // only used in addEdges()

    skiatest::Reporter*   fReporter;
};

// This test adds a single invalidating edge.
static void test_1(skiatest::Reporter* reporter) {
    Graph g(10, reporter);

    Node* nodeQ = g.addNode('q');
    Node* nodeY = g.addNode('y');
    Node* nodeA = g.addNode('a');
    Node* nodeZ = g.addNode('z');
    Node* nodeB = g.addNode('b');
    /*Node* nodeC =*/ g.addNode('c');
    Node* nodeW = g.addNode('w');
    Node* nodeD = g.addNode('d');
    Node* nodeX = g.addNode('x');
    Node* nodeR = g.addNode('r');

    // All these edge are non-invalidating
    g.addEdge(nodeD, nodeR);
    g.addEdge(nodeZ, nodeW);
    g.addEdge(nodeA, nodeB);
    g.addEdge(nodeY, nodeZ);
    g.addEdge(nodeQ, nodeA);

    {
        const SkString kExpectedInitialState("q,y,a,z,b,c,w,d,x,r");
        SkString actualInitialState;
        g.getActual(&actualInitialState);
        REPORTER_ASSERT(reporter, kExpectedInitialState == actualInitialState);
    }

    // Add the invalidating edge
    g.addEdge(nodeX, nodeY);

    {
        const SkString kExpectedFinalState("q,a,b,c,d,x,y,z,w,r");
        SkString actualFinalState;
        g.getActual(&actualFinalState);
        REPORTER_ASSERT(reporter, kExpectedFinalState == actualFinalState);
    }
}

// This test adds two invalidating edge sequentially
static void test_2(skiatest::Reporter* reporter) {
    Graph g(10, reporter);

    Node* nodeY = g.addNode('y');
    /*Node* nodeA =*/ g.addNode('a');
    Node* nodeW = g.addNode('w');
    /*Node* nodeB =*/ g.addNode('b');
    Node* nodeZ = g.addNode('z');
    Node* nodeU = g.addNode('u');
    /*Node* nodeC =*/ g.addNode('c');
    Node* nodeX = g.addNode('x');
    /*Node* nodeD =*/ g.addNode('d');
    Node* nodeV = g.addNode('v');

    // All these edge are non-invalidating
    g.addEdge(nodeU, nodeX);
    g.addEdge(nodeW, nodeU);
    g.addEdge(nodeW, nodeZ);
    g.addEdge(nodeY, nodeZ);

    {
        const SkString kExpectedInitialState("y,a,w,b,z,u,c,x,d,v");
        SkString actualInitialState;
        g.getActual(&actualInitialState);
        REPORTER_ASSERT(reporter, kExpectedInitialState == actualInitialState);
    }

    // Add the first invalidating edge
    g.addEdge(nodeX, nodeY);

    {
        const SkString kExpectedFirstState("a,w,b,u,c,x,y,z,d,v");
        SkString actualFirstState;
        g.getActual(&actualFirstState);
        REPORTER_ASSERT(reporter, kExpectedFirstState == actualFirstState);
    }

    // Add the second invalidating edge
    g.addEdge(nodeV, nodeW);

    {
        const SkString kExpectedSecondState("a,b,c,d,v,w,u,x,y,z");
        SkString actualSecondState;
        g.getActual(&actualSecondState);
        REPORTER_ASSERT(reporter, kExpectedSecondState == actualSecondState);
    }
}

static void test_diamond(skiatest::Reporter* reporter) {
    Graph g(4, reporter);

    /* Create the graph (the '.' is the pointy side of the arrow):
     *        b
     *      .    \
     *     /      .
     *   a          d
     *     \      .
     *       .   /
     *         c
     * Possible topological orders are [a,c,b,d] and [a,b,c,d].
     */

    Node* nodeD = g.addNode('d');
    Node* nodeC = g.addNode('c');
    Node* nodeB = g.addNode('b');

    {
        SkTDArray<Node*> dependedOn;
        dependedOn.push_back(nodeB);
        dependedOn.push_back(nodeC);

        g.addEdges(&dependedOn, nodeD); // nodes B and C must come before node D
    }

    Node* nodeA = g.addNode('a');
    g.addEdge(nodeA, nodeB);            // node A must come before node B
    g.addEdge(nodeA, nodeC);            // node A must come before node C

    const SkString kExpected0("a,c,b,d");
    const SkString kExpected1("a,b,c,d");
    SkString actual;
    g.getActual(&actual);

    REPORTER_ASSERT(reporter, kExpected0 == actual || kExpected1 == actual);
}

static void test_lopsided_binary_tree(skiatest::Reporter* reporter) {
    Graph g(7, reporter);

    /* Create the graph (the '.' is the pointy side of the arrow):
     *        a
     *       /  \
     *      .    .
     *     b      c
     *           /  \
     *          .    .
     *         d      e
     *               /  \
     *              .    .
     *             f      g
     *
     * Possible topological order is: [a,b,c,d,e,f,g].
     */

    Node* nodeG = g.addNode('g');
    Node* nodeF = g.addNode('f');
    Node* nodeE = g.addNode('e');
    Node* nodeD = g.addNode('d');
    Node* nodeC = g.addNode('c');
    Node* nodeB = g.addNode('b');
    Node* nodeA = g.addNode('a');

    g.addEdge(nodeE, nodeG);
    g.addEdge(nodeE, nodeF);
    g.addEdge(nodeC, nodeE);
    g.addEdge(nodeC, nodeD);
    g.addEdge(nodeA, nodeC);
    g.addEdge(nodeA, nodeB);

    const SkString kExpected("a,b,c,d,e,f,g");
    SkString actual;
    g.getActual(&actual);

    REPORTER_ASSERT(reporter, kExpected == actual);
}

DEF_TEST(IncrTopoSort, reporter) {
    test_1(reporter);
    test_2(reporter);
    test_diamond(reporter);
    test_lopsided_binary_tree(reporter);
}
