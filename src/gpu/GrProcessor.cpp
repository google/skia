/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrProcessor.h"
#include "GrContext.h"
#include "GrGeometryProcessor.h"
#include "GrMemoryPool.h"
#include "GrSamplerParams.h"
#include "GrTexturePriv.h"
#include "GrTextureProxy.h"
#include "GrXferProcessor.h"
#include "SkSpinlock.h"

#if GR_TEST_UTILS

GrResourceProvider* GrProcessorTestData::resourceProvider() {
    return fContext->resourceProvider();
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
template<>
SkTArray<GrProcessorTestFactory<GrFragmentProcessor>*, true>*
GrProcessorTestFactory<GrFragmentProcessor>::GetFactories() {
    static SkTArray<GrProcessorTestFactory<GrFragmentProcessor>*, true> gFactories;
    return &gFactories;
}

template<>
SkTArray<GrProcessorTestFactory<GrGeometryProcessor>*, true>*
GrProcessorTestFactory<GrGeometryProcessor>::GetFactories() {
    static SkTArray<GrProcessorTestFactory<GrGeometryProcessor>*, true> gFactories;
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
static const int kFPFactoryCount = 41;
static const int kGPFactoryCount = 14;
static const int kXPFactoryCount = 4;

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

void GrXPFactoryTestFactory::VerifyFactoryCount() {
    if (kXPFactoryCount != GetFactories()->count()) {
        SkDebugf("\nExpected %d xp factory factories, found %d.\n",
                 kXPFactoryCount, GetFactories()->count());
        SkFAIL("Wrong number of xp factory factories!");
    }
}

#endif
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

void GrResourceIOProcessor::addImageStorageAccess(const ImageStorageAccess* access) {
    fImageStorageAccesses.push_back(access);
}

void GrResourceIOProcessor::addPendingIOs() const {
    for (const auto& sampler : fTextureSamplers) {
        sampler->programTexture()->markPendingIO();
    }
    for (const auto& buffer : fBufferAccesses) {
        buffer->programBuffer()->markPendingIO();
    }
    for (const auto& imageStorage : fImageStorageAccesses) {
        imageStorage->programTexture()->markPendingIO();
    }
}

void GrResourceIOProcessor::removeRefs() const {
    for (const auto& sampler : fTextureSamplers) {
        sampler->programTexture()->removeRef();
    }
    for (const auto& buffer : fBufferAccesses) {
        buffer->programBuffer()->removeRef();
    }
    for (const auto& imageStorage : fImageStorageAccesses) {
        imageStorage->programTexture()->removeRef();
    }
}

void GrResourceIOProcessor::pendingIOComplete() const {
    for (const auto& sampler : fTextureSamplers) {
        sampler->programTexture()->pendingIOComplete();
    }
    for (const auto& buffer : fBufferAccesses) {
        buffer->programBuffer()->pendingIOComplete();
    }
    for (const auto& imageStorage : fImageStorageAccesses) {
        imageStorage->programTexture()->pendingIOComplete();
    }
}

bool GrResourceIOProcessor::hasSameSamplersAndAccesses(const GrResourceIOProcessor& that) const {
    if (this->numTextureSamplers() != that.numTextureSamplers() ||
        this->numBuffers() != that.numBuffers() ||
        this->numImageStorages() != that.numImageStorages()) {
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
    for (int i = 0; i < this->numImageStorages(); ++i) {
        if (this->imageStorageAccess(i) != that.imageStorageAccess(i)) {
            return false;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

GrResourceIOProcessor::TextureSampler::TextureSampler() {}

GrResourceIOProcessor::TextureSampler::TextureSampler(GrTexture* texture,
                                                      const GrSamplerParams& params) {
    this->reset(texture, params);
}

GrResourceIOProcessor::TextureSampler::TextureSampler(GrTexture* texture,
                                                      GrSamplerParams::FilterMode filterMode,
                                                      SkShader::TileMode tileXAndY,
                                                      GrShaderFlags visibility) {
    this->reset(texture, filterMode, tileXAndY, visibility);
}

GrResourceIOProcessor::TextureSampler::TextureSampler(GrResourceProvider* resourceProvider,
                                                      sk_sp<GrTextureProxy> proxy,
                                                      const GrSamplerParams& params) {
    this->reset(resourceProvider, std::move(proxy), params);
}

GrResourceIOProcessor::TextureSampler::TextureSampler(GrResourceProvider* resourceProvider,
                                                      sk_sp<GrTextureProxy> proxy,
                                                      GrSamplerParams::FilterMode filterMode,
                                                      SkShader::TileMode tileXAndY,
                                                      GrShaderFlags visibility) {
    this->reset(resourceProvider, std::move(proxy), filterMode, tileXAndY, visibility);
}

void GrResourceIOProcessor::TextureSampler::reset(GrTexture* texture,
                                                  const GrSamplerParams& params,
                                                  GrShaderFlags visibility) {
    SkASSERT(texture);
    fTexture.set(SkRef(texture), kRead_GrIOType);
    fParams = params;
    fParams.setFilterMode(SkTMin(params.filterMode(), texture->texturePriv().highestFilterMode()));
    fVisibility = visibility;
}

void GrResourceIOProcessor::TextureSampler::reset(GrTexture* texture,
                                                  GrSamplerParams::FilterMode filterMode,
                                                  SkShader::TileMode tileXAndY,
                                                  GrShaderFlags visibility) {
    SkASSERT(texture);
    fTexture.set(SkRef(texture), kRead_GrIOType);
    filterMode = SkTMin(filterMode, texture->texturePriv().highestFilterMode());
    fParams.reset(tileXAndY, filterMode);
    fVisibility = visibility;
}

void GrResourceIOProcessor::TextureSampler::reset(GrResourceProvider* resourceProvider,
                                                  sk_sp<GrTextureProxy> proxy,
                                                  const GrSamplerParams& params,
                                                  GrShaderFlags visibility) {
    // For now, end the deferral at this time. Once all the TextureSamplers are swapped over
    // to taking a GrSurfaceProxy just use the IORefs on the proxy
    GrTexture* texture = proxy->instantiate(resourceProvider);
    SkASSERT(texture);
    fTexture.set(SkRef(texture), kRead_GrIOType);
    fParams = params;
    fParams.setFilterMode(SkTMin(params.filterMode(), texture->texturePriv().highestFilterMode()));
    fVisibility = visibility;
}

void GrResourceIOProcessor::TextureSampler::reset(GrResourceProvider* resourceProvider,
                                                  sk_sp<GrTextureProxy> proxy,
                                                  GrSamplerParams::FilterMode filterMode,
                                                  SkShader::TileMode tileXAndY,
                                                  GrShaderFlags visibility) {
    // For now, end the deferral at this time. Once all the TextureSamplers are swapped over
    // to taking a GrSurfaceProxy just use the IORefs on the proxy
    GrTexture* texture = proxy->instantiate(resourceProvider);
    SkASSERT(texture);
    fTexture.set(SkRef(texture), kRead_GrIOType);
    filterMode = SkTMin(filterMode, texture->texturePriv().highestFilterMode());
    fParams.reset(tileXAndY, filterMode);
    fVisibility = visibility;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

GrResourceIOProcessor::ImageStorageAccess::ImageStorageAccess(sk_sp<GrTexture> texture,
                                                              GrIOType ioType,
                                                              GrSLMemoryModel memoryModel,
                                                              GrSLRestrict restrict,
                                                              GrShaderFlags visibility) {
    SkASSERT(texture);
    fTexture.set(texture.release(), ioType);
    fMemoryModel = memoryModel;
    fRestrict = restrict;
    fVisibility = visibility;
    // We currently infer this from the config. However, we could allow the client to specify
    // a format that is different but compatible with the config.
    switch (fTexture.get()->config()) {
        case kRGBA_8888_GrPixelConfig:
            fFormat = GrImageStorageFormat::kRGBA8;
            break;
        case kRGBA_8888_sint_GrPixelConfig:
            fFormat = GrImageStorageFormat::kRGBA8i;
            break;
        case kRGBA_half_GrPixelConfig:
            fFormat = GrImageStorageFormat::kRGBA16f;
            break;
        case kRGBA_float_GrPixelConfig:
            fFormat = GrImageStorageFormat::kRGBA32f;
            break;
        default:
            SkFAIL("Config is not (yet) supported as image storage.");
            break;
    }
}
