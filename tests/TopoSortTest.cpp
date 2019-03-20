/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRandom.h"
#include "SkTTopoSort.h"
#include "Test.h"

#include "ToolUtils.h"

typedef void (*CreateGraphPF)(SkTArray<sk_sp<ToolUtils::TopoTestNode>>* graph);

/* Simple diamond
 *       3
 *     /   \
 *    1     2
 *     \   /
 *       0
 */
static void create_graph0(SkTArray<sk_sp<ToolUtils::TopoTestNode>>* graph) {
    ToolUtils::TopoTestNode::AllocNodes(graph, 4);

    (*graph)[0]->dependsOn((*graph)[1].get());
    (*graph)[0]->dependsOn((*graph)[2].get());
    (*graph)[1]->dependsOn((*graph)[3].get());
    (*graph)[2]->dependsOn((*graph)[3].get());
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
static void create_graph1(SkTArray<sk_sp<ToolUtils::TopoTestNode>>* graph) {
    ToolUtils::TopoTestNode::AllocNodes(graph, 4);

    (*graph)[0]->dependsOn((*graph)[1].get());
    (*graph)[1]->dependsOn((*graph)[2].get());
    (*graph)[2]->dependsOn((*graph)[3].get());
}

/* Loop
 *       2
 *     /   \
 *    0 --- 1
 */
static void create_graph2(SkTArray<sk_sp<ToolUtils::TopoTestNode>>* graph) {
    ToolUtils::TopoTestNode::AllocNodes(graph, 3);

    (*graph)[0]->dependsOn((*graph)[1].get());
    (*graph)[1]->dependsOn((*graph)[2].get());
    (*graph)[2]->dependsOn((*graph)[0].get());
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
static void create_graph3(SkTArray<sk_sp<ToolUtils::TopoTestNode>>* graph) {
    ToolUtils::TopoTestNode::AllocNodes(graph, 7);

    (*graph)[0]->dependsOn((*graph)[1].get());
    (*graph)[0]->dependsOn((*graph)[2].get());
    (*graph)[1]->dependsOn((*graph)[3].get());
    (*graph)[2]->dependsOn((*graph)[3].get());

    (*graph)[3]->dependsOn((*graph)[4].get());
    (*graph)[3]->dependsOn((*graph)[5].get());
    (*graph)[4]->dependsOn((*graph)[6].get());
    (*graph)[5]->dependsOn((*graph)[6].get());
}

/* Two independent diamonds
 *       3           7
 *     /   \       /   \
 *    1     2     5     6
 *     \   /       \   /
 *       0           4
 */
static void create_graph4(SkTArray<sk_sp<ToolUtils::TopoTestNode>>* graph) {
    ToolUtils::TopoTestNode::AllocNodes(graph, 8);

    (*graph)[0]->dependsOn((*graph)[1].get());
    (*graph)[0]->dependsOn((*graph)[2].get());
    (*graph)[1]->dependsOn((*graph)[3].get());
    (*graph)[2]->dependsOn((*graph)[3].get());

    (*graph)[4]->dependsOn((*graph)[5].get());
    (*graph)[4]->dependsOn((*graph)[6].get());
    (*graph)[5]->dependsOn((*graph)[7].get());
    (*graph)[6]->dependsOn((*graph)[7].get());
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
        SkTArray<sk_sp<ToolUtils::TopoTestNode>> graph;

        (tests[i].fCreate)(&graph);

        ToolUtils::TopoTestNode::Shuffle(&graph, &rand);

        bool actualResult = SkTTopoSort<ToolUtils::TopoTestNode>(&graph);
        REPORTER_ASSERT(reporter, actualResult == tests[i].fExpectedResult);

        if (tests[i].fExpectedResult) {
            for (int j = 0; j < graph.count(); ++j) {
                REPORTER_ASSERT(reporter, graph[j]->check());
            }
        }

        //SkDEBUGCODE(print(graph);)
    }
}
