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
    virtual void createAndEmitEffects(const GrGeometryStage* geometryProcessor,
                                      const GrFragmentStage* colorStages[],
                                      const GrFragmentStage* coverageStages[],
                                      GrGLSLExpr4* inputColor,
                                      GrGLSLExpr4* inputCoverage) SK_OVERRIDE;

    GrGLProgramEffects* onCreateAndEmitEffects(const GrFragmentStage* effectStages[],
                                               int effectCnt,
                                               const GrGLProgramDesc::EffectKeyProvider&,
                                                   GrGLSLExpr4* inOutFSColor);

    virtual void emitEffect(const GrProcessorStage& stage,
                            const GrProcessorKey& key,
                            const char* outColor,
                            const char* inColor,
                            int stageIndex) SK_OVERRIDE;

    /**
     * Helper for emitEffect(). Allocates texture units from the builder for each transform in an
     * effect. The transforms all use adjacent texture units. They either use two or three of the
     * coordinates at a given texture unit, depending on if they need perspective interpolation.
     * The expressions to access the transformed coords (i.e. 'vec2(gl_TexCoord[0])') as well as the
     * types are appended to the TransformedCoordsArray* object, which is in turn passed to the
     * effect's emitCode() function.
     */
    void setupPathTexGen(const GrProcessorStage&, GrGLProcessor::TransformedCoordsArray*);

    virtual GrGLProgramEffects* getProgramEffects() SK_OVERRIDE { return fProgramEffects.get(); }

    typedef GrGLProgramDesc::EffectKeyProvider EffectKeyProvider;

    SkAutoTDelete<GrGLPathTexGenProgramEffects> fProgramEffects;

    typedef GrGLProgramBuilder INHERITED;
};

#endif
