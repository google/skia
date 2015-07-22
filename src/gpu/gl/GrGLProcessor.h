/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLProcessor_DEFINED
#define GrGLProcessor_DEFINED

#include "GrGLProgramDataManager.h"
#include "GrProcessor.h"
#include "GrTextureAccess.h"

/** @file
    This file contains specializations for OpenGL of the shader stages declared in
    include/gpu/GrProcessor.h. Objects of type GrGLProcessor are responsible for emitting the
    GLSL code that implements a GrProcessor and for uploading uniforms at draw time. If they don't
    always emit the same GLSL code, they must have a function:
        static inline void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*)
    that is used to implement a program cache. When two GrProcessors produce the same key this means
    that their GrGLProcessors would emit the same GLSL code.

    The GrGLProcessor subclass must also have a constructor of the form:
        ProcessorSubclass::ProcessorSubclass(const GrBackendProcessorFactory&, const GrProcessor&)

    These objects are created by the factory object returned by the GrProcessor::getFactory().
*/
// TODO delete this and make TextureSampler its own thing
class GrGLProcessor {
public:
    typedef GrGLProgramDataManager::UniformHandle UniformHandle;

    /**
     * Passed to GrGLProcessors so they can add transformed coordinates to their shader code.
     */
    typedef GrShaderVar TransformedCoords;
    typedef SkTArray<GrShaderVar> TransformedCoordsArray;

    /**
     * Passed to GrGLProcessors so they can add texture reads to their shader code.
     */
    class TextureSampler {
    public:
        TextureSampler(UniformHandle uniform, const GrTextureAccess& access)
            : fSamplerUniform(uniform)
            , fConfigComponentMask(GrPixelConfigComponentMask(access.getTexture()->config())) {
            SkASSERT(0 != fConfigComponentMask);
            memcpy(fSwizzle, access.getSwizzle(), 5);
        }

        // bitfield of GrColorComponentFlags present in the texture's config.
        uint32_t configComponentMask() const { return fConfigComponentMask; }
        // this is .abcd
        const char* swizzle() const { return fSwizzle; }

    private:
        UniformHandle fSamplerUniform;
        uint32_t      fConfigComponentMask;
        char          fSwizzle[5];

        friend class GrGLShaderBuilder;
    };

    typedef SkTArray<TextureSampler> TextureSamplerArray;
};

#endif
