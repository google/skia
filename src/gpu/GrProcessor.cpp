/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrProcessor.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGeometryProcessor.h"
#include "GrMemoryPool.h"
#include "GrSamplerState.h"
#include "GrTextureProxy.h"
#include "GrXferProcessor.h"
#include "SkSpinlock.h"

#if GR_TEST_UTILS

GrResourceProvider* GrProcessorTestData::resourceProvider() {
    return fContext->contextPriv().resourceProvider();
}

GrProxyProvider* GrProcessorTestData::proxyProvider() {
    return fContext->contextPriv().proxyProvider();
}

const GrCaps* GrProcessorTestData::caps() {
    return fContext->caps();
}

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
static const int kFPFactoryCount = 38;
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

///////////////////////////////////////////////////////////////////////////////

void GrResourceIOProcessor::addTextureSampler(const TextureSampler* access) {
    fTextureSamplers.push_back(access);
}

void GrResourceIOProcessor::addBufferAccess(const BufferAccess* access) {
    fBufferAccesses.push_back(access);
}

void GrResourceIOProcessor::addPendingIOs() const {
    for (const auto& sampler : fTextureSamplers) {
        sampler->programProxy()->markPendingIO();
    }
    for (const auto& buffer : fBufferAccesses) {
        buffer->programBuffer()->markPendingIO();
    }
}

void GrResourceIOProcessor::removeRefs() const {
    for (const auto& sampler : fTextureSamplers) {
        sampler->programProxy()->removeRef();
    }
    for (const auto& buffer : fBufferAccesses) {
        buffer->programBuffer()->removeRef();
    }
}

void GrResourceIOProcessor::pendingIOComplete() const {
    for (const auto& sampler : fTextureSamplers) {
        sampler->programProxy()->pendingIOComplete();
    }
    for (const auto& buffer : fBufferAccesses) {
        buffer->programBuffer()->pendingIOComplete();
    }
}

bool GrResourceIOProcessor::instantiate(GrResourceProvider* resourceProvider) const {
    for (const auto& sampler : fTextureSamplers) {
        if (!sampler->instantiate(resourceProvider)) {
            return false;
        }
    }

    // MDB TODO: instantiate 'fBufferAccesses' here as well

    return true;
}

bool GrResourceIOProcessor::hasSameSamplersAndAccesses(const GrResourceIOProcessor& that) const {
    if (this->numTextureSamplers() != that.numTextureSamplers() ||
        this->numBuffers() != that.numBuffers()) {
        return false;
    }
    for (int i = 0; i < this->numTextureSamplers(); ++i) {
        if (this->textureSampler(i) != that.textureSampler(i)) {
            return false;
        }
    }
    for (int i = 0; i < this->numBuffers(); ++i) {
        if (this->bufferAccess(i) != that.bufferAccess(i)) {
            return false;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

GrResourceIOProcessor::TextureSampler::TextureSampler() {}

GrResourceIOProcessor::TextureSampler::TextureSampler(sk_sp<GrTextureProxy> proxy,
                                                      const GrSamplerState& samplerState) {
    this->reset(std::move(proxy), samplerState);
}

GrResourceIOProcessor::TextureSampler::TextureSampler(sk_sp<GrTextureProxy> proxy,
                                                      GrSamplerState::Filter filterMode,
                                                      GrSamplerState::WrapMode wrapXAndY,
                                                      GrShaderFlags visibility) {
    this->reset(std::move(proxy), filterMode, wrapXAndY, visibility);
}

void GrResourceIOProcessor::TextureSampler::reset(sk_sp<GrTextureProxy> proxy,
                                                  const GrSamplerState& samplerState,
                                                  GrShaderFlags visibility) {
    fSamplerState = samplerState;
    fProxyRef.setProxy(std::move(proxy), kRead_GrIOType);
    fSamplerState.setFilterMode(SkTMin(samplerState.filter(), this->proxy()->highestFilterMode()));
    fVisibility = visibility;
}

void GrResourceIOProcessor::TextureSampler::reset(sk_sp<GrTextureProxy> proxy,
                                                  GrSamplerState::Filter filterMode,
                                                  GrSamplerState::WrapMode wrapXAndY,
                                                  GrShaderFlags visibility) {
    fProxyRef.setProxy(std::move(proxy), kRead_GrIOType);
    filterMode = SkTMin(filterMode, this->proxy()->highestFilterMode());
    fSamplerState = GrSamplerState(wrapXAndY, filterMode);
    fVisibility = visibility;
}
