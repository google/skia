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
    GrGLLegacyNvprProgramBuilder(GrGpuGL*, const GrOptDrawState&);

    virtual GrGLProgram* createProgram(GrGLuint programID);

private:
    int addTexCoordSets(int count);
    void emitTransforms(const GrFragmentStage&,
                        GrGLProcessor::TransformedCoordsArray* outCoords,
                        GrGLInstalledFragProc*);

    int fTexCoordSetCnt;

    typedef GrGLProgramBuilder INHERITED;
};

#endif
