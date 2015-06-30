/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLPathProcessor_DEFINED
#define GrGLPathProcessor_DEFINED

#include "GrGLPrimitiveProcessor.h"

class GrPathProcessor;
class GrGLPathRendering;
class GrGLGpu;
class GrGLPathProgramDataManager;

class GrGLPathProcessor : public GrGLPrimitiveProcessor {
public:
    GrGLPathProcessor(const GrPathProcessor&, const GrBatchTracker&);

    static void GenKey(const GrPathProcessor&,
                       const GrBatchTracker& bt,
                       const GrGLSLCaps&,
                       GrProcessorKeyBuilder* b);

    void emitCode(EmitArgs&) override;

    void emitTransforms(GrGLGPBuilder*, const TransformsIn&, TransformsOut*);

    void bindSeparableVaryings(GrGLGpu* gpu, GrGLuint programID);
    void resolveSeparableVaryings(GrGLGpu* gpu, GrGLuint programId);

    void setData(const GrGLProgramDataManager&,
                 const GrPrimitiveProcessor&,
                 const GrBatchTracker&) override;

    void setTransformData(const GrPrimitiveProcessor&,
                          const GrGLPathProgramDataManager&,
                          int index,
                          const SkTArray<const GrCoordTransform*, true>& transforms);

    virtual void didSetData(GrGLPathRendering*) {}

private:
    UniformHandle fColorUniform;
    GrColor fColor;

    typedef GrGLPrimitiveProcessor INHERITED;
};

#endif
