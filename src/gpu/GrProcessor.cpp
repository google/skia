/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrProcessor.h"

#include <cstdlib>
#include "GrContext.h"
#include "GrGeometryProcessor.h"
#include "GrInvariantOutput.h"
#include "GrSamplerParams.h"
#include "GrTexturePriv.h"
#include "GrXferProcessor.h"

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

int32_t GrProcessor::gCurrProcessorClassID = GrProcessor::kIllegalProcessorClassID;

///////////////////////////////////////////////////////////////////////////////

GrProcessor::~GrProcessor() {}

void GrProcessor::addTextureSampler(const TextureSampler* access) {
    fTextureSamplers.push_back(access);
    this->addGpuResource(access->programTexture());
}

void GrProcessor::addBufferAccess(const BufferAccess* access) {
    fBufferAccesses.push_back(access);
    this->addGpuResource(access->programBuffer());
}

void GrProcessor::addImageStorageAccess(const ImageStorageAccess* access) {
    fImageStorageAccesses.push_back(access);
    this->addGpuResource(access->programTexture());
}

void* GrProcessor::operator new(size_t size) {
    return malloc(size);
}

void GrProcessor::operator delete(void* target) {
    free(target);
}

bool GrProcessor::hasSameSamplersAndAccesses(const GrProcessor &that) const {
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

GrProcessor::TextureSampler::TextureSampler() {}

GrProcessor::TextureSampler::TextureSampler(GrTexture* texture, const GrSamplerParams& params) {
    this->reset(texture, params);
}

GrProcessor::TextureSampler::TextureSampler(GrTexture* texture,
                                            GrSamplerParams::FilterMode filterMode,
                                            SkShader::TileMode tileXAndY,
                                            GrShaderFlags visibility) {
    this->reset(texture, filterMode, tileXAndY, visibility);
}

void GrProcessor::TextureSampler::reset(GrTexture* texture,
                                        const GrSamplerParams& params,
                                        GrShaderFlags visibility) {
    SkASSERT(texture);
    fTexture.set(SkRef(texture), kRead_GrIOType);
    fParams = params;
    fParams.setFilterMode(SkTMin(params.filterMode(), texture->texturePriv().highestFilterMode()));
    fVisibility = visibility;
}

void GrProcessor::TextureSampler::reset(GrTexture* texture,
                                        GrSamplerParams::FilterMode filterMode,
                                        SkShader::TileMode tileXAndY,
                                        GrShaderFlags visibility) {
    SkASSERT(texture);
    fTexture.set(SkRef(texture), kRead_GrIOType);
    filterMode = SkTMin(filterMode, texture->texturePriv().highestFilterMode());
    fParams.reset(tileXAndY, filterMode);
    fVisibility = visibility;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

GrProcessor::ImageStorageAccess::ImageStorageAccess(sk_sp<GrTexture> texture, GrIOType ioType,
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

///////////////////////////////////////////////////////////////////////////////////////////////////

// Initial static variable from GrXPFactory
int32_t GrXPFactory::gCurrXPFClassID = GrXPFactory::kIllegalXPFClassID;
