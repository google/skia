/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkRandom.h"
#include "SkString.h"
#if SK_SUPPORT_GPU
#include "GrOrderedSet.h"

static const int NUM_ELEMENTS = 1000;

// Time how long it takes to build a set
class GrOrderedSetBuildBench : public Benchmark {
public:
    GrOrderedSetBuildBench() {
        fName.append("ordered_set_build");
    }

    bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return kNonRendering_Backend == backend;
    }

    virtual ~GrOrderedSetBuildBench() {}

protected:
    const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    void onPreDraw() SK_OVERRIDE {
        SkRandom rand;
        for (int j = 0; j < NUM_ELEMENTS; ++j) {
            fData[j] = rand.nextU() % NUM_ELEMENTS;
        }
    }

    void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        for (int i = 0; i < loops; ++i) {
            GrOrderedSet<int> set;
            for (int j = 0; j < NUM_ELEMENTS; ++j) {
                set.insert(fData[j]);
            }
            set.reset();
        }
    }

private:
    SkString fName;
    int fData[NUM_ELEMENTS];
    typedef Benchmark INHERITED;
};

// Time how long it takes to find elements in a set
class GrOrderedSetFindBench : public Benchmark {
public:
    GrOrderedSetFindBench() {
        fName.append("ordered_set_find");
    }

    bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return kNonRendering_Backend == backend;
    }

    virtual ~GrOrderedSetFindBench() {}

protected:
    const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    void onPreDraw() SK_OVERRIDE {
        SkRandom rand;
        for (int j = 0; j < NUM_ELEMENTS; ++j) {
            fData[j] = rand.nextU() % 1500;
            fSet.insert(rand.nextU() % NUM_ELEMENTS);
        }
    }

    void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        for (int i = 0; i < loops; ++i) {
            for (int j = 0; j < NUM_ELEMENTS; ++j) {
                fSet.find(fData[j]);
            }
        }
    }

private:
    SkString fName;
    int fData[NUM_ELEMENTS];
    GrOrderedSet<int> fSet;
    typedef Benchmark INHERITED;
};

// Time how long it takes to iterate over and remove all elements from set
class GrOrderedSetRemoveBench : public Benchmark {
public:
    GrOrderedSetRemoveBench() {
        fName.append("ordered_set_remove");
    }

    bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return kNonRendering_Backend == backend;
    }

    virtual ~GrOrderedSetRemoveBench() {}

protected:
    const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    void onPreDraw() SK_OVERRIDE {
        SkRandom rand;
        for (int j = 0; j < NUM_ELEMENTS; ++j) {
            fSet.insert(rand.nextU() % NUM_ELEMENTS);
        }
    }

    void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        typedef GrOrderedSet<int>::Iter SetIter;
        for (int i = 0; i < loops; ++i) {
            GrOrderedSet<int> testSet;
            for (SetIter s = fSet.begin(); fSet.end() != s; ++s) {
                testSet.insert(*s);
            }
            for (int j = 0; j < NUM_ELEMENTS; ++j) {
                testSet.remove(testSet.find(j));
            }
        }
    }

private:
    SkString fName;
    GrOrderedSet<int> fSet;
    typedef Benchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH(return SkNEW_ARGS(GrOrderedSetBuildBench, ());)
DEF_BENCH(return SkNEW_ARGS(GrOrderedSetFindBench, ());)
DEF_BENCH(return SkNEW_ARGS(GrOrderedSetRemoveBench, ());)
#endif
