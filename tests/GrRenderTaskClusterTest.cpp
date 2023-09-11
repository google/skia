/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkTInternalLList.h"
#include "src/gpu/ganesh/GrRenderTask.h"
#include "src/gpu/ganesh/GrRenderTaskCluster.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/mock/GrMockRenderTask.h"
#include "src/gpu/ganesh/mock/GrMockSurfaceProxy.h"
#include "tests/Test.h"

#include <array>
#include <cstddef>
#include <utility>

using namespace skia_private;

typedef void (*CreateGraphPF)(TArray<sk_sp<GrMockRenderTask>>* graph,
                              TArray<sk_sp<GrMockRenderTask>>* expected);

static void make_proxies(int count, TArray<sk_sp<GrSurfaceProxy>>* proxies) {
    proxies->reset(count);
    for (int i = 0; i < count; i++) {
        auto name = SkStringPrintf("%c", 'A' + i);
        proxies->at(i) = sk_make_sp<GrMockSurfaceProxy>(std::move(name),
        /*label=*/"RenderTaskClusterTest");
    }
}

static void make_tasks(int count, TArray<sk_sp<GrMockRenderTask>>* tasks) {
    tasks->reset(count);
    for (int i = 0; i < count; i++) {
        tasks->at(i) = sk_make_sp<GrMockRenderTask>();
    }
}

/*
 * In:  A1 B1 A2
 * Out: B1 A1 A2
 */
static void create_graph0(TArray<sk_sp<GrMockRenderTask>>* graph,
                          TArray<sk_sp<GrMockRenderTask>>* expected) {
    TArray<sk_sp<GrSurfaceProxy>> proxies;
    make_proxies(2, &proxies);
    make_tasks(3, graph);

    graph->at(0)->addTarget(proxies[0]);
    graph->at(1)->addTarget(proxies[1]);
    graph->at(2)->addTarget(proxies[0]);
    graph->at(2)->addDependency(graph->at(1).get());

    expected->push_back(graph->at(1));
    expected->push_back(graph->at(0));
    expected->push_back(graph->at(2));
}

/*
 * In:  A1 B1 A2 C1 A3
 * Out: B1 C1 A1 A2 A3
 */
static void create_graph1(TArray<sk_sp<GrMockRenderTask>>* graph,
                          TArray<sk_sp<GrMockRenderTask>>* expected) {
    TArray<sk_sp<GrSurfaceProxy>> proxies;
    make_proxies(3, &proxies);
    make_tasks(5, graph);

    graph->at(0)->addTarget(proxies[0]);
    graph->at(1)->addTarget(proxies[1]);
    graph->at(2)->addTarget(proxies[0]);
    graph->at(3)->addTarget(proxies[2]);
    graph->at(4)->addTarget(proxies[0]);

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
static void create_graph2(TArray<sk_sp<GrMockRenderTask>>* graph,
                          TArray<sk_sp<GrMockRenderTask>>* expected) {
    TArray<sk_sp<GrSurfaceProxy>> proxies;
    make_proxies(2, &proxies);
    make_tasks(3, graph);

    graph->at(0)->addTarget(proxies[0]);
    graph->at(1)->addTarget(proxies[1]);
    graph->at(2)->addTarget(proxies[0]);

    graph->at(1)->addDependency(graph->at(0).get());
    graph->at(2)->addDependency(graph->at(1).get());

    // expected is empty. Can't reorder.
}

/*
 * Write-after-read case.
 * In:   A1 B1 A2 B2
 * Srcs: A1->B1, A2->B2
 * Used: B1(A), B2(A)
 * Out:  Can't reorder.
 */
static void create_graph3(TArray<sk_sp<GrMockRenderTask>>* graph,
                          TArray<sk_sp<GrMockRenderTask>>* expected) {
    TArray<sk_sp<GrSurfaceProxy>> proxies;
    make_proxies(2, &proxies);
    make_tasks(4, graph);

    graph->at(0)->addTarget(proxies[0]);
    graph->at(1)->addTarget(proxies[1]);
    graph->at(2)->addTarget(proxies[0]);
    graph->at(3)->addTarget(proxies[1]);

    graph->at(1)->addDependency(graph->at(0).get());
    graph->at(3)->addDependency(graph->at(2).get());

    graph->at(1)->addUsed(proxies[0]);
    graph->at(3)->addUsed(proxies[0]);

    // expected is empty. Can't reorder.
}

DEF_TEST(GrRenderTaskCluster, reporter) {
    CreateGraphPF tests[] = {
        create_graph0,
        create_graph1,
        create_graph2,
        create_graph3
    };

    for (size_t i = 0; i < std::size(tests); ++i) {
        TArray<sk_sp<GrMockRenderTask>> graph;
        TArray<sk_sp<GrMockRenderTask>> expectedOutput;

        (tests[i])(&graph, &expectedOutput);

        SkTInternalLList<GrRenderTask> llist;
        // TODO: Why does Span not want to convert from sk_sp<GrMockRenderTask> to
        // `const sk_sp<GrRenderTask>`?
        SkSpan<const sk_sp<GrRenderTask>> graphSpan(
            reinterpret_cast<sk_sp<GrRenderTask>*>(graph.data()), graph.size());
        bool actualResult = GrClusterRenderTasks(graphSpan, &llist);

        if (expectedOutput.empty()) {
            REPORTER_ASSERT(reporter, !actualResult);
            size_t newCount = 0;
            for (const GrRenderTask* t : llist) {
                REPORTER_ASSERT(reporter, newCount < graphSpan.size() &&
                                          t == graph[newCount].get());
                ++newCount;
            }
            REPORTER_ASSERT(reporter, newCount == graphSpan.size());
        } else {
            REPORTER_ASSERT(reporter, actualResult);
            // SkTInternalLList::countEntries is debug-only and these tests run in release.
            int newCount = 0;
            for ([[maybe_unused]] GrRenderTask* t : llist) {
                newCount++;
            }
            REPORTER_ASSERT(reporter, newCount == expectedOutput.size());

            int j = 0;
            for (GrRenderTask* n : llist) {
                REPORTER_ASSERT(reporter, n == expectedOutput[j++].get());
            }
        }

        //SkDEBUGCODE(print(graph);)
    }
}
