/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrProcessor.h"
#include "GrContext.h"
#include "GrGeometryProcessor.h"
#include "GrInvariantOutput.h"
#include "GrMemoryPool.h"
#include "GrTextureParams.h"
#include "GrTexturePriv.h"
#include "GrXferProcessor.h"
#include "SkSpinlock.h"

#if SK_ALLOW_STATIC_GLOBAL_INITIALIZERS

class GrFragmentProcessor;
class GrGeometryProcessor;

/*
 * Originally these were both in the processor unit test header, but then it seemed to cause linker
 * problems on android.
 */
template<>
SkTArray<GrProcessorTestFactory<GrFragmentProcessor>*, true>*
GrProcessorTestFactory<GrFragmentProcessor>::GetFactories() {
    static SkTArray<GrProcessorTestFactory<GrFragmentProcessor>*, true> gFactories;
    return &gFactories;
}

template<>
SkTArray<GrProcessorTestFactory<GrXPFactory>*, true>*
GrProcessorTestFactory<GrXPFactory>::GetFactories() {
    static SkTArray<GrProcessorTestFactory<GrXPFactory>*, true> gFactories;
    return &gFactories;
}

template<>
SkTArray<GrProcessorTestFactory<GrGeometryProcessor>*, true>*
GrProcessorTestFactory<GrGeometryProcessor>::GetFactories() {
    static SkTArray<GrProcessorTestFactory<GrGeometryProcessor>*, true> gFactories;
    return &gFactories;
}

/*
 * To ensure we always have successful static initialization, before creating from the factories
 * we verify the count is as expected.  If a new factory is added, then these numbers must be
 * manually adjusted.
 */
static const int kFPFactoryCount = 40;
static const int kGPFactoryCount = 14;
static const int kXPFactoryCount = 5;

template<>
void GrProcessorTestFactory<GrFragmentProcessor>::VerifyFactoryCount() {
    if (kFPFactoryCount != GetFactories()->count()) {
        SkDebugf("\nExpected %d fragment processor factories, found %d.\n",
                 kFPFactoryCount, GetFactories()->count());
        SkFAIL("Wrong number of fragment processor factories!");
    }
}

template<>
void GrProcessorTestFactory<GrGeometryProcessor>::VerifyFactoryCount() {
    if (kGPFactoryCount != GetFactories()->count()) {
        SkDebugf("\nExpected %d geometry processor factories, found %d.\n",
                 kGPFactoryCount, GetFactories()->count());
        SkFAIL("Wrong number of geometry processor factories!");
    }
}

template<>
void GrProcessorTestFactory<GrXPFactory>::VerifyFactoryCount() {
    if (kXPFactoryCount != GetFactories()->count()) {
        SkDebugf("\nExpected %d xp factory factories, found %d.\n",
                 kXPFactoryCount, GetFactories()->count());
        SkFAIL("Wrong number of xp factory factories!");
    }
}

#endif


// We use a global pool protected by a mutex(spinlock). Chrome may use the same GrContext on
// different threads. The GrContext is not used concurrently on different threads and there is a
// memory barrier between accesses of a context on different threads. Also, there may be multiple
// GrContexts and those contexts may be in use concurrently on different threads.
namespace {
static SkSpinlock gProcessorSpinlock;
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

int32_t GrProcessor::gCurrProcessorClassID = GrProcessor::kIllegalProcessorClassID;

///////////////////////////////////////////////////////////////////////////////

GrProcessor::~GrProcessor() {}

void GrProcessor::addTextureSampler(const TextureSampler* access) {
    fTextureSamplers.push_back(access);
    this->addGpuResource(access->programTexture());
}

void GrProcessor::addBufferAccess(const GrBufferAccess* access) {
    fBufferAccesses.push_back(access);
    this->addGpuResource(access->getProgramBuffer());
}

void* GrProcessor::operator new(size_t size) {
    return MemoryPoolAccessor().pool()->allocate(size);
}

void GrProcessor::operator delete(void* target) {
    return MemoryPoolAccessor().pool()->release(target);
}

bool GrProcessor::hasSameSamplers(const GrProcessor& that) const {
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

GrProcessor::TextureSampler::TextureSampler() {}

GrProcessor::TextureSampler::TextureSampler(GrTexture* texture, const GrTextureParams& params) {
    this->reset(texture, params);
}

GrProcessor::TextureSampler::TextureSampler(GrTexture* texture,
                                            GrTextureParams::FilterMode filterMode,
                                            SkShader::TileMode tileXAndY,
                                            GrShaderFlags visibility) {
    this->reset(texture, filterMode, tileXAndY, visibility);
}

void GrProcessor::TextureSampler::reset(GrTexture* texture,
                                        const GrTextureParams& params,
                                        GrShaderFlags visibility) {
    SkASSERT(texture);
    fTexture.set(SkRef(texture), kRead_GrIOType);
    fParams = params;
    fParams.setFilterMode(SkTMin(params.filterMode(), texture->texturePriv().highestFilterMode()));
    fVisibility = visibility;
}

void GrProcessor::TextureSampler::reset(GrTexture* texture,
                                        GrTextureParams::FilterMode filterMode,
                                        SkShader::TileMode tileXAndY,
                                        GrShaderFlags visibility) {
    SkASSERT(texture);
    fTexture.set(SkRef(texture), kRead_GrIOType);
    filterMode = SkTMin(filterMode, texture->texturePriv().highestFilterMode());
    fParams.reset(tileXAndY, filterMode);
    fVisibility = visibility;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// Initial static variable from GrXPFactory
int32_t GrXPFactory::gCurrXPFClassID =
        GrXPFactory::kIllegalXPFClassID;
