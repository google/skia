/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRandom.h"
#include "SkTTopoSort.h"
#include "Test.h"

#include "sk_tool_utils.h"

typedef void (*CreateGraphPF)(SkTDArray<sk_tool_utils::TopoTestNode*>* graph);

/* Simple diamond
 *       3
 *     /   \
 *    1     2
 *     \   /
 *       0
 */
static void create_graph0(SkTDArray<sk_tool_utils::TopoTestNode*>* graph) {
    sk_tool_utils::TopoTestNode::AllocNodes(graph, 4);

    (*graph)[0]->dependsOn((*graph)[1]);
    (*graph)[0]->dependsOn((*graph)[2]);
    (*graph)[1]->dependsOn((*graph)[3]);
    (*graph)[2]->dependsOn((*graph)[3]);
}

/* Simple chain
 *     3
 *     |
 *     2
 *     |
 *     1
 *     |
 *     0
 */
static void create_graph1(SkTDArray<sk_tool_utils::TopoTestNode*>* graph) {
    sk_tool_utils::TopoTestNode::AllocNodes(graph, 4);

    (*graph)[0]->dependsOn((*graph)[1]);
    (*graph)[1]->dependsOn((*graph)[2]);
    (*graph)[2]->dependsOn((*graph)[3]);
}

/* Loop
 *       2
 *     /   \
 *    0 --- 1
 */
static void create_graph2(SkTDArray<sk_tool_utils::TopoTestNode*>* graph) {
    sk_tool_utils::TopoTestNode::AllocNodes(graph, 3);

    (*graph)[0]->dependsOn((*graph)[1]);
    (*graph)[1]->dependsOn((*graph)[2]);
    (*graph)[2]->dependsOn((*graph)[0]);
}

/* Double diamond
 *       6
 *     /   \
 *    4     5
 *     \   /
 *       3
 *     /   \
 *    1     2
 *     \   /
 *       0
 */
static void create_graph3(SkTDArray<sk_tool_utils::TopoTestNode*>* graph) {
    sk_tool_utils::TopoTestNode::AllocNodes(graph, 7);

    (*graph)[0]->dependsOn((*graph)[1]);
    (*graph)[0]->dependsOn((*graph)[2]);
    (*graph)[1]->dependsOn((*graph)[3]);
    (*graph)[2]->dependsOn((*graph)[3]);

    (*graph)[3]->dependsOn((*graph)[4]);
    (*graph)[3]->dependsOn((*graph)[5]);
    (*graph)[4]->dependsOn((*graph)[6]);
    (*graph)[5]->dependsOn((*graph)[6]);
}

/* Two independent diamonds
 *       3           7
 *     /   \       /   \
 *    1     2     5     6
 *     \   /       \   /
 *       0           4
 */
static void create_graph4(SkTDArray<sk_tool_utils::TopoTestNode*>* graph) {
    sk_tool_utils::TopoTestNode::AllocNodes(graph, 8);

    (*graph)[0]->dependsOn((*graph)[1]);
    (*graph)[0]->dependsOn((*graph)[2]);
    (*graph)[1]->dependsOn((*graph)[3]);
    (*graph)[2]->dependsOn((*graph)[3]);

    (*graph)[4]->dependsOn((*graph)[5]);
    (*graph)[4]->dependsOn((*graph)[6]);
    (*graph)[5]->dependsOn((*graph)[7]);
    (*graph)[6]->dependsOn((*graph)[7]);
}

DEF_TEST(TopoSort, reporter) {
    SkRandom rand;

    struct {
        CreateGraphPF fCreate;
        bool          fExpectedResult;
    } tests[] = {
        { create_graph0, true  },
        { create_graph1, true  },
        { create_graph2, false },
        { create_graph3, true  },
        { create_graph4, true  },
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(tests); ++i) {
        SkTDArray<sk_tool_utils::TopoTestNode*> graph;

        (tests[i].fCreate)(&graph);

        sk_tool_utils::TopoTestNode::Shuffle(&graph, &rand);

        bool actualResult = SkTTopoSort<sk_tool_utils::TopoTestNode>(&graph);
        REPORTER_ASSERT(reporter, actualResult == tests[i].fExpectedResult);

        if (tests[i].fExpectedResult) {
            for (int j = 0; j < graph.count(); ++j) {
                REPORTER_ASSERT(reporter, graph[j]->check());
            }
        }

        //SkDEBUGCODE(print(graph);)
        sk_tool_utils::TopoTestNode::DeallocNodes(&graph);
    }
}
