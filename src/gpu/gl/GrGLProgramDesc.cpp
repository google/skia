/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "GrGLProgramDesc.h"

#include "GrProcessor.h"
#include "GrGLGpu.h"
#include "GrPipeline.h"
#include "SkChecksum.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"

/**
 * Do we need to either map r,g,b->a or a->r. configComponentMask indicates which channels are
 * present in the texture's config. swizzleComponentMask indicates the channels present in the
 * shader swizzle.
 */
static bool swizzle_requires_alpha_remapping(const GrGLSLCaps& caps, GrPixelConfig config) {
    if (!caps.mustSwizzleInShader()) {
        // Any remapping is handled using texture swizzling not shader modifications.
        return false;
    }
    const char* swizzleMap = caps.getSwizzleMap(config);
    
    return SkToBool(memcmp(swizzleMap, "rgba", 4));
}

static uint32_t gen_texture_key(const GrProcessor& proc, const GrGLCaps& caps) {
    uint32_t key = 0;
    int numTextures = proc.numTextures();
    for (int t = 0; t < numTextures; ++t) {
        const GrTextureAccess& access = proc.textureAccess(t);
        if (swizzle_requires_alpha_remapping(*caps.glslCaps(), access.getTexture()->config())) {
            key |= 1 << t;
        }
    }
    return key;
}

/**
 * A function which emits a meta key into the key builder.  This is required because shader code may
 * be dependent on properties of the effect that the effect itself doesn't use
 * in its key (e.g. the pixel format of textures used). So we create a meta-key for
 * every effect using this function. It is also responsible for inserting the effect's class ID
 * which must be different for every GrProcessor subclass. It can fail if an effect uses too many
 * textures, transforms, etc, for the space allotted in the meta-key.  NOTE, both FPs and GPs share
 * this function because it is hairy, though FPs do not have attribs, and GPs do not have transforms
 *
 * TODO: A better name for this function  would be "compute" instead of "get".
 */
static bool get_meta_key(const GrProcessor& proc,
                         const GrGLCaps& caps,
                         uint32_t transformKey,
                         GrProcessorKeyBuilder* b) {
    size_t processorKeySize = b->size();
    uint32_t textureKey = gen_texture_key(proc, caps);
    uint32_t classID = proc.classID();

    // Currently we allow 16 bits for each of the above portions of the meta-key. Fail if they
    // don't fit.
    static const uint32_t kMetaKeyInvalidMask = ~((uint32_t) SK_MaxU16);
    if ((textureKey | transformKey | classID) & kMetaKeyInvalidMask) {
        return false;
    }
    if (processorKeySize > SK_MaxU16) {
        return false;
    }

    uint32_t* key = b->add32n(2);
    key[0] = (textureKey << 16 | transformKey);
    key[1] = (classID << 16 | SkToU16(processorKeySize));
    return true;
}

/*
 * TODO: A better name for this function  would be "compute" instead of "get".
 */
static bool get_frag_proc_and_meta_keys(const GrPrimitiveProcessor& primProc,
                                        const GrFragmentProcessor& fp,
                                        const GrGLCaps& caps,
                                        GrProcessorKeyBuilder* b) {
    for (int i = 0; i < fp.numChildProcessors(); ++i) {
        if (!get_frag_proc_and_meta_keys(primProc, fp.childProcessor(i), caps, b)) {
            return false;
        }
    }

    fp.getGLSLProcessorKey(*caps.glslCaps(), b);

    //**** use glslCaps here?
    return get_meta_key(fp, caps, primProc.getTransformKey(fp.coordTransforms(),
                                                           fp.numTransformsExclChildren()), b);
}

bool GrGLProgramDescBuilder::Build(GrProgramDesc* desc,
                                   const GrPrimitiveProcessor& primProc,
                                   const GrPipeline& pipeline,
                                   const GrGLGpu* gpu) {
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

    primProc.getGLSLProcessorKey(*gpu->glCaps().glslCaps(), &b);
    //**** use glslCaps here?
    if (!get_meta_key(primProc, gpu->glCaps(), 0, &b)) {
        glDesc->key().reset();
        return false;
    }

    for (int i = 0; i < pipeline.numFragmentProcessors(); ++i) {
        const GrFragmentProcessor& fp = pipeline.getFragmentProcessor(i);
        if (!get_frag_proc_and_meta_keys(primProc, fp, gpu->glCaps(), &b)) {
            glDesc->key().reset();
            return false;
        }
    }

    const GrXferProcessor& xp = *pipeline.getXferProcessor();
    xp.getGLSLProcessorKey(*gpu->glCaps().glslCaps(), &b);
    //**** use glslCaps here?
    if (!get_meta_key(xp, gpu->glCaps(), 0, &b)) {
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
    header->fSnapVerticesToPixelCenters = pipeline.snapVerticesToPixelCenters();
    header->fColorEffectCnt = pipeline.numColorFragmentProcessors();
    header->fCoverageEffectCnt = pipeline.numCoverageFragmentProcessors();
    glDesc->finalize();
    return true;
}
