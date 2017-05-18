/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkRandom.h"
#include "SkString.h"
#include "SkTTopoSort.h"

#include "sk_tool_utils.h"

class TopoSortBench : public Benchmark {
public:
    TopoSortBench() { }

    ~TopoSortBench() override {
        sk_tool_utils::TopoTestNode::DeallocNodes(&fGraph);
    }

    bool isSuitableFor(Backend backend) override {
        return kNonRendering_Backend == backend;
    }

protected:
    const char* onGetName() override {
        return "sort_topo_rand";
    }

    // Delayed initialization only done if onDraw will be called.
    void onDelayedSetup() override {
        sk_tool_utils::TopoTestNode::AllocNodes(&fGraph, kNumElements);

        for (int i = kNumElements-1; i > 0; --i) {
            int numEdges = fRand.nextU() % (kMaxEdges+1);

            for (int j = 0; j < numEdges; ++j) {
                int dep = fRand.nextU() % i;

                fGraph[i]->dependsOn(fGraph[dep]);
            }
        }
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; ++i) {
            for (int j = 0; j < fGraph.count(); ++j) {
                fGraph[j]->reset();
            }

            sk_tool_utils::TopoTestNode::Shuffle(&fGraph, &fRand);

            SkDEBUGCODE(bool actualResult =) SkTTopoSort<sk_tool_utils::TopoTestNode>(&fGraph);
            SkASSERT(actualResult);

#ifdef SK_DEBUG
            for (int j = 0; j < fGraph.count(); ++j) {
                SkASSERT(fGraph[j]->check());
            }
#endif
        }
    }

private:
    static const int kNumElements = 1000;
    static const int kMaxEdges = 5;

    SkTDArray<sk_tool_utils::TopoTestNode*> fGraph;
    SkRandom fRand;

    typedef Benchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new TopoSortBench(); )
