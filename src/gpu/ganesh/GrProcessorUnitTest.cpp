/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrProcessorUnitTest.h"

#if defined(GPU_TEST_UTILS)

#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/private/base/SkDebug.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkRandom.h"
#include "src/core/SkColorData.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"

#include <cstdint>
#include <memory>
#include <utility>

using namespace skia_private;

GrProcessorTestData::GrProcessorTestData(SkRandom* random, skgpu::ganesh::SurfaceDrawContext* sdc,
                                         int maxTreeDepth, int numViews, const ViewInfo views[])
        : GrProcessorTestData(random, sdc, maxTreeDepth, numViews, views,
                              /*inputFP=*/nullptr) {}

GrProcessorTestData::GrProcessorTestData(SkRandom* random, skgpu::ganesh::SurfaceDrawContext* sdc,
                                         int maxTreeDepth, int numViews, const ViewInfo views[],
                                         std::unique_ptr<GrFragmentProcessor> inputFP)
        : fRandom(random)
        , fMaxTreeDepth(maxTreeDepth)
        , fContext(sdc->recordingContext())
        , fDrawContext(sdc)
        , fInputFP(std::move(inputFP)) {
    fViews.reset(views, numViews);
    fArena = std::make_unique<SkArenaAlloc>(1000);
}

GrProcessorTestData::~GrProcessorTestData() {}

GrProxyProvider* GrProcessorTestData::proxyProvider() { return fContext->priv().proxyProvider(); }

const GrCaps* GrProcessorTestData::caps() { return fContext->priv().caps(); }

std::unique_ptr<GrFragmentProcessor> GrProcessorTestData::inputFP() {
    if (fCurrentTreeDepth == 0) {
        // At the top level of the tree, provide the input FP from the test data.
        return fInputFP ? fInputFP->clone() : nullptr;
    } else {
        // At deeper levels of recursion, synthesize a random input.
        return GrProcessorUnitTest::MakeChildFP(this);
    }
}

GrProcessorTestData::ViewInfo GrProcessorTestData::randomView() {
    SkASSERT(!fViews.empty());
    return fViews[fRandom->nextULessThan(fViews.size())];
}

GrProcessorTestData::ViewInfo GrProcessorTestData::randomAlphaOnlyView() {
    int numAlphaOnly = 0;
    for (const auto& [v, ct, at] : fViews) {
        if (GrColorTypeIsAlphaOnly(ct)) {
            ++numAlphaOnly;
        }
    }
    SkASSERT(numAlphaOnly);
    int idx = fRandom->nextULessThan(numAlphaOnly);
    for (const auto& [v, ct, at] : fViews) {
        if (GrColorTypeIsAlphaOnly(ct) && !idx--) {
            return {v, ct, at};
        }
    }
    SkUNREACHABLE;
}

template <class ProcessorSmartPtr>
GrProcessorTestFactory<ProcessorSmartPtr>::GrProcessorTestFactory(MakeProc makeProc,
                                                                  const char* name)
        : fMakeProc(makeProc), fName(name) {
    GetFactories()->push_back(this);
}

template <class ProcessorSmartPtr>
ProcessorSmartPtr GrProcessorTestFactory<ProcessorSmartPtr>::Make(GrProcessorTestData* data) {
    VerifyFactoryCount();
    if (GetFactories()->size() == 0) {
        return nullptr;
    }
    uint32_t idx = data->fRandom->nextULessThan(GetFactories()->size());
    return MakeIdx(idx, data);
}

template <class ProcessorSmartPtr>
ProcessorSmartPtr GrProcessorTestFactory<ProcessorSmartPtr>::MakeIdx(int idx,
                                                                     GrProcessorTestData* data) {
    SkASSERT(idx < GetFactories()->size());
    GrProcessorTestFactory<ProcessorSmartPtr>* factory = (*GetFactories())[idx];
    ProcessorSmartPtr processor = factory->fMakeProc(data);
    if (processor == nullptr) {
        SK_ABORT("%s: TestCreate returned null", factory->fName.c_str());
    }
    return processor;
}

template <class ProcessorSmartPtr>
int GrProcessorTestFactory<ProcessorSmartPtr>::Count() {
    return GetFactories()->size();
}

GrXPFactoryTestFactory::GrXPFactoryTestFactory(GetFn* getProc) : fGetProc(getProc) {
    GetFactories()->push_back(this);
}

const GrXPFactory* GrXPFactoryTestFactory::Get(GrProcessorTestData* data) {
    VerifyFactoryCount();
    if (GetFactories()->empty()) {
        return nullptr;
    }
    uint32_t idx = data->fRandom->nextRangeU(0, GetFactories()->size() - 1);
    const GrXPFactory* xpf = (*GetFactories())[idx]->fGetProc(data);
    SkASSERT(xpf);
    return xpf;
}

/*
 * Originally these were both in the processor unit test header, but then it seemed to cause linker
 * problems on android.
 */
template <>
TArray<GrFragmentProcessorTestFactory*, true>* GrFragmentProcessorTestFactory::GetFactories() {
    static TArray<GrFragmentProcessorTestFactory*, true> gFactories;
    return &gFactories;
}

template <>
TArray<GrGeometryProcessorTestFactory*, true>* GrGeometryProcessorTestFactory::GetFactories() {
    static TArray<GrGeometryProcessorTestFactory*, true> gFactories;
    return &gFactories;
}

TArray<GrXPFactoryTestFactory*, true>* GrXPFactoryTestFactory::GetFactories() {
    static TArray<GrXPFactoryTestFactory*, true> gFactories;
    return &gFactories;
}

/*
 * To ensure we always have successful static initialization, before creating from the factories
 * we verify the count is as expected.  If a new factory is added, then these numbers must be
 * manually adjusted.
 */
static constexpr int kFPFactoryCount = 10;
static constexpr int kGPFactoryCount = 14;
static constexpr int kXPFactoryCount = 4;

template <> void GrFragmentProcessorTestFactory::VerifyFactoryCount() {
    if (kFPFactoryCount != GetFactories()->size()) {
        SkDebugf("\nExpected %d fragment processor factories, found %d.\n", kFPFactoryCount,
                 GetFactories()->size());
        SK_ABORT("Wrong number of fragment processor factories!");
    }
}

template <> void GrGeometryProcessorTestFactory::VerifyFactoryCount() {
    if (kGPFactoryCount != GetFactories()->size()) {
        SkDebugf("\nExpected %d geometry processor factories, found %d.\n", kGPFactoryCount,
                 GetFactories()->size());
        SK_ABORT("Wrong number of geometry processor factories!");
    }
}

void GrXPFactoryTestFactory::VerifyFactoryCount() {
    if (kXPFactoryCount != GetFactories()->size()) {
        SkDebugf("\nExpected %d xp factory factories, found %d.\n", kXPFactoryCount,
                 GetFactories()->size());
        SK_ABORT("Wrong number of xp factory factories!");
    }
}

std::unique_ptr<GrFragmentProcessor> GrProcessorUnitTest::MakeChildFP(GrProcessorTestData* data) {
    std::unique_ptr<GrFragmentProcessor> fp;

    ++data->fCurrentTreeDepth;
    if (data->fCurrentTreeDepth > data->fMaxTreeDepth) {
        // We've gone too deep, but we can't necessarily return null without risking an assertion.
        // Instead, return a known-simple zero-child FP. This limits the recursion, and the
        // generated FP will be rejected by the numNonNullChildProcessors check below.
        fp = GrFragmentProcessor::MakeColor(SK_PMColor4fTRANSPARENT);
    } else {
        for (;;) {
            fp = GrFragmentProcessorTestFactory::Make(data);
            SkASSERT(fp);
            // If our tree has already reached its max depth, we must reject FPs that have children.
            if (data->fCurrentTreeDepth < data->fMaxTreeDepth ||
                fp->numNonNullChildProcessors() == 0) {
                break;
            }
        }
    }

    --data->fCurrentTreeDepth;
    return fp;
}

std::unique_ptr<GrFragmentProcessor> GrProcessorUnitTest::MakeOptionalChildFP(
        GrProcessorTestData* data) {
    return data->fRandom->nextBool() ? MakeChildFP(data) : nullptr;
}

template class GrProcessorTestFactory<GrGeometryProcessor*>;
template class GrProcessorTestFactory<std::unique_ptr<GrFragmentProcessor>>;

#endif
