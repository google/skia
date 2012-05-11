/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLShaderBuilder_DEFINED
#define GrGLShaderBuilder_DEFINED

#include "GrAllocator.h"
#include "gl/GrGLShaderVar.h"
#include "gl/GrGLSL.h"

typedef GrTAllocator<GrGLShaderVar> VarArray;

/**
  Containts all the incremental state of a shader as it is being built,
  as well as helpers to manipulate that state.
  TODO: migrate CompileShaders() here?
*/

class GrGLShaderBuilder {

public:

    GrGLShaderBuilder();

    void appendVarying(GrSLType type,
                       const char* name,
                       const char** vsOutName = NULL,
                       const char** fsInName = NULL);

    void appendVarying(GrSLType type,
                       const char* name,
                       int stageNum,
                       const char** vsOutName = NULL,
                       const char** fsInName = NULL);

    GrStringBuilder fHeader; // VS+FS, GLSL version, etc
    VarArray        fVSUnis;
    VarArray        fVSAttrs;
    VarArray        fVSOutputs;
    VarArray        fGSInputs;
    VarArray        fGSOutputs;
    VarArray        fFSInputs;
    GrStringBuilder fGSHeader; // layout qualifiers specific to GS
    VarArray        fFSUnis;
    VarArray        fFSOutputs;
    GrStringBuilder fFSFunctions;
    GrStringBuilder fVSCode;
    GrStringBuilder fGSCode;
    GrStringBuilder fFSCode;

    bool            fUsesGS;
};

#endif
