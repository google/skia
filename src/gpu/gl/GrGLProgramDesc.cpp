/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "GrGLProgramDesc.h"

#include "GrProcessor.h"
#include "GrPipeline.h"
#include "SkChecksum.h"
#include "gl/GrGLDefines.h"
#include "gl/GrGLTexture.h"
#include "gl/GrGLTypes.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLCaps.h"

static uint16_t texture_target_key(GrGLenum target) {
    SkASSERT((uint32_t)target < SK_MaxU16);
    return target;
}

static void add_texture_key(GrProcessorKeyBuilder* b, const GrProcessor& proc,
                            const GrGLSLCaps& caps) {
    int numTextures = proc.numTextures();
    // Need two bytes per key (swizzle and target).
    int word32Count = (proc.numTextures() + 1) / 2;
    if (0 == word32Count) {
        return;
    }
    uint16_t* k16 = SkTCast<uint16_t*>(b->add32n(word32Count));
    for (int i = 0; i < numTextures; ++i) {
        const GrTextureAccess& access = proc.textureAccess(i);
        GrGLTexture* texture = static_cast<GrGLTexture*>(access.getTexture());
        k16[i] = caps.configTextureSwizzle(texture->config()).asKey() |
                 (texture_target_key(texture->target()) << 16);
    }
    // zero the last 16 bits if the number of textures is odd.
    if (numTextures & 0x1) {
        k16[numTextures] = 0;
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
static bool gen_meta_key(const GrProcessor& proc,
                         const GrGLSLCaps& glslCaps,
                         uint32_t transformKey,
                         GrProcessorKeyBuilder* b) {
    size_t processorKeySize = b->size();
    uint32_t classID = proc.classID();

    // Currently we allow 16 bits for the class id and the overall processor key size.
    static const uint32_t kMetaKeyInvalidMask = ~((uint32_t) SK_MaxU16);
    if ((processorKeySize | classID) & kMetaKeyInvalidMask) {
        return false;
    }

    add_texture_key(b, proc, glslCaps);

    uint32_t* key = b->add32n(2);
    key[0] = (classID << 16) | SkToU32(processorKeySize);
    key[1] = transformKey;
    return true;
}

static bool gen_frag_proc_and_meta_keys(const GrPrimitiveProcessor& primProc,
                                        const GrFragmentProcessor& fp,
                                        const GrGLSLCaps& glslCaps,
                                        GrProcessorKeyBuilder* b) {
    for (int i = 0; i < fp.numChildProcessors(); ++i) {
        if (!gen_frag_proc_and_meta_keys(primProc, fp.childProcessor(i), glslCaps, b)) {
            return false;
        }
    }

    fp.getGLSLProcessorKey(glslCaps, b);

    return gen_meta_key(fp, glslCaps, primProc.getTransformKey(fp.coordTransforms(),
                                                               fp.numTransformsExclChildren()), b);
}

bool GrGLProgramDescBuilder::Build(GrProgramDesc* desc,
                                   const GrPrimitiveProcessor& primProc,
                                   const GrPipeline& pipeline,
                                   const GrGLSLCaps& glslCaps) {
    // The descriptor is used as a cache key. Thus when a field of the
    // descriptor will not affect program generation (because of the attribute
    // bindings in use or other descriptor field settings) it should be set
    // to a canonical value to avoid duplicate programs with different keys.

    GrGLProgramDesc* glDesc = (GrGLProgramDesc*) desc;

    GR_STATIC_ASSERT(0 == kProcessorKeysOffset % sizeof(uint32_t));
    // Make room for everything up to the effect keys.
    glDesc->key().reset();
    glDesc->key().push_back_n(kProcessorKeysOffset);

    GrProcessorKeyBuilder b(&glDesc->key());

    primProc.getGLSLProcessorKey(glslCaps, &b);
    if (!gen_meta_key(primProc, glslCaps, 0, &b)) {
        glDesc->key().reset();
        return false;
    }

    for (int i = 0; i < pipeline.numFragmentProcessors(); ++i) {
        const GrFragmentProcessor& fp = pipeline.getFragmentProcessor(i);
        if (!gen_frag_proc_and_meta_keys(primProc, fp, glslCaps, &b)) {
            glDesc->key().reset();
            return false;
        }
    }

    const GrXferProcessor& xp = pipeline.getXferProcessor();
    xp.getGLSLProcessorKey(glslCaps, &b);
    if (!gen_meta_key(xp, glslCaps, 0, &b)) {
        glDesc->key().reset();
        return false;
    }

    // --------DO NOT MOVE HEADER ABOVE THIS LINE--------------------------------------------------
    // Because header is a pointer into the dynamic array, we can't push any new data into the key
    // below here.
    KeyHeader* header = glDesc->atOffset<KeyHeader, kHeaderOffset>();

    // make sure any padding in the header is zeroed.
    memset(header, 0, kHeaderSize);

    if (pipeline.readsFragPosition()) {
        header->fFragPosKey =
                GrGLSLFragmentShaderBuilder::KeyForFragmentPosition(pipeline.getRenderTarget());
    } else {
        header->fFragPosKey = 0;
    }

    header->fOutputSwizzle =
        glslCaps.configOutputSwizzle(pipeline.getRenderTarget()->config()).asKey();

    if (pipeline.ignoresCoverage()) {
        header->fIgnoresCoverage = 1;
    } else {
        header->fIgnoresCoverage = 0;
    }

    header->fSnapVerticesToPixelCenters = pipeline.snapVerticesToPixelCenters();
    header->fColorEffectCnt = pipeline.numColorFragmentProcessors();
    header->fCoverageEffectCnt = pipeline.numCoverageFragmentProcessors();
    glDesc->finalize();
    return true;
}
