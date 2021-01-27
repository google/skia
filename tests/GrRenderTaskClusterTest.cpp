/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrRenderTaskCluster.h"
#include "src/gpu/mock/GrMockRenderTask.h"
#include "src/gpu/mock/GrMockSurfaceProxy.h"
#include "tests/Test.h"

typedef void (*CreateGraphPF)(SkTArray<sk_sp<GrMockRenderTask>>* graph,
                              SkTArray<sk_sp<GrMockRenderTask>>* expected);

static void make_proxies(int count, SkTArray<sk_sp<GrSurfaceProxy>>* proxies) {
    proxies->reset(count);
    for (int i = 0; i < count; i++) {
        auto name = SkStringPrintf("%c", 'A' + i);
        proxies->at(i) = sk_make_sp<GrMockSurfaceProxy>(std::move(name));
    }
}

static void make_tasks(int count, SkTArray<sk_sp<GrMockRenderTask>>* tasks) {
    tasks->reset(count);
    for (int i = 0; i < count; i++) {
        tasks->at(i) = sk_make_sp<GrMockRenderTask>();
    }
}

/*
 * In:  A1 B1 A2
 * Out: B1 A1 A2
 */
static void create_graph0(SkTArray<sk_sp<GrMockRenderTask>>* graph,
                          SkTArray<sk_sp<GrMockRenderTask>>* expected) {
    SkTArray<sk_sp<GrSurfaceProxy>> proxies;
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
static void create_graph1(SkTArray<sk_sp<GrMockRenderTask>>* graph,
                          SkTArray<sk_sp<GrMockRenderTask>>* expected) {
    SkTArray<sk_sp<GrSurfaceProxy>> proxies;
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
static void create_graph2(SkTArray<sk_sp<GrMockRenderTask>>* graph,
                          SkTArray<sk_sp<GrMockRenderTask>>* expected) {
    SkTArray<sk_sp<GrSurfaceProxy>> proxies;
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
static void create_graph3(SkTArray<sk_sp<GrMockRenderTask>>* graph,
                          SkTArray<sk_sp<GrMockRenderTask>>* expected) {
    SkTArray<sk_sp<GrSurfaceProxy>> proxies;
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

    for (size_t i = 0; i < SK_ARRAY_COUNT(tests); ++i) {
        SkTArray<sk_sp<GrMockRenderTask>> graph;
        SkTArray<sk_sp<GrMockRenderTask>> expectedOutput;

        (tests[i])(&graph, &expectedOutput);

        SkTInternalLList<GrRenderTask> llist;
        // TODO: Why does Span not want to convert from sk_sp<GrMockRenderTask> to
        // `const sk_sp<GrRenderTask>`?
        SkSpan<const sk_sp<GrRenderTask>> graphSpan(
            reinterpret_cast<sk_sp<GrRenderTask>*>(graph.data()), graph.count());
        bool actualResult = GrClusterRenderTasks(graphSpan, &llist);

        if (expectedOutput.empty()) {
            REPORTER_ASSERT(reporter, !actualResult);
        } else {
            REPORTER_ASSERT(reporter, actualResult);
            // SkTInternalLList::countEntries is debug-only and these tests run in release.
            int newCount = 0;
            for ([[maybe_unused]] GrRenderTask* t : llist) {
                newCount++;
            }
            REPORTER_ASSERT(reporter, newCount == expectedOutput.count());

            int j = 0;
            for (GrRenderTask* n : llist) {
                REPORTER_ASSERT(reporter, n == expectedOutput[j++].get());
            }
        }

        //SkDEBUGCODE(print(graph);)
    }
}
