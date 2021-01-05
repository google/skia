/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrTCluster.h"
#include "tests/Test.h"

#include "tools/ToolUtils.h"

using ToolUtils::TopoTestNode;

typedef void (*CreateGraphPF)(SkTArray<sk_sp<TopoTestNode>>* graph,
                              SkTArray<sk_sp<TopoTestNode>>* expected);

/*
 * In:  A1 B1 A2
 * Out: B1 A1 A2
 */
static void create_graph0(SkTArray<sk_sp<TopoTestNode>>* graph,
                          SkTArray<sk_sp<TopoTestNode>>* expected) {
    ToolUtils::TopoTestNode::AllocNodes(graph, 3);

    graph->at(0)->targets(0);
    graph->at(1)->targets(1);
    graph->at(2)->targets(0);
    graph->at(2)->dependsOn(graph->at(0).get());

    expected->push_back(graph->at(1));
    expected->push_back(graph->at(0));
    expected->push_back(graph->at(2));
}

/*
 * In:  A1 B1 A2 C1 A3
 * Out: B1 C1 A1 A2 A3
 */
static void create_graph1(SkTArray<sk_sp<TopoTestNode>>* graph,
                          SkTArray<sk_sp<TopoTestNode>>* expected) {
    ToolUtils::TopoTestNode::AllocNodes(graph, 5);

    graph->at(0)->targets(0);
    graph->at(1)->targets(1);
    graph->at(2)->targets(0);
    graph->at(3)->targets(2);
    graph->at(4)->targets(0);

    expected->push_back(graph->at(1));
    expected->push_back(graph->at(3));
    expected->push_back(graph->at(0));
    expected->push_back(graph->at(2));
    expected->push_back(graph->at(4));
}

/*
 * In:   A1 B1 A2.
 * Srcs: A1->B1, B1->A2.
 * Out:  A1 B1 A2. Can't reorder.
 */
static void create_graph2(SkTArray<sk_sp<TopoTestNode>>* graph,
                          SkTArray<sk_sp<TopoTestNode>>* expected) {
    ToolUtils::TopoTestNode::AllocNodes(graph, 3);

    graph->at(0)->targets(0);
    graph->at(1)->targets(1);
    graph->at(2)->targets(0);

    graph->at(1)->dependsOn(graph->at(0).get());
    graph->at(2)->dependsOn(graph->at(1).get());

    // expected is empty. Can't reorder.
}

DEF_TEST(GrTCluster, reporter) {
    CreateGraphPF tests[] = {
        create_graph0,
        create_graph1,
        create_graph2
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(tests); ++i) {
        SkTArray<sk_sp<TopoTestNode>> graph;
        SkTArray<sk_sp<TopoTestNode>> expectedOutput;

        (tests[i])(&graph, &expectedOutput);

        SkTInternalLList<TopoTestNode> llist;
        bool actualResult = GrTCluster<TopoTestNode>(graph, &llist);

        if (expectedOutput.empty()) {
            REPORTER_ASSERT(reporter, !actualResult);
        } else {
            REPORTER_ASSERT(reporter, actualResult);
            // SkTInternalLList::countEntries is debug-only and these tests run in release.
            int newCount = 0;
            for ([[maybe_unused]] TopoTestNode* t : llist) {
                newCount++;
            }
            REPORTER_ASSERT(reporter, newCount == expectedOutput.count());

            int j = 0;
            for (TopoTestNode* n : llist) {
                REPORTER_ASSERT(reporter, n == expectedOutput[j++].get());
            }
        }

        //SkDEBUGCODE(print(graph);)
    }
}
