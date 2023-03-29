/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkRandom.h"
#include "src/gpu/ganesh/GrTTopoSort.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"

#include <cstddef>
#include <vector>

using namespace skia_private;

typedef void (*CreateGraphPF)(TArray<sk_sp<ToolUtils::TopoTestNode>>* graph);

/* Simple diamond
 *       3
 *      . .
 *     /   \
 *    1     2
 *    .     .
 *     \   /
 *       0
 */
static void create_graph0(TArray<sk_sp<ToolUtils::TopoTestNode>>* graph) {
    ToolUtils::TopoTestNode::AllocNodes(graph, 4);

    (*graph)[0]->dependsOn((*graph)[1].get());
    (*graph)[0]->dependsOn((*graph)[2].get());
    (*graph)[1]->dependsOn((*graph)[3].get());
    (*graph)[2]->dependsOn((*graph)[3].get());
}

static void create_simple_chain(TArray<sk_sp<ToolUtils::TopoTestNode>>* graph, int n) {
    ToolUtils::TopoTestNode::AllocNodes(graph, n);

    for (int i = 0; i < n - 1; ++i) {
        (*graph)[i+1]->dependsOn((*graph)[i].get());
    }
}

/* Simple chain
 *     0
 *     ^
 *     |
 *     1
 *     ^
 *     |
 *     2
 *     ^
 *     |
 *     3
 */
static void create_graph1(TArray<sk_sp<ToolUtils::TopoTestNode>>* graph) {
    create_simple_chain(graph, 4);
}

/* Simple Loop
 *       2
 *      / .
 *     /   \
 *    .     \
 *    0 ---> 1
 */
static void create_graph2(TArray<sk_sp<ToolUtils::TopoTestNode>>* graph) {
    ToolUtils::TopoTestNode::AllocNodes(graph, 3);

    (*graph)[0]->dependsOn((*graph)[1].get());
    (*graph)[1]->dependsOn((*graph)[2].get());
    (*graph)[2]->dependsOn((*graph)[0].get());
}

/* Double diamond
 *       6
 *      . .
 *     /   \
 *    4     5
 *    .     .
 *     \   /
 *       3
 *      . .
 *     /   \
 *    1     2
 *    .     .
 *     \   /
 *       0
 */
static void create_graph3(TArray<sk_sp<ToolUtils::TopoTestNode>>* graph) {
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
 *      . .         . .
 *     /   \       /   \
 *    1     2     5     6
 *    .     .     .     .
 *     \   /       \   /
 *       0           4
 */
static void create_graph4(TArray<sk_sp<ToolUtils::TopoTestNode>>* graph) {
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

/* Two linked diamonds w/ two loops
 *       5     6
 *      / .   . \
 *     .   \ /   .
 *    2     3     4
 *     \    .    /
 *      .  / \  .
 *       0     1
 */
static void create_graph5(TArray<sk_sp<ToolUtils::TopoTestNode>>* graph) {
    ToolUtils::TopoTestNode::AllocNodes(graph, 7);

    (*graph)[0]->dependsOn((*graph)[3].get());
    (*graph)[1]->dependsOn((*graph)[3].get());
    (*graph)[2]->dependsOn((*graph)[0].get());
    (*graph)[3]->dependsOn((*graph)[5].get());
    (*graph)[3]->dependsOn((*graph)[6].get());
    (*graph)[4]->dependsOn((*graph)[1].get());
    (*graph)[5]->dependsOn((*graph)[2].get());
    (*graph)[6]->dependsOn((*graph)[4].get());
}

/* Two disjoint loops
 *       2          5
 *      / .        / .
 *     /   \      /   \
 *    .     \    .     \
 *    0 ---> 1   3 ---> 4
 */
static void create_graph6(TArray<sk_sp<ToolUtils::TopoTestNode>>* graph) {
    ToolUtils::TopoTestNode::AllocNodes(graph, 6);

    (*graph)[0]->dependsOn((*graph)[1].get());
    (*graph)[1]->dependsOn((*graph)[2].get());
    (*graph)[2]->dependsOn((*graph)[0].get());

    (*graph)[3]->dependsOn((*graph)[4].get());
    (*graph)[4]->dependsOn((*graph)[5].get());
    (*graph)[5]->dependsOn((*graph)[3].get());
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
        { create_graph5, false },
        { create_graph6, false },
    };

    for (size_t i = 0; i < std::size(tests); ++i) {
        TArray<sk_sp<ToolUtils::TopoTestNode>> graph;

        (tests[i].fCreate)(&graph);

        const int numNodes = graph.size();

        ToolUtils::TopoTestNode::Shuffle(graph, &rand);

        bool actualResult = GrTTopoSort<ToolUtils::TopoTestNode>(graph);
        REPORTER_ASSERT(reporter, actualResult == tests[i].fExpectedResult);
        REPORTER_ASSERT(reporter, numNodes == graph.size());

        if (tests[i].fExpectedResult) {
            for (const auto& node : graph) {
                REPORTER_ASSERT(reporter, node->check());
            }
        } else {
            // When the topological sort fails all the nodes should still appear in the result
            // but their order can be somewhat arbitrary.
            std::vector<bool> seen(numNodes, false);

            for (const auto& node : graph) {
                SkASSERT(node);
                SkASSERT(!seen[node->id()]);
                seen[node->id()] = true;
            }
        }

        //SkDEBUGCODE(print(graph);)
    }

    // Some additional tests that do multiple partial sorts of graphs where we know nothing in an
    // earlier partion depends on anything in a later partition.
    for (int n = 2; n < 6; ++n) {
        for (int split = 1; split < n; ++split) {
            TArray<sk_sp<ToolUtils::TopoTestNode>> graph;
            create_simple_chain(&graph, n);
            SkSpan spanA = SkSpan(graph.begin(), split);
            SkSpan spanB = SkSpan(graph.begin() + split, n - split);
            ToolUtils::TopoTestNode::Shuffle(spanA, &rand);
            ToolUtils::TopoTestNode::Shuffle(spanB, &rand);
            bool result = GrTTopoSort(spanA);
            if (!result) {
                ERRORF(reporter, "Topo sort on partial chain failed.");
                return;
            }
            // Nothing outside of the processed span should have been output.
            for (const auto& node : spanB) {
                REPORTER_ASSERT(reporter, !ToolUtils::TopoTestNode::WasOutput(node.get()));
            }
            result = GrTTopoSort(spanB, split);
            if (!result) {
                ERRORF(reporter, "Topo sort on partial chain failed.");
                return;
            }
            for (const auto& node : graph) {
                REPORTER_ASSERT(reporter, node->check());
            }
        }
    }
}
