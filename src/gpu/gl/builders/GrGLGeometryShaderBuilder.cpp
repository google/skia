/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLGeometryShaderBuilder.h"
#include "GrGLShaderStringBuilder.h"
#include "GrGLProgramBuilder.h"
#include "../GrGLGpu.h"

GrGLGeometryBuilder::GrGLGeometryBuilder(GrGLProgramBuilder* program)
    : INHERITED(program) {

}

void GrGLGeometryBuilder::addVarying(const char* name, GrGLVarying* v) {
    // if we have a GS take each varying in as an array
    // and output as non-array.
    if (v->vsVarying()) {
        fInputs.push_back();
        fInputs.back().setType(v->fType);
        fInputs.back().setTypeModifier(GrGLShaderVar::kVaryingIn_TypeModifier);
        fInputs.back().setUnsizedArray();
        *fInputs.back().accessName() = v->fVsOut;
        v->fGsIn = v->fVsOut;
    }

    if (v->fsVarying()) {
        fOutputs.push_back();
        fOutputs.back().setType(v->fType);
        fOutputs.back().setTypeModifier(GrGLShaderVar::kVaryingOut_TypeModifier);
        fProgramBuilder->nameVariable(fOutputs.back().accessName(), 'g', name);
        v->fGsOut = fOutputs.back().getName().c_str();
    }
}

bool GrGLGeometryBuilder::compileAndAttachShaders(GrGLuint programId,
        SkTDArray<GrGLuint>* shaderIds) const {
    const GrGLContext& glCtx = fProgramBuilder->gpu()->glContext();
    SkASSERT(fProgramBuilder->ctxInfo().glslGeneration() >= k150_GrGLSLGeneration);
    SkString geomShaderSrc(GrGetGLSLVersionDecl(fProgramBuilder->ctxInfo()));
    geomShaderSrc.append("layout(triangles) in;\n"
                         "layout(triangle_strip, max_vertices = 6) out;\n");
    this->appendDecls(fInputs, &geomShaderSrc);
    this->appendDecls(fOutputs, &geomShaderSrc);
    geomShaderSrc.append("void main() {\n");
    geomShaderSrc.append("\tfor (int i = 0; i < 3; ++i) {\n"
                         "\t\tgl_Position = gl_in[i].gl_Position;\n");
    geomShaderSrc.append("\t\tgl_PointSize = 1.0;\n");
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
                                   fProgramBuilder->gpu()->stats());
    if (!geomShaderId) {
        return false;
    }
    *shaderIds->append() = geomShaderId;
    return true;
}
