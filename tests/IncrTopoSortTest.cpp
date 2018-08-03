/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRefCnt.h"
#include "SkTSort.h"
#include "Test.h"

#include "sk_tool_utils.h"

// A node in the graph. This corresponds to an opList in the MDB world.
class Node : public SkRefCnt {
public:
    uint32_t id() const { return fID; }
    int indexInSort() const { SkASSERT(fIndexInSort >= 0); return fIndexInSort; }
    bool visited() const { return fVisited; }

#ifdef SK_DEBUG
    void print() const {
        SkDebugf("%d: id %d", fIndexInSort, fID);
        if (fNodesIDependOn.count()) {
            SkDebugf("I depend on (%d): ", fNodesIDependOn.count());
            for (Node* tmp : fNodesIDependOn) {
                SkDebugf("%d, ", tmp->id());
            }
        }
        if (fNodesThatDependOnMe.count()) {
            SkDebugf("(%d) depend on me: ", fNodesThatDependOnMe.count());
            for (Node* tmp : fNodesThatDependOnMe) {
                SkDebugf("%d, ", tmp->id());
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
        REPORTER_ASSERT(reporter, !fVisited); // this flag should only be true w/in addEdges()
    }

    static bool CompareIndicesGT(Node* const& a, Node* const& b) {
        return a->indexInSort() > b->indexInSort();
    }

    int numDependents() const { return fNodesThatDependOnMe.count(); }
    Node* dependent(int index) const { return fNodesThatDependOnMe[index]; }
private:
    friend class Graph;

    Node(uint32_t id) : fID(id), fIndexInSort(-1), fVisited(false) {}

    void setIndexInSort(int indexInSort) { fIndexInSort = indexInSort; }
    void setVisited(bool visited) { fVisited = visited; }
    void addDependency(Node* dependedOn) {
        SkASSERT(dependedOn->indexInSort() < this->indexInSort());
        fNodesIDependOn.push(dependedOn);

        dependedOn->addDependent(this);
    }
    void addDependent(Node* dependent) {
        fNodesThatDependOnMe.push(dependent);
    }

    uint32_t         fID;
    SkTDArray<Node*> fNodesIDependOn;      // These nodes must appear before this one in the sort
    SkTDArray<Node*> fNodesThatDependOnMe; // These ones must appear after this one
    int              fIndexInSort;
    bool             fVisited;             // only used in addEdges()
};

// A queue adaptor for an SkTDArray. Hacky.
class NodeQueue {
public:
    NodeQueue() : fFront(0) {}

    int count() const {
        return fNodes.count() - fFront;
    }

    bool isEmpty() const {
        return 0 == this->count();
    }

    void pushBack(Node* node) {
        fNodes.push(node);
    }

    Node* front() {
        SkASSERT(fFront < fNodes.count());
        return fNodes[fFront];
    }

    void popFront() {
        SkASSERT(fFront < fNodes.count());
        fFront++;
    }

private:
    int              fFront;
    SkTDArray<Node*> fNodes;
};

// The DAG driving the incremental topological sort. This corresponds to the opList DAG in
// the MDB world.
class Graph {
public:
    Graph(int numNodesToReserve, skiatest::Reporter* reporter)
            : fNodes(numNodesToReserve)
            , fReporter(reporter) {
    }

    Node* addNode(uint32_t id) {
        sk_sp<Node> tmp(new Node(id));

        fNodes.push_back(tmp);       // The graph gets the creation ref
        tmp->setIndexInSort(fNodes.count()-1);
        this->validate();
        return tmp.release();
    }

    // 'dependedOn' must appear before 'dependent' in the sort
    bool addEdge(Node* dependedOn, Node* dependent) {
        // TODO: this would be faster if all the SkTDArray code was stripped out of
        // addEdges but, when used in MDB sorting, this entry point will never be used.
        SkTDArray<Node*> tmp(&dependedOn, 1);
        return this->addEdges(tmp, dependent);
    }

    // All the nodes in 'dependedOn' must appear before 'dependent' in the sort.
    // This is O(v + e + cost_of_sorting(b)) where:
    //    v: number of nodes
    //    e: number of edges
    //    b: number of new edges in 'dependedOn'
    bool addEdges(SkTDArray<Node*> dependedOn, Node* dependent) {
        this->validate();

        // remove any of the new dependencies that are already satisfied
        for (int i = 0; i < dependedOn.count(); ++i) {
            if (dependedOn[i]->indexInSort() < dependent->indexInSort()) {
                dependent->addDependency(dependedOn[i]);
                dependedOn.removeShuffle(i);
                i--;
            }
        }

        this->validate();

        // Sort the remaining dependencies into descending order based on their indices in the
        // sort. This means that we will be proceeding from right to left in the sort when
        // correcting the order.
        // TODO: QSort is waaay overkill here!
        SkTQSort<Node*>(dependedOn.begin(), dependedOn.end() - 1, Node::CompareIndicesGT);

#ifdef SK_DEBUG
        int lastIndex = fNodes.count();
        for (int i = 0; i < dependedOn.count(); ++i) {
            SkASSERT(lastIndex > dependedOn[i]->indexInSort());
            lastIndex = dependedOn[i]->indexInSort();
        }
#endif

        // TODO: although this is the general algorithm, I think this can be simplified for our
        // use case (i.e., the same dependent for all the new edges).
        int lowerBound = fNodes.count();
        for (int i = 0; i < dependedOn.count(); ++i) {
            if (dependedOn[i]->indexInSort() < lowerBound) {
                this->shift(lowerBound);
            }

            if (!dependent->visited()) {
                this->dfs(dependent, dependedOn[i]->indexInSort());
            }

            lowerBound = SkTMin(dependent->indexInSort(), lowerBound);
        }

        this->shift(lowerBound);

        this->validate();
        return false;
    }

    // Get the list of node ids in the current sorted order
    void getActual(SkString* actual) const {
        for (int i = 0; i < fNodes.count(); ++i) {
            actual->appendU32(fNodes[i]->id());
            if (i < fNodes.count()-1) {
                actual->append(",");
            }
        }
    }

#ifdef SK_DEBUG
    void print() const {
        SkDebugf("-------------------\n");
        for (int i = 0; i < fNodes.count(); ++i) {
            fNodes[i]->print();
        }
    }
#endif

    void validate() const {
        REPORTER_ASSERT(fReporter, fQueue.isEmpty());

        for (int i = 0; i < fNodes.count(); ++i) {
            REPORTER_ASSERT(fReporter, fNodes[i]->indexInSort() == i);

            fNodes[i]->validate(fReporter);
        }
    }

private:
    void dfs(Node* node, int upperBound) {
        node->setVisited(true);

        for (int i = 0; i < node->numDependents(); ++i) {
            Node* dependent = node->dependent(i);

            SkASSERT(dependent->indexInSort() != upperBound); // this would be a cycle

            if (!dependent->visited() && dependent->indexInSort() < upperBound) {
                this->dfs(dependent, upperBound);
            }
        }

        fQueue.pushBack(node);
    }

    void place(sk_sp<Node> node, int index) {
        SkASSERT(!fNodes[index]);
        fNodes[index] = node;
        node->setIndexInSort(index);
    }

    void shift(int index) {
        int numRemoved = 0;
        while (!fQueue.isEmpty()) {
            sk_sp<Node> node = fNodes[index];

            if (node->visited()) {
                node->setVisited(false);  // reset for future use
                numRemoved++;
            } else {
                this->place(node, index - numRemoved);
            }

            Node* tmp = fQueue.front();
            while (!fQueue.isEmpty() && node.get() == tmp) {
                numRemoved--;

                this->place(sk_ref_sp(tmp), index - numRemoved);

                fQueue.popFront();
                tmp = fQueue.front();
            }

            index++;
        }
    }

    SkTArray<sk_sp<Node>> fNodes;

    NodeQueue             fQueue;     // only used in addEdges()

    skiatest::Reporter*   fReporter;
};

static void test_queue(skiatest::Reporter* reporter) {
    Graph g(2, reporter);
    NodeQueue q;
    Node* node1 = g.addNode(1);
    Node* node2 = g.addNode(2);

    REPORTER_ASSERT(reporter, q.isEmpty());
    REPORTER_ASSERT(reporter, 0 == q.count());

    q.pushBack(node1);
    REPORTER_ASSERT(reporter, !q.isEmpty());
    REPORTER_ASSERT(reporter, 1 == q.count());
    REPORTER_ASSERT(reporter, node1 == q.front());

    q.pushBack(node2);
    REPORTER_ASSERT(reporter, !q.isEmpty());
    REPORTER_ASSERT(reporter, 2 == q.count());
    REPORTER_ASSERT(reporter, node1 == q.front());

    q.popFront();
    REPORTER_ASSERT(reporter, !q.isEmpty());
    REPORTER_ASSERT(reporter, 1 == q.count());
    REPORTER_ASSERT(reporter, node2 == q.front());

    q.popFront();
    REPORTER_ASSERT(reporter, q.isEmpty());
    REPORTER_ASSERT(reporter, 0 == q.count());
}

static void test_diamond(skiatest::Reporter* reporter) {
    Graph g(4, reporter);

    // Create the graph (the '.' is the pointy side of the arrow):
    //        1
    //      .    \
    //     /      .
    //   0          3
    //     \      .
    //       .   /
    //         2
    // Possible topological orders are [0,2,1,3] and [0,1,2,3].

    Node* node3 = g.addNode(3);
    Node* node2 = g.addNode(2);
    Node* node1 = g.addNode(1);
    g.validate();

    SkTDArray<Node*> dependedOn;
    dependedOn.push(node1);
    dependedOn.push(node2);

    g.addEdges(dependedOn, node3); // nodes 1 and 2 must come before node3
    g.print();

    Node* node0 = g.addNode(0);
    g.addEdge(node0, node1);            // node0 must come before node1
    g.print();
    g.addEdge(node0, node2);            // node0 must come before node2
    g.print();

    const SkString kExpected0("0,2,1,3");
    const SkString kExpected1("0,1,2,3");
    SkString actual;
    g.getActual(&actual);

    REPORTER_ASSERT(reporter, kExpected0 == actual || kExpected1 == actual);
}

static void test_lopsided_binary_tree(skiatest::Reporter* reporter) {
    Graph g(7, reporter);

    // Create the graph (the '.' is the pointy side of the arrow):
    //        0
    //       /  \
    //      .    .
    //     1      2
    //           /  \
    //          .    .
    //         3      4
    //               /  \
    //              .    .
    //             5      6
    //
    // Possible topological order is: [0,1,2,3,4,5,6].

}

DEF_TEST(IncrTopoSort, reporter) {
    test_queue(reporter);
    test_diamond(reporter);
//    test_lopsided_binary_tree(reporter);
}
