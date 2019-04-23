/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrContext.h"
#include "include/gpu/GrSamplerState.h"
#include "include/private/GrTextureProxy.h"
#include "include/private/SkSpinlock.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrProcessor.h"
#include "src/gpu/GrXferProcessor.h"

#if GR_TEST_UTILS

GrResourceProvider* GrProcessorTestData::resourceProvider() {
    return fContext->priv().resourceProvider();
}

GrProxyProvider* GrProcessorTestData::proxyProvider() {
    return fContext->priv().proxyProvider();
}

const GrCaps* GrProcessorTestData::caps() { return fContext->priv().caps(); }

#if SK_ALLOW_STATIC_GLOBAL_INITIALIZERS
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
static const int kFPFactoryCount = 36;
static const int kGPFactoryCount = 14;
static const int kXPFactoryCount = 4;

template <>
void GrFragmentProcessorTestFactory::VerifyFactoryCount() {
    if (kFPFactoryCount != GetFactories()->count()) {
        SkDebugf("\nExpected %d fragment processor factories, found %d.\n",
                 kFPFactoryCount, GetFactories()->count());
        SK_ABORT("Wrong number of fragment processor factories!");
    }
}

template <>
void GrGeometryProcessorTestFactory::VerifyFactoryCount() {
    if (kGPFactoryCount != GetFactories()->count()) {
        SkDebugf("\nExpected %d geometry processor factories, found %d.\n",
                 kGPFactoryCount, GetFactories()->count());
        SK_ABORT("Wrong number of geometry processor factories!");
    }
}

void GrXPFactoryTestFactory::VerifyFactoryCount() {
    if (kXPFactoryCount != GetFactories()->count()) {
        SkDebugf("\nExpected %d xp factory factories, found %d.\n",
                 kXPFactoryCount, GetFactories()->count());
        SK_ABORT("Wrong number of xp factory factories!");
    }
}

#endif
#endif


// We use a global pool protected by a mutex(spinlock). Chrome may use the same GrContext on
// different threads. The GrContext is not used concurrently on different threads and there is a
// memory barrier between accesses of a context on different threads. Also, there may be multiple
// GrContexts and those contexts may be in use concurrently on different threads.
namespace {
#if !defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
static SkSpinlock gProcessorSpinlock;
#endif
class MemoryPoolAccessor {
public:

// We know in the Android framework there is only one GrContext.
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
    MemoryPoolAccessor() {}
    ~MemoryPoolAccessor() {}
#else
    MemoryPoolAccessor() { gProcessorSpinlock.acquire(); }
    ~MemoryPoolAccessor() { gProcessorSpinlock.release(); }
#endif

    GrMemoryPool* pool() const {
        static GrMemoryPool gPool(4096, 4096);
        return &gPool;
    }
};
}

///////////////////////////////////////////////////////////////////////////////

void* GrProcessor::operator new(size_t size) { return MemoryPoolAccessor().pool()->allocate(size); }

void GrProcessor::operator delete(void* target) {
    return MemoryPoolAccessor().pool()->release(target);
}
