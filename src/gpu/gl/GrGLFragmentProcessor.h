/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLFragmentProcessor_DEFINED
#define GrGLFragmentProcessor_DEFINED

#include "GrGLProgramDataManager.h"
#include "GrGLProcessor.h"
#include "GrTextureAccess.h"

class GrGLFPBuilder;

class GrGLFragmentProcessor {
public:
    GrGLFragmentProcessor() {}

    virtual ~GrGLFragmentProcessor() {}

    typedef GrGLProgramDataManager::UniformHandle UniformHandle;
    typedef GrGLProcessor::TransformedCoordsArray TransformedCoordsArray;
    typedef GrGLProcessor::TextureSamplerArray TextureSamplerArray;

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
        */

    struct EmitArgs {
        EmitArgs(GrGLFPBuilder* builder,
                 const GrFragmentProcessor& fp,
                 const char* outputColor,
                 const char* inputColor,
                 const TransformedCoordsArray& coords,
                 const TextureSamplerArray& samplers)
            : fBuilder(builder)
            , fFp(fp)
            , fOutputColor(outputColor)
            , fInputColor(inputColor)
            , fCoords(coords)
            , fSamplers(samplers) {}
        GrGLFPBuilder* fBuilder;
        const GrFragmentProcessor& fFp;
        const char* fOutputColor;
        const char* fInputColor;
        const TransformedCoordsArray& fCoords;
        const TextureSamplerArray& fSamplers;
    };

    virtual void emitCode(EmitArgs&) = 0;

    /** A GrGLFragmentProcessor instance can be reused with any GrFragmentProcessor that produces
        the same stage key; this function reads data from a GrFragmentProcessor and uploads any
        uniform variables required by the shaders created in emitCode(). The GrFragmentProcessor
        parameter is guaranteed to be of the same type that created this GrGLFragmentProcessor and
        to have an identical processor key as the one that created this GrGLFragmentProcessor.  */
    // TODO update this to pass in GrFragmentProcessor
    virtual void setData(const GrGLProgramDataManager&, const GrProcessor&) {}

    static void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*) {}

private:
    typedef GrGLProcessor INHERITED;
};

#endif
