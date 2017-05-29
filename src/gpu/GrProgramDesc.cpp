/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "GrProgramDesc.h"
#include "GrPipeline.h"
#include "GrPrimitiveProcessor.h"
#include "GrProcessor.h"
#include "GrRenderTargetPriv.h"
#include "GrShaderCaps.h"
#include "GrTexturePriv.h"
#include "SkChecksum.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"

enum {
    kSamplerOrImageTypeKeyBits = 4
};

static inline uint16_t image_storage_or_sampler_uniform_type_key(GrSLType type ) {
    int value = UINT16_MAX;
    switch (type) {
        case kTexture2DSampler_GrSLType:
            value = 0;
            break;
        case kITexture2DSampler_GrSLType:
            value = 1;
            break;
        case kTextureExternalSampler_GrSLType:
            value = 2;
            break;
        case kTexture2DRectSampler_GrSLType:
            value = 3;
            break;
        case kBufferSampler_GrSLType:
            value = 4;
            break;
        case kImageStorage2D_GrSLType:
            value = 5;
            break;
        case kIImageStorage2D_GrSLType:
            value = 6;
            break;

        default:
            break;
    }
    SkASSERT((value & ((1 << kSamplerOrImageTypeKeyBits) - 1)) == value);
    return value;
}

static uint16_t sampler_key(GrSLType samplerType, GrPixelConfig config, GrShaderFlags visibility,
                            const GrShaderCaps& caps) {
    int samplerTypeKey = image_storage_or_sampler_uniform_type_key(samplerType);

    GR_STATIC_ASSERT(1 == sizeof(caps.configTextureSwizzle(config).asKey()));
    return SkToU16(samplerTypeKey |
                   caps.configTextureSwizzle(config).asKey() << kSamplerOrImageTypeKeyBits |
                   (caps.samplerPrecision(config, visibility) << (8 + kSamplerOrImageTypeKeyBits)));
}

static uint16_t storage_image_key(const GrResourceIOProcessor::ImageStorageAccess& imageAccess) {
    GrSLType type = imageAccess.proxy()->imageStorageType();
    return image_storage_or_sampler_uniform_type_key(type) |
           (int)imageAccess.format() << kSamplerOrImageTypeKeyBits;
}

static void add_sampler_and_image_keys(GrProcessorKeyBuilder* b, const GrResourceIOProcessor& proc,
                                       const GrShaderCaps& caps) {
    int numTextureSamplers = proc.numTextureSamplers();
    int numBuffers = proc.numBuffers();
    int numImageStorages = proc.numImageStorages();
    int numUniforms = numTextureSamplers + numBuffers + numImageStorages;
    // Need two bytes per key.
    int word32Count = (numUniforms + 1) / 2;
    if (0 == word32Count) {
        return;
    }
    uint16_t* k16 = SkTCast<uint16_t*>(b->add32n(word32Count));
    int j = 0;
    for (int i = 0; i < numTextureSamplers; ++i, ++j) {
        const GrResourceIOProcessor::TextureSampler& sampler = proc.textureSampler(i);
        const GrTexture* tex = sampler.peekTexture();

        k16[j] = sampler_key(tex->texturePriv().samplerType(), tex->config(), sampler.visibility(),
                             caps);
    }
    for (int i = 0; i < numBuffers; ++i, ++j) {
        const GrResourceIOProcessor::BufferAccess& access = proc.bufferAccess(i);
        k16[j] = sampler_key(kBufferSampler_GrSLType, access.texelConfig(), access.visibility(),
                             caps);
    }
    for (int i = 0; i < numImageStorages; ++i, ++j) {
        k16[j] = storage_image_key(proc.imageStorageAccess(i));
    }
    // zero the last 16 bits if the number of uniforms for samplers and image storages is odd.
    if (numUniforms & 0x1) {
        k16[numUniforms] = 0;
    }
}

/**
 * A function which emits a meta key into the key builder.  This is required because shader code may
 * be dependent on properties of the effect that the effect itself doesn't use
 * in its key (e.g. the pixel format of textures used). So we create a meta-key for
 * every effect using this function. It is also responsible for inserting the effect's class ID
 * which must be different for every GrProcessor subclass. It can fail if an effect uses too many
 * transforms, etc, for the space allotted in the meta-key.  NOTE, both FPs and GPs share this
 * function because it is hairy, though FPs do not have attribs, and GPs do not have transforms
 */
static bool gen_meta_key(const GrResourceIOProcessor& proc,
                         const GrShaderCaps& shaderCaps,
                         uint32_t transformKey,
                         GrProcessorKeyBuilder* b) {
    size_t processorKeySize = b->size();
    uint32_t classID = proc.classID();

    // Currently we allow 16 bits for the class id and the overall processor key size.
    static const uint32_t kMetaKeyInvalidMask = ~((uint32_t)SK_MaxU16);
    if ((processorKeySize | classID) & kMetaKeyInvalidMask) {
        return false;
    }

    add_sampler_and_image_keys(b, proc, shaderCaps);

    uint32_t* key = b->add32n(2);
    key[0] = (classID << 16) | SkToU32(processorKeySize);
    key[1] = transformKey;
    return true;
}

static bool gen_meta_key(const GrXferProcessor& xp,
                         const GrShaderCaps& shaderCaps,
                         GrProcessorKeyBuilder* b) {
    size_t processorKeySize = b->size();
    uint32_t classID = xp.classID();

    // Currently we allow 16 bits for the class id and the overall processor key size.
    static const uint32_t kMetaKeyInvalidMask = ~((uint32_t)SK_MaxU16);
    if ((processorKeySize | classID) & kMetaKeyInvalidMask) {
        return false;
    }

    b->add32((classID << 16) | SkToU32(processorKeySize));
    return true;
}

static bool gen_frag_proc_and_meta_keys(const GrPrimitiveProcessor& primProc,
                                        const GrFragmentProcessor& fp,
                                        const GrShaderCaps& shaderCaps,
                                        GrProcessorKeyBuilder* b) {
    for (int i = 0; i < fp.numChildProcessors(); ++i) {
        if (!gen_frag_proc_and_meta_keys(primProc, fp.childProcessor(i), shaderCaps, b)) {
            return false;
        }
    }

    fp.getGLSLProcessorKey(shaderCaps, b);

    return gen_meta_key(fp, shaderCaps, primProc.getTransformKey(fp.coordTransforms(),
                                                                 fp.numCoordTransforms()), b);
}

bool GrProgramDesc::Build(GrProgramDesc* desc,
                          const GrPrimitiveProcessor& primProc,
                          bool hasPointSize,
                          const GrPipeline& pipeline,
                          const GrShaderCaps& shaderCaps) {
    // The descriptor is used as a cache key. Thus when a field of the
    // descriptor will not affect program generation (because of the attribute
    // bindings in use or other descriptor field settings) it should be set
    // to a canonical value to avoid duplicate programs with different keys.

    GR_STATIC_ASSERT(0 == kProcessorKeysOffset % sizeof(uint32_t));
    // Make room for everything up to the effect keys.
    desc->key().reset();
    desc->key().push_back_n(kProcessorKeysOffset);

    GrProcessorKeyBuilder b(&desc->key());

    primProc.getGLSLProcessorKey(shaderCaps, &b);
    if (!gen_meta_key(primProc, shaderCaps, 0, &b)) {
        desc->key().reset();
        return false;
    }
    GrProcessor::RequiredFeatures requiredFeatures = primProc.requiredFeatures();

    for (int i = 0; i < pipeline.numFragmentProcessors(); ++i) {
        const GrFragmentProcessor& fp = pipeline.getFragmentProcessor(i);
        if (!gen_frag_proc_and_meta_keys(primProc, fp, shaderCaps, &b)) {
            desc->key().reset();
            return false;
        }
        requiredFeatures |= fp.requiredFeatures();
    }

    const GrXferProcessor& xp = pipeline.getXferProcessor();
    const GrSurfaceOrigin* originIfDstTexture = nullptr;
    GrSurfaceOrigin origin;
    if (pipeline.dstTextureProxy()) {
        origin = pipeline.dstTextureProxy()->origin();
        originIfDstTexture = &origin;
    }
    xp.getGLSLProcessorKey(shaderCaps, &b, originIfDstTexture);
    if (!gen_meta_key(xp, shaderCaps, &b)) {
        desc->key().reset();
        return false;
    }
    requiredFeatures |= xp.requiredFeatures();

    // --------DO NOT MOVE HEADER ABOVE THIS LINE--------------------------------------------------
    // Because header is a pointer into the dynamic array, we can't push any new data into the key
    // below here.
    KeyHeader* header = desc->atOffset<KeyHeader, kHeaderOffset>();

    // make sure any padding in the header is zeroed.
    memset(header, 0, kHeaderSize);

    GrRenderTarget* rt = pipeline.getRenderTarget();

    if (requiredFeatures & GrProcessor::kSampleLocations_RequiredFeature) {
        SkASSERT(pipeline.isHWAntialiasState());
        header->fSamplePatternKey =
            rt->renderTargetPriv().getMultisampleSpecs(pipeline).fUniqueID;
    } else {
        header->fSamplePatternKey = 0;
    }

    header->fOutputSwizzle = shaderCaps.configOutputSwizzle(rt->config()).asKey();

    header->fSnapVerticesToPixelCenters = pipeline.snapVerticesToPixelCenters();
    header->fColorFragmentProcessorCnt = pipeline.numColorFragmentProcessors();
    header->fCoverageFragmentProcessorCnt = pipeline.numCoverageFragmentProcessors();
    // Fail if the client requested more processors than the key can fit.
    if (header->fColorFragmentProcessorCnt != pipeline.numColorFragmentProcessors() ||
        header->fCoverageFragmentProcessorCnt != pipeline.numCoverageFragmentProcessors()) {
        return false;
    }
    header->fHasPointSize = hasPointSize ? 1 : 0;
    return true;
}
