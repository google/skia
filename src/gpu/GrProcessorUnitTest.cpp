/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrProcessorUnitTest.h"

#include "include/gpu/GrContext.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrFragmentProcessor.h"

#if GR_TEST_UTILS

GrProcessorTestData::GrProcessorTestData(SkRandom* random,
                                         GrContext* context,
                                         int numViews,
                                         const ViewInfo views[])
        : fRandom(random), fContext(context) {
    fViews.reset(views, numViews);
    fArena = std::unique_ptr<SkArenaAlloc>(new SkArenaAlloc(1000));
}

GrResourceProvider* GrProcessorTestData::resourceProvider() {
    return fContext->priv().resourceProvider();
}

GrProxyProvider* GrProcessorTestData::proxyProvider() { return fContext->priv().proxyProvider(); }

const GrCaps* GrProcessorTestData::caps() { return fContext->priv().caps(); }

GrProcessorTestData::ViewInfo GrProcessorTestData::randomView() {
    SkASSERT(!fViews.empty());
    return fViews[fRandom->nextULessThan(fViews.count())];
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

class GrFragmentProcessor;
class GrGeometryProcessor;

/*
 * Originally these were both in the processor unit test header, but then it seemed to cause linker
 * problems on android.
 */
template <>
SkTArray<GrFragmentProcessorTestFactory*, true>* GrFragmentProcessorTestFactory::GetFactories() {
    static SkTArray<GrFragmentProcessorTestFactory*, true> gFactories;
    return &gFactories;
}

template <>
SkTArray<GrGeometryProcessorTestFactory*, true>* GrGeometryProcessorTestFactory::GetFactories() {
    static SkTArray<GrGeometryProcessorTestFactory*, true> gFactories;
    return &gFactories;
}

SkTArray<GrXPFactoryTestFactory*, true>* GrXPFactoryTestFactory::GetFactories() {
    static SkTArray<GrXPFactoryTestFactory*, true> gFactories;
    return &gFactories;
}

/*
 * To ensure we always have successful static initialization, before creating from the factories
 * we verify the count is as expected.  If a new factory is added, then these numbers must be
 * manually adjusted.
 */
static const int kFPFactoryCount = 35;
static const int kGPFactoryCount = 14;
static const int kXPFactoryCount = 4;

template <> void GrFragmentProcessorTestFactory::VerifyFactoryCount() {
    if (kFPFactoryCount != GetFactories()->count()) {
        SkDebugf("\nExpected %d fragment processor factories, found %d.\n", kFPFactoryCount,
                 GetFactories()->count());
        SK_ABORT("Wrong number of fragment processor factories!");
    }
}

template <> void GrGeometryProcessorTestFactory::VerifyFactoryCount() {
    if (kGPFactoryCount != GetFactories()->count()) {
        SkDebugf("\nExpected %d geometry processor factories, found %d.\n", kGPFactoryCount,
                 GetFactories()->count());
        SK_ABORT("Wrong number of geometry processor factories!");
    }
}

void GrXPFactoryTestFactory::VerifyFactoryCount() {
    if (kXPFactoryCount != GetFactories()->count()) {
        SkDebugf("\nExpected %d xp factory factories, found %d.\n", kXPFactoryCount,
                 GetFactories()->count());
        SK_ABORT("Wrong number of xp factory factories!");
    }
}

std::unique_ptr<GrFragmentProcessor> GrProcessorUnitTest::MakeChildFP(GrProcessorTestData* data) {
    std::unique_ptr<GrFragmentProcessor> fp;
    do {
        fp = GrFragmentProcessorTestFactory::Make(data);
        SkASSERT(fp);
    } while (fp->numChildProcessors() != 0);
    return fp;
}
#endif
