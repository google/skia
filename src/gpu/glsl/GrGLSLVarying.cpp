/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "glsl/GrGLSLVarying.h"

#include "glsl/GrGLSLProgramBuilder.h"

void GrGLSLVaryingHandler::addPassThroughAttribute(const GrGeometryProcessor::Attribute* input,
                                                   const char* output) {
    GrSLType type = GrVertexAttribTypeToSLType(input->fType);
    GrGLSLVertToFrag v(type);
    this->addVarying(input->fName, &v);
    fProgramBuilder->fVS.codeAppendf("%s = %s;", v.vsOut(), input->fName);

    if (fProgramBuilder->primitiveProcessor().willUseGeoShader()) {
        fProgramBuilder->fGS.codeAppendf("%s = %s[0];", v.gsOut(), v.gsIn());
    }

    fProgramBuilder->fFS.codeAppendf("%s = %s;", output, v.fsIn());
}

void GrGLSLVaryingHandler::addVarying(const char* name,
                                      GrGLSLVarying* varying,
                                      GrSLPrecision precision) {
    SkASSERT(varying);
    if (varying->vsVarying()) {
        this->addVertexVarying(name, precision, varying);
    }
    if (fProgramBuilder->primitiveProcessor().willUseGeoShader()) {
        this->addGeomVarying(name, precision, varying);
    }
    if (varying->fsVarying()) {
        this->addFragVarying(precision, varying);
    }
}

void GrGLSLVaryingHandler::addVertexVarying(const char* name,
                                            GrSLPrecision precision,
                                            GrGLSLVarying* v) {
    fVertexOutputs.push_back();
    fVertexOutputs.back().setType(v->fType);
    fVertexOutputs.back().setTypeModifier(GrGLSLShaderVar::kVaryingOut_TypeModifier);
    fVertexOutputs.back().setPrecision(precision);
    fProgramBuilder->nameVariable(fVertexOutputs.back().accessName(), 'v', name);
    v->fVsOut = fVertexOutputs.back().getName().c_str();
}
void GrGLSLVaryingHandler::addGeomVarying(const char* name,
                                          GrSLPrecision precision,
                                          GrGLSLVarying* v) {
    // if we have a GS take each varying in as an array
    // and output as non-array.
    if (v->vsVarying()) {
        fGeomInputs.push_back();
        fGeomInputs.back().setType(v->fType);
        fGeomInputs.back().setTypeModifier(GrGLSLShaderVar::kVaryingIn_TypeModifier);
        fGeomInputs.back().setPrecision(precision);
        fGeomInputs.back().setUnsizedArray();
        *fGeomInputs.back().accessName() = v->fVsOut;
        v->fGsIn = v->fVsOut;
    }

    if (v->fsVarying()) {
        fGeomOutputs.push_back();
        fGeomOutputs.back().setType(v->fType);
        fGeomOutputs.back().setTypeModifier(GrGLSLShaderVar::kVaryingOut_TypeModifier);
        fGeomOutputs.back().setPrecision(precision);
        fProgramBuilder->nameVariable(fGeomOutputs.back().accessName(), 'g', name);
        v->fGsOut = fGeomOutputs.back().getName().c_str();
    }
}

void GrGLSLVaryingHandler::addFragVarying(GrSLPrecision precision, GrGLSLVarying* v) {
    v->fFsIn = v->fGsOut ? v->fGsOut : v->fVsOut;
    fFragInputs.push_back().set(v->fType,
                                GrGLSLShaderVar::kVaryingIn_TypeModifier,
                                v->fFsIn,
                                precision);
}

void GrGLSLVaryingHandler::emitAttributes(const GrGeometryProcessor& gp) {
    int vaCount = gp.numAttribs();
    for (int i = 0; i < vaCount; i++) {
        const GrGeometryProcessor::Attribute& attr = gp.getAttrib(i);
        this->addAttribute(GrShaderVar(attr.fName,
                                       GrVertexAttribTypeToSLType(attr.fType),
                                       GrShaderVar::kAttribute_TypeModifier,
                                       GrShaderVar::kNonArray,
                                       attr.fPrecision));
    }
}

void GrGLSLVaryingHandler::addAttribute(const GrShaderVar& var) {
    SkASSERT(GrShaderVar::kAttribute_TypeModifier == var.getTypeModifier());
    for (int j = 0; j < fVertexInputs.count(); ++j) {
        const GrGLSLShaderVar& attr = fVertexInputs[j];
        // if attribute already added, don't add it again
        if (attr.getName().equals(var.getName())) {
            return;
        }
    }
    fVertexInputs.push_back(var);
}

void GrGLSLVaryingHandler::appendDecls(const VarArray& vars, SkString* out) const {
    for (int i = 0; i < vars.count(); ++i) {
        vars[i].appendDecl(fProgramBuilder->glslCaps(), out);
        out->append(";");
    }
}

void GrGLSLVaryingHandler::getVertexDecls(SkString* inputDecls, SkString* outputDecls) const {
    this->appendDecls(fVertexInputs, inputDecls);
    this->appendDecls(fVertexOutputs, outputDecls);
}

void GrGLSLVaryingHandler::getGeomDecls(SkString* inputDecls, SkString* outputDecls) const {
    this->appendDecls(fGeomInputs, inputDecls);
    this->appendDecls(fGeomOutputs, outputDecls);
}

void GrGLSLVaryingHandler::getFragDecls(SkString* inputDecls, SkString* outputDecls) const {
    // We should not have any outputs in the fragment shader when using version 1.10
    SkASSERT(k110_GrGLSLGeneration != fProgramBuilder->glslCaps()->generation() ||
             fFragOutputs.empty());
    this->appendDecls(fFragInputs, inputDecls);
    this->appendDecls(fFragOutputs, outputDecls);
}

