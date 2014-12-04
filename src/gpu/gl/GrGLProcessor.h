/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLProcessor_DEFINED
#define GrGLProcessor_DEFINED

#include "GrBackendProcessorFactory.h"
#include "GrGLProgramDataManager.h"
#include "GrTextureAccess.h"

/** @file
    This file contains specializations for OpenGL of the shader stages declared in
    include/gpu/GrProcessor.h. Objects of type GrGLProcessor are responsible for emitting the
    GLSL code that implements a GrProcessor and for uploading uniforms at draw time. If they don't
    always emit the same GLSL code, they must have a function:
        static inline void GenKey(const GrProcessor&, const GrGLCaps&, GrProcessorKeyBuilder*)
    that is used to implement a program cache. When two GrProcessors produce the same key this means
    that their GrGLProcessors would emit the same GLSL code.

    The GrGLProcessor subclass must also have a constructor of the form:
        ProcessorSubclass::ProcessorSubclass(const GrBackendProcessorFactory&, const GrProcessor&)

    These objects are created by the factory object returned by the GrProcessor::getFactory().
*/

class GrGLProcessor {
public:
    GrGLProcessor(const GrBackendProcessorFactory& factory)
        : fFactory(factory) {
    }

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

    virtual ~GrGLProcessor() {}

    const char* name() const { return fFactory.name(); }

    static void GenKey(const GrProcessor&, const GrGLCaps&, GrProcessorKeyBuilder*) {}

protected:
    const GrBackendProcessorFactory& fFactory;
};

class GrGLFPBuilder;

class GrGLFragmentProcessor : public GrGLProcessor {
public:
    GrGLFragmentProcessor(const GrBackendProcessorFactory& factory)
        : INHERITED(factory) {
    }

    virtual ~GrGLFragmentProcessor() {}

    /** Called when the program stage should insert its code into the shaders. The code in each
        shader will be in its own block ({}) and so locally scoped names will not collide across
        stages.

        @param builder      Interface used to emit code in the shaders.
        @param processor    The processor that generated this program stage.
        @param key          The key that was computed by GenKey() from the generating GrProcessor.
        @param outputColor  A predefined vec4 in the FS in which the stage should place its output
                            color (or coverage).
        @param inputColor   A vec4 that holds the input color to the stage in the FS. This may be
                            NULL in which case the implied input is solid white (all ones).
                            TODO: Better system for communicating optimization info (e.g. input
                            color is solid white, trans black, known to be opaque, etc.) that allows
                            the processor to communicate back similar known info about its output.
        @param samplers     Contains one entry for each GrTextureAccess of the GrProcessor. These
                            can be passed to the builder to emit texture reads in the generated
                            code.
        TODO this should take a struct
        */
    virtual void emitCode(GrGLFPBuilder* builder,
                          const GrFragmentProcessor&,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray& coords,
                          const TextureSamplerArray& samplers) = 0;

    /** A GrGLFragmentProcessor instance can be reused with any GrFragmentProcessor that produces
        the same stage key; this function reads data from a GrFragmentProcessor and uploads any
        uniform variables required by the shaders created in emitCode(). The GrFragmentProcessor
        parameter is guaranteed to be of the same type that created this GrGLFragmentProcessor and
        to have an identical processor key as the one that created this GrGLFragmentProcessor.  */
    // TODO update this to pass in GrFragmentProcessor
    virtual void setData(const GrGLProgramDataManager&, const GrProcessor&) {}

private:
    typedef GrGLProcessor INHERITED;
};

class GrGLXferProcessor : public GrGLFragmentProcessor {
public:
    GrGLXferProcessor(const GrBackendProcessorFactory& factory)
        : INHERITED(factory) {
    }
    
    virtual ~GrGLXferProcessor() {}

private:
    typedef GrGLFragmentProcessor INHERITED;
};

#endif
