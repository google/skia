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
                       const GrGLCaps&,
                       GrProcessorKeyBuilder* b);

    void emitCode(EmitArgs&) SK_OVERRIDE;

    virtual void emitTransforms(GrGLGPBuilder*, const TransformsIn&, TransformsOut*) = 0;

    virtual void resolveSeparableVaryings(GrGLGpu* gpu, GrGLuint programId) {}

    void setData(const GrGLProgramDataManager&,
                 const GrPrimitiveProcessor&,
                 const GrBatchTracker&) SK_OVERRIDE;

    virtual void setTransformData(const GrPrimitiveProcessor&,
                                  int index,
                                  const SkTArray<const GrCoordTransform*, true>& transforms,
                                  GrGLPathRendering*,
                                  GrGLuint programID) = 0;

    virtual void didSetData(GrGLPathRendering*) {}

private:
    UniformHandle fColorUniform;
    GrColor fColor;

    typedef GrGLPrimitiveProcessor INHERITED;
};

class GrGLLegacyPathProcessor : public GrGLPathProcessor {
public:
    GrGLLegacyPathProcessor(const GrPathProcessor& pathProc, const GrBatchTracker& bt,
                            int maxTexCoords)
        : INHERITED(pathProc, bt)
        , fTexCoordSetCnt(0) {
        SkDEBUGCODE(fMaxTexCoords = maxTexCoords;)
    }

    int addTexCoordSets(int count) {
        int firstFreeCoordSet = fTexCoordSetCnt;
        fTexCoordSetCnt += count;
        SkASSERT(fMaxTexCoords >= fTexCoordSetCnt);
        return firstFreeCoordSet;
    }

    void emitTransforms(GrGLGPBuilder*, const TransformsIn& tin, TransformsOut* tout) SK_OVERRIDE;

    void setTransformData(const GrPrimitiveProcessor& primProc,
                          int index,
                          const SkTArray<const GrCoordTransform*, true>& transforms,
                          GrGLPathRendering* glpr,
                          GrGLuint) SK_OVERRIDE;

    void didSetData(GrGLPathRendering* glpr) SK_OVERRIDE;

private:
    SkDEBUGCODE(int fMaxTexCoords;)
    int fTexCoordSetCnt;

    typedef GrGLPathProcessor INHERITED;
};

class GrGLNormalPathProcessor : public GrGLPathProcessor {
public:
    GrGLNormalPathProcessor(const GrPathProcessor& pathProc, const GrBatchTracker& bt)
        : INHERITED(pathProc, bt) {}

    void emitTransforms(GrGLGPBuilder* pb, const TransformsIn& tin,TransformsOut* tout) SK_OVERRIDE;

    void resolveSeparableVaryings(GrGLGpu* gpu, GrGLuint programId);

    void setTransformData(const GrPrimitiveProcessor& primProc,
                          int index,
                          const SkTArray<const GrCoordTransform*, true>& coordTransforms,
                          GrGLPathRendering* glpr,
                          GrGLuint programID) SK_OVERRIDE;

private:
    struct SeparableVaryingInfo {
        GrSLType      fType;
        GrGLShaderVar fVariable;
        GrGLint       fLocation;
    };

    typedef SkSTArray<8, SeparableVaryingInfo, true> SeparableVaryingInfoArray;

    SeparableVaryingInfoArray fSeparableVaryingInfos;

    typedef GrGLPathProcessor INHERITED;
};

#endif
