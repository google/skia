/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLFragmentOnlyProgramBuilder_DEFINED
#define GrGLFragmentOnlyProgramBuilder_DEFINED

#include "GrGLProgramBuilder.h"

class GrGLFragmentOnlyProgramBuilder : public GrGLProgramBuilder {
public:
    GrGLFragmentOnlyProgramBuilder(GrGpuGL*, const GrGLProgramDesc&);

    int addTexCoordSets(int count);

private:
    virtual void emitCodeBeforeEffects(GrGLSLExpr4* color,
                                       GrGLSLExpr4* coverage) SK_OVERRIDE {}

    virtual void emitGeometryProcessor(const GrEffectStage* geometryProcessor,
                                       GrGLSLExpr4* coverage) SK_OVERRIDE {
        SkASSERT(NULL == geometryProcessor);
    }

    virtual GrGLProgramEffects* createAndEmitEffects(const GrEffectStage* effectStages[],
                                                     int effectCnt,
                                                     const GrGLProgramDesc::EffectKeyProvider&,
                                                     GrGLSLExpr4* inOutFSColor) SK_OVERRIDE;

    virtual void emitCodeAfterEffects() SK_OVERRIDE {}

    typedef GrGLProgramBuilder INHERITED;
};

#endif
