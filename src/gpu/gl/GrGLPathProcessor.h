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

class GrGLPathProcessor : public GrGLPrimitiveProcessor {
public:
    GrGLPathProcessor(const GrPathProcessor&, const GrBatchTracker&);

    static void GenKey(const GrPathProcessor&,
                       const GrBatchTracker& bt,
                       const GrGLSLCaps&,
                       GrProcessorKeyBuilder* b);

    void emitCode(EmitArgs&) override;

    void emitTransforms(GrGLGPBuilder*, const TransformsIn&, TransformsOut*);

    void resolveSeparableVaryings(GrGLGpu* gpu, GrGLuint programId);

    void setData(const GrGLProgramDataManager&,
                 const GrPrimitiveProcessor&,
                 const GrBatchTracker&) override;

    void setTransformData(const GrPrimitiveProcessor&,
                          int index,
                          const SkTArray<const GrCoordTransform*, true>& transforms,
                          GrGLPathRendering*,
                          GrGLuint programID);

    virtual void didSetData(GrGLPathRendering*) {}

private:
    UniformHandle fColorUniform;
    GrColor fColor;
    struct SeparableVaryingInfo {
        GrSLType      fType;
        GrGLShaderVar fVariable;
        GrGLint       fLocation;
    };

    typedef SkSTArray<8, SeparableVaryingInfo, true> SeparableVaryingInfoArray;

    SeparableVaryingInfoArray fSeparableVaryingInfos;

    typedef GrGLPrimitiveProcessor INHERITED;
};

#endif
