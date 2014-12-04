/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLGeometryProcessor_DEFINED
#define GrGLGeometryProcessor_DEFINED

#include "GrGLProcessor.h"

class GrBatchTracker;
class GrGLGPBuilder;

/**
 * If a GL effect needs a GrGLFullShaderBuilder* object to emit vertex code, then it must inherit
 * from this class. Since paths don't have vertices, this class is only meant to be used internally
 * by skia, for special cases.
 */
class GrGLGeometryProcessor {
public:
    GrGLGeometryProcessor() {}
    virtual ~GrGLGeometryProcessor() {}

    typedef GrGLProcessor::TextureSamplerArray TextureSamplerArray;
    struct EmitArgs {
        EmitArgs(GrGLGPBuilder* pb,
                 const GrGeometryProcessor& gp,
                 const GrBatchTracker& bt,
                 const char* outputColor,
                 const char* outputCoverage,
                 const TextureSamplerArray& samplers)
            : fPB(pb)
            , fGP(gp)
            , fBT(bt)
            , fOutputColor(outputColor)
            , fOutputCoverage(outputCoverage)
            , fSamplers(samplers) {}
        GrGLGPBuilder* fPB;
        const GrGeometryProcessor& fGP;
        const GrBatchTracker& fBT;
        const char* fOutputColor;
        const char* fOutputCoverage;
        const TextureSamplerArray& fSamplers;
    };
    /**
     * This is similar to emitCode() in the base class, except it takes a full shader builder.
     * This allows the effect subclass to emit vertex code.
     */
    virtual void emitCode(const EmitArgs&) = 0;

    /** A GrGLGeometryProcessor instance can be reused with any GrGLGeometryProcessor that produces
        the same stage key; this function reads data from a GrGLGeometryProcessor and uploads any
        uniform variables required  by the shaders created in emitCode(). The GrGeometryProcessor
        parameter is guaranteed to be of the same type that created this GrGLGeometryProcessor and
        to have an identical processor key as the one that created this GrGLGeometryProcessor.  */
    virtual void setData(const GrGLProgramDataManager&,
                         const GrGeometryProcessor&,
                         const GrBatchTracker&) = 0;

private:
    typedef GrGLProcessor INHERITED;
};

#endif
