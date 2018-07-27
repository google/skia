
/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRefCnt.h"
#include "Test.h"

#include "sk_tool_utils.h"


class Graph {
public:
    class Node : public SkRefCnt {
    public:
        uint32_t id() const { return fID; }
        int indexInSort() const { return fIndexInSort; }

        void print() const {
            SkDebugf("%d: id %d", fIndexInSort, fID);
            if (fDependedOn.count()) {
                SkDebugf("depends on (%d): ", fDependedOn.count());
                for (Node* tmp : fDependedOn) {
                    SkDebugf("%d, ", tmp->id());
                }
            }

            SkDebugf("\n");
        }

        void validate(skiatest::Reporter* reporter) const {
            for (Node* dependedOn : fDependedOn) {
                REPORTER_ASSERT(reporter, dependedOn->indexInSort() < this->indexInSort());
            }
        }

    private:
        friend class Graph;

        Node(uint32_t id) : fID(id) {}

        void setIndexInSort(int indexInSort) { fIndexInSort = indexInSort; }
        void addDependency(Node* dependedOn) {
            fDependedOn.push(dependedOn);
        }

        uint32_t fID;
        SkTDArray<Node*> fDependedOn;  // All these nodes must appear before this one in the sort
        int fIndexInSort;
    };

    Graph(int numNodes, skiatest::Reporter* reporter) : fNodes(numNodes), fReporter(reporter) {}

    Node* addNode(uint32_t id) {
        sk_sp<Node> tmp(new Node(id));

        fNodes.push_back(tmp);       // The graph gets the creation ref
        tmp->setIndexInSort(fNodes.count()-1);
        this->validate();

        return tmp.release();
    }

    // 'dependedOn' must appear before 'dependent' in the sort
    bool addEdge(Node* dependedOn, Node* dependent) {
        SkTDArray<Node*> tmp(&dependedOn, 1);
        return this->addEdges(tmp, dependent);
    }

    // All the nodes in 'dependedOn' must appear before 'dependent' in the sort
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
        return false;
    }

    void getActual(SkString* actual) const {
        for (int i = 0; i < fNodes.count(); ++i) {
            actual->appendU32(fNodes[i]->id());
            if (i < fNodes.count()-1) {
                actual->append(",");
            }
        }
    }

    void print() const {
        SkDebugf("-------------------\n");
        for (int i = 0; i < fNodes.count(); ++i) {
            fNodes[i]->print();
        }
    }

    void validate() const {
        for (int i = 0; i < fNodes.count(); ++i) {
            REPORTER_ASSERT(fReporter, fNodes[i]->indexInSort() == i);

            fNodes[i]->validate(fReporter);
        }
    }

private:
    SkTArray<sk_sp<Node>> fNodes;

    skiatest::Reporter* fReporter;
};


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

    Graph::Node* node3 = g.addNode(3);
    Graph::Node* node2 = g.addNode(2);
    Graph::Node* node1 = g.addNode(1);
    g.validate();

    SkTDArray<Graph::Node*> dependedOn;
    dependedOn.push(node1);
    dependedOn.push(node2);

    g.addEdges(dependedOn, node3); // nodes 1 and 2 must come before node3
    g.print();

    Graph::Node* node0 = g.addNode(0);
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
    test_diamond(reporter);
    test_lopsided_binary_tree(reporter);
}
