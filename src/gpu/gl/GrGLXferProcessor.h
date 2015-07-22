/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLXferProcessor_DEFINED
#define GrGLXferProcessor_DEFINED

#include "GrGLFragmentProcessor.h"

class GrGLXPBuilder;
class GrXferProcessor;

class GrGLXferProcessor {
public:
    GrGLXferProcessor() {}
    virtual ~GrGLXferProcessor() {}

    typedef GrGLProcessor::TextureSamplerArray TextureSamplerArray;
    struct EmitArgs {
        EmitArgs(GrGLXPBuilder* pb,
                 const GrXferProcessor& xp,
                 const char* inputColor,
                 const char* inputCoverage,
                 const char* outputPrimary,
                 const char* outputSecondary,
                 const TextureSamplerArray& samplers)
            : fPB(pb)
            , fXP(xp)
            , fInputColor(inputColor)
            , fInputCoverage(inputCoverage)
            , fOutputPrimary(outputPrimary)
            , fOutputSecondary(outputSecondary)
            , fSamplers(samplers) {}

        GrGLXPBuilder* fPB;
        const GrXferProcessor& fXP;
        const char* fInputColor;
        const char* fInputCoverage;
        const char* fOutputPrimary;
        const char* fOutputSecondary;
        const TextureSamplerArray& fSamplers;
    };
    /**
     * This is similar to emitCode() in the base class, except it takes a full shader builder.
     * This allows the effect subclass to emit vertex code.
     */
    void emitCode(const EmitArgs&);

    /** A GrGLXferProcessor instance can be reused with any GrGLXferProcessor that produces
        the same stage key; this function reads data from a GrGLXferProcessor and uploads any
        uniform variables required  by the shaders created in emitCode(). The GrXferProcessor
        parameter is guaranteed to be of the same type that created this GrGLXferProcessor and
        to have an identical processor key as the one that created this GrGLXferProcessor. This
        function calls onSetData on the subclass of GrGLXferProcessor
     */
    void setData(const GrGLProgramDataManager& pdm, const GrXferProcessor& xp);

private:
    /**
     * Called by emitCode() when the XP will not be performing a dst read. This method is
     * responsible for both blending and coverage. A subclass only needs to implement this method if
     * it can construct a GrXferProcessor that will not read the dst color.
     */
    virtual void emitOutputsForBlendState(const EmitArgs&) {
        SkFAIL("emitOutputsForBlendState not implemented.");
    }

    /**
     * Called by emitCode() when the XP will perform a dst read. This method only needs to supply
     * the blending logic. The base class applies coverage. A subclass only needs to implement this
     * method if it can construct a GrXferProcessor that reads the dst color.
     */
    virtual void emitBlendCodeForDstRead(GrGLXPBuilder*, const char* srcColor, const char* dstColor,
                                         const char* outColor, const GrXferProcessor&) {
        SkFAIL("emitBlendCodeForDstRead not implemented.");
    }

    virtual void onSetData(const GrGLProgramDataManager&, const GrXferProcessor&) = 0;

    GrGLProgramDataManager::UniformHandle fDstTopLeftUni;
    GrGLProgramDataManager::UniformHandle fDstScaleUni;

    typedef GrGLProcessor INHERITED;
};
#endif
