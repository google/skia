/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrProgramDesc.h"

#include "include/private/SkChecksum.h"
#include "include/private/SkTo.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrPrimitiveProcessor.h"
#include "src/gpu/GrProcessor.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"

enum {
    kSamplerOrImageTypeKeyBits = 4
};

static inline uint16_t texture_type_key(GrTextureType type) {
    int value = UINT16_MAX;
    switch (type) {
        case GrTextureType::k2D:
            value = 0;
            break;
        case GrTextureType::kExternal:
            value = 1;
            break;
        case GrTextureType::kRectangle:
            value = 2;
            break;
        default:
            SK_ABORT("Unexpected texture type");
            value = 3;
            break;
    }
    SkASSERT((value & ((1 << kSamplerOrImageTypeKeyBits) - 1)) == value);
    return SkToU16(value);
}

static uint32_t sampler_key(GrTextureType textureType, const GrSwizzle& swizzle,
                            const GrShaderCaps& caps) {
    int samplerTypeKey = texture_type_key(textureType);

    GR_STATIC_ASSERT(2 == sizeof(swizzle.asKey()));
    uint16_t swizzleKey = 0;
    if (caps.textureSwizzleAppliedInShader()) {
        swizzleKey = swizzle.asKey();
    }
    return SkToU32(samplerTypeKey | swizzleKey << kSamplerOrImageTypeKeyBits);
}

static void add_sampler_keys(GrProcessorKeyBuilder* b, const GrFragmentProcessor& fp,
                             GrGpu* gpu, const GrShaderCaps& caps) {
    int numTextureSamplers = fp.numTextureSamplers();
    if (!numTextureSamplers) {
        return;
    }
    for (int i = 0; i < numTextureSamplers; ++i) {
        const GrFragmentProcessor::TextureSampler& sampler = fp.textureSampler(i);
        const GrTexture* tex = sampler.peekTexture();
        uint32_t samplerKey = sampler_key(
                tex->texturePriv().textureType(), sampler.swizzle(), caps);
        uint32_t extraSamplerKey = gpu->getExtraSamplerKeyForProgram(
                sampler.samplerState(), sampler.proxy()->backendFormat());
        if (extraSamplerKey) {
            // We first mark the normal sampler key with last bit to flag that it has an extra
            // sampler key. We then add both keys.
            SkASSERT((samplerKey & (1 << 31)) == 0);
            b->add32(samplerKey | (1 << 31));
            b->add32(extraSamplerKey);
        } else {
            b->add32(samplerKey);
        }
    }
}

static void add_sampler_keys(GrProcessorKeyBuilder* b, const GrPrimitiveProcessor& pp,
                             const GrShaderCaps& caps) {
    int numTextureSamplers = pp.numTextureSamplers();
    if (!numTextureSamplers) {
        return;
    }
    for (int i = 0; i < numTextureSamplers; ++i) {
        const GrPrimitiveProcessor::TextureSampler& sampler = pp.textureSampler(i);
        uint32_t samplerKey = sampler_key(
                sampler.textureType(), sampler.swizzle(), caps);
        uint32_t extraSamplerKey = sampler.extraSamplerKey();
        if (extraSamplerKey) {
            // We first mark the normal sampler key with last bit to flag that it has an extra
            // sampler key. We then add both keys.
            SkASSERT((samplerKey & (1 << 31)) == 0);
            b->add32(samplerKey | (1 << 31));
            b->add32(extraSamplerKey);
        } else {
            b->add32(samplerKey);
        }
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
static bool gen_meta_key(const GrFragmentProcessor& fp,
                         GrGpu* gpu,
                         const GrShaderCaps& shaderCaps,
                         uint32_t transformKey,
                         GrProcessorKeyBuilder* b) {
    size_t processorKeySize = b->size();
    uint32_t classID = fp.classID();

    // Currently we allow 16 bits for the class id and the overall processor key size.
    static const uint32_t kMetaKeyInvalidMask = ~((uint32_t)UINT16_MAX);
    if ((processorKeySize | classID) & kMetaKeyInvalidMask) {
        return false;
    }

    add_sampler_keys(b, fp, gpu, shaderCaps);

    uint32_t* key = b->add32n(2);
    key[0] = (classID << 16) | SkToU32(processorKeySize);
    key[1] = transformKey;
    return true;
}

static bool gen_meta_key(const GrPrimitiveProcessor& pp,
                         const GrShaderCaps& shaderCaps,
                         uint32_t transformKey,
                         GrProcessorKeyBuilder* b) {
    size_t processorKeySize = b->size();
    uint32_t classID = pp.classID();

    // Currently we allow 16 bits for the class id and the overall processor key size.
    static const uint32_t kMetaKeyInvalidMask = ~((uint32_t)UINT16_MAX);
    if ((processorKeySize | classID) & kMetaKeyInvalidMask) {
        return false;
    }

    add_sampler_keys(b, pp, shaderCaps);

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
    static const uint32_t kMetaKeyInvalidMask = ~((uint32_t)UINT16_MAX);
    if ((processorKeySize | classID) & kMetaKeyInvalidMask) {
        return false;
    }

    b->add32((classID << 16) | SkToU32(processorKeySize));
    return true;
}

static bool gen_frag_proc_and_meta_keys(const GrPrimitiveProcessor& primProc,
                                        const GrFragmentProcessor& fp,
                                        GrGpu* gpu,
                                        const GrShaderCaps& shaderCaps,
                                        GrProcessorKeyBuilder* b) {
    for (int i = 0; i < fp.numChildProcessors(); ++i) {
        if (!gen_frag_proc_and_meta_keys(primProc, fp.childProcessor(i), gpu, shaderCaps, b)) {
            return false;
        }
    }

    fp.getGLSLProcessorKey(shaderCaps, b);

    return gen_meta_key(fp, gpu, shaderCaps, primProc.getTransformKey(fp.coordTransforms(),
                                                                      fp.numCoordTransforms()), b);
}

bool GrProgramDesc::Build(GrProgramDesc* desc, const GrRenderTarget* renderTarget,
                          const GrProgramInfo& programInfo, bool hasPointSize, GrGpu* gpu) {
    // The descriptor is used as a cache key. Thus when a field of the
    // descriptor will not affect program generation (because of the attribute
    // bindings in use or other descriptor field settings) it should be set
    // to a canonical value to avoid duplicate programs with different keys.

    const GrShaderCaps& shaderCaps = *gpu->caps()->shaderCaps();

    GR_STATIC_ASSERT(0 == kProcessorKeysOffset % sizeof(uint32_t));
    // Make room for everything up to the effect keys.
    desc->key().reset();
    desc->key().push_back_n(kProcessorKeysOffset);

    GrProcessorKeyBuilder b(&desc->key());

    programInfo.primProc().getGLSLProcessorKey(shaderCaps, &b);
    programInfo.primProc().getAttributeKey(&b);
    if (!gen_meta_key(programInfo.primProc(), shaderCaps, 0, &b)) {
        desc->key().reset();
        return false;
    }

    // TODO: use programInfo.requestedFeatures here
    GrProcessor::CustomFeatures processorFeatures = programInfo.primProc().requestedFeatures();

    for (int i = 0; i < programInfo.pipeline().numFragmentProcessors(); ++i) {
        const GrFragmentProcessor& fp = programInfo.pipeline().getFragmentProcessor(i);
        if (!gen_frag_proc_and_meta_keys(programInfo.primProc(), fp, gpu, shaderCaps, &b)) {
            desc->key().reset();
            return false;
        }
        processorFeatures |= fp.requestedFeatures();
    }

    const GrXferProcessor& xp = programInfo.pipeline().getXferProcessor();
    const GrSurfaceOrigin* originIfDstTexture = nullptr;
    GrSurfaceOrigin origin;
    if (programInfo.pipeline().dstTextureProxy()) {
        origin = programInfo.pipeline().dstTextureProxy()->origin();
        originIfDstTexture = &origin;
    }
    xp.getGLSLProcessorKey(shaderCaps, &b, originIfDstTexture);
    if (!gen_meta_key(xp, shaderCaps, &b)) {
        desc->key().reset();
        return false;
    }
    processorFeatures |= xp.requestedFeatures();

    if (processorFeatures & GrProcessor::CustomFeatures::kSampleLocations) {
        SkASSERT(programInfo.pipeline().isHWAntialiasState());
        b.add32(renderTarget->renderTargetPriv().getSamplePatternKey());
    }

    // --------DO NOT MOVE HEADER ABOVE THIS LINE--------------------------------------------------
    // Because header is a pointer into the dynamic array, we can't push any new data into the key
    // below here.
    KeyHeader* header = desc->atOffset<KeyHeader, kHeaderOffset>();

    // make sure any padding in the header is zeroed.
    memset(header, 0, kHeaderSize);
    header->fOutputSwizzle = programInfo.pipeline().outputSwizzle().asKey();
    header->fColorFragmentProcessorCnt = programInfo.pipeline().numColorFragmentProcessors();
    header->fCoverageFragmentProcessorCnt = programInfo.pipeline().numCoverageFragmentProcessors();
    // Fail if the client requested more processors than the key can fit.
    if (header->fColorFragmentProcessorCnt != programInfo.pipeline().numColorFragmentProcessors() ||
        header->fCoverageFragmentProcessorCnt !=
                                         programInfo.pipeline().numCoverageFragmentProcessors()) {
        return false;
    }
    // If we knew the shader won't depend on origin, we could skip this (and use the same program
    // for both origins). Instrumenting all fragment processors would be difficult and error prone.
    header->fSurfaceOriginKey =
        GrGLSLFragmentShaderBuilder::KeyForSurfaceOrigin(programInfo.origin());
    header->fProcessorFeatures = (uint8_t)processorFeatures;
    SkASSERT(header->processorFeatures() == processorFeatures);  // Ensure enough bits.
    header->fSnapVerticesToPixelCenters = programInfo.pipeline().snapVerticesToPixelCenters();
    header->fHasPointSize = hasPointSize ? 1 : 0;
    return true;
}
