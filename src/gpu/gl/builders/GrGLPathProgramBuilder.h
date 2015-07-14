/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrGLPathProgramBuilder_DEFINED
#define GrGLPathProgramBuilder_DEFINED

#include "GrGLProgramBuilder.h"

class GrGLPathProgramBuilder : public GrGLProgramBuilder {
public:
    GrGLPathProgramBuilder(GrGLGpu* gpu, const DrawArgs& args);

    GrGLProgram* createProgram(GrGLuint programID) override;

    SeparableVaryingHandle addSeparableVarying(const char* name, GrGLVertToFrag* v,
                                               GrSLPrecision fsPrecision) override;
    void bindProgramResourceLocations(GrGLuint programID) override;
    void resolveProgramResourceLocations(GrGLuint programID) override;

private:
    typedef GrGLPathProgramDataManager::SeparableVaryingInfo SeparableVaryingInfo;
    typedef GrGLPathProgramDataManager::SeparableVaryingInfoArray SeparableVaryingInfoArray;

    SeparableVaryingInfoArray fSeparableVaryingInfos;

    typedef GrGLProgramBuilder INHERITED;
};

#endif
