/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLShaderStringBuilder_DEFINED
#define GrGLShaderStringBuilder_DEFINED

#include "GrAllocator.h"
#include "GrGpu.h"
#include "gl/GrGLContext.h"
#include "SkSLGLSLCodeGenerator.h"
#include "SkTypes.h"

GrGLuint GrGLCompileAndAttachShader(const GrGLContext& glCtx,
                                    GrGLuint programId,
                                    GrGLenum type,
                                    const char** skslStrings,
                                    int* lengths,
                                    int count,
                                    GrGpu::Stats*,
                                    const SkSL::Program::Settings& settings,
                                    SkSL::Program::Inputs* inputs);

void GrGLPrintShader(const GrGLContext&, GrGLenum type, const char** skslStrings, int* lengths,
                     int count, const SkSL::Program::Settings&);

#endif
