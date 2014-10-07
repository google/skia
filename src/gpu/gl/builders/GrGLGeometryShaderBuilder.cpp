/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLGeometryShaderBuilder.h"
#include "GrGLShaderStringBuilder.h"
#include "GrGLProgramBuilder.h"
#include "../GrGpuGL.h"

GrGLGeometryShaderBuilder::GrGLGeometryShaderBuilder(GrGLFullProgramBuilder* program)
    : INHERITED(program) {

}

void GrGLGeometryShaderBuilder::addVarying(GrSLType type,
               const char* name,
               const char** gsOutName) {
    // if we have a GS take each varying in as an array
    // and output as non-array.
    fInputs.push_back();
    fInputs.back().setType(type);
    fInputs.back().setTypeModifier(GrGLShaderVar::kVaryingIn_TypeModifier);
    fInputs.back().setUnsizedArray();
    *fInputs.back().accessName() = name;
    fOutputs.push_back();
    fOutputs.back().setType(type);
    fOutputs.back().setTypeModifier(GrGLShaderVar::kVaryingOut_TypeModifier);
    fProgramBuilder->nameVariable(fOutputs.back().accessName(), 'g', name);
    if (gsOutName) {
        *gsOutName = fOutputs.back().getName().c_str();
    }
}


bool GrGLGeometryShaderBuilder::compileAndAttachShaders(GrGLuint programId,
        SkTDArray<GrGLuint>* shaderIds) const {
    const GrGLContext& glCtx = fProgramBuilder->gpu()->glContext();
    SkASSERT(fProgramBuilder->ctxInfo().glslGeneration() >= k150_GrGLSLGeneration);
    SkString geomShaderSrc(GrGetGLSLVersionDecl(fProgramBuilder->ctxInfo()));
    geomShaderSrc.append("layout(triangles) in;\n"
                         "layout(triangle_strip, max_vertices = 6) out;\n");
    fProgramBuilder->appendDecls(fInputs, &geomShaderSrc);
    fProgramBuilder->appendDecls(fOutputs, &geomShaderSrc);
    geomShaderSrc.append("void main() {\n");
    geomShaderSrc.append("\tfor (int i = 0; i < 3; ++i) {\n"
                         "\t\tgl_Position = gl_in[i].gl_Position;\n");
    if (fProgramBuilder->desc().getHeader().fEmitsPointSize) {
        geomShaderSrc.append("\t\tgl_PointSize = 1.0;\n");
    }
    SkASSERT(fInputs.count() == fOutputs.count());
    for (int i = 0; i < fInputs.count(); ++i) {
        geomShaderSrc.appendf("\t\t%s = %s[i];\n",
                              fOutputs[i].getName().c_str(),
                              fInputs[i].getName().c_str());
    }
    geomShaderSrc.append("\t\tEmitVertex();\n"
                         "\t}\n"
                         "\tEndPrimitive();\n");
    geomShaderSrc.append("}\n");
    GrGLuint geomShaderId =
        GrGLCompileAndAttachShader(glCtx, programId,
                                   GR_GL_GEOMETRY_SHADER, geomShaderSrc,
                                   fProgramBuilder->gpu()->gpuStats());
    if (!geomShaderId) {
        return false;
    }
    *shaderIds->append() = geomShaderId;
    return true;
}
