/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLLegacyNvprProgramBuilder_DEFINED
#define GrGLLegacyNvprProgramBuilder_DEFINED

#include "GrGLProgramBuilder.h"

class GrGLLegacyNvprProgramBuilder : public GrGLProgramBuilder {
public:
    GrGLLegacyNvprProgramBuilder(GrGLGpu*, const GrOptDrawState&);

    virtual GrGLProgram* createProgram(GrGLuint programID) SK_OVERRIDE;

private:
    int addTexCoordSets(int count);
    void emitTransforms(const GrPendingFragmentStage&,
                        GrGLProcessor::TransformedCoordsArray* outCoords,
                        GrGLInstalledFragProc*) SK_OVERRIDE;

    int fTexCoordSetCnt;

    typedef GrGLProgramBuilder INHERITED;
};

#endif
