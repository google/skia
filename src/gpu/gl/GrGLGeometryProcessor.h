/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLGeometryProcessor_DEFINED
#define GrGLGeometryProcessor_DEFINED

#include "GrGLProcessor.h"

class GrGLGPBuilder;

/**
 * If a GL effect needs a GrGLFullShaderBuilder* object to emit vertex code, then it must inherit
 * from this class. Since paths don't have vertices, this class is only meant to be used internally
 * by skia, for special cases.
 */
class GrGLGeometryProcessor : public GrGLProcessor {
public:
    GrGLGeometryProcessor(const GrBackendProcessorFactory& factory)
        : INHERITED(factory) {}

    struct EmitArgs {
        EmitArgs(GrGLGPBuilder* pb,
                 const GrGeometryProcessor& gp,
                 const GrProcessorKey& key,
                 const char* output,
                 const char* input,
                 const TextureSamplerArray& samplers)
            : fPB(pb), fGP(gp), fKey(key), fOutput(output), fInput(input), fSamplers(samplers) {}
        GrGLGPBuilder* fPB;
        const GrGeometryProcessor& fGP;
        const GrProcessorKey& fKey;
        const char* fOutput;
        const char* fInput;
        const TextureSamplerArray& fSamplers;
    };
    /**
     * This is similar to emitCode() in the base class, except it takes a full shader builder.
     * This allows the effect subclass to emit vertex code.
     */
    virtual void emitCode(const EmitArgs&) = 0;

private:
    typedef GrGLProcessor INHERITED;
};

#endif
