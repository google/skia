/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrShaderCaps.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLProgramBuilder.h"

void GrGLSLVaryingHandler::addPassThroughAttribute(const GrGeometryProcessor::Attribute* input,
                                                   const char* output) {
    GrSLType type = GrVertexAttribTypeToSLType(input->fType);
    GrGLSLVarying v(type);
    this->addVarying(input->fName, &v);
    this->writePassThroughAttribute(input, output, v);
}

void GrGLSLVaryingHandler::addFlatPassThroughAttribute(const GrGeometryProcessor::Attribute* input,
                                                       const char* output) {
    GrSLType type = GrVertexAttribTypeToSLType(input->fType);
    GrGLSLVarying v(type);
    this->addFlatVarying(input->fName, &v);
    this->writePassThroughAttribute(input, output, v);
}

void GrGLSLVaryingHandler::writePassThroughAttribute(const GrGeometryProcessor::Attribute* input,
                                                     const char* output, const GrGLSLVarying& v) {
    SkASSERT(!fProgramBuilder->primitiveProcessor().willUseGeoShader());
    fProgramBuilder->fVS.codeAppendf("%s = %s;", v.vsOut(), input->fName);
    fProgramBuilder->fFS.codeAppendf("%s = %s;", output, v.fsIn());
}

void GrGLSLVaryingHandler::internalAddVarying(const char* name, GrGLSLVarying* varying, bool flat) {
    bool willUseGeoShader = fProgramBuilder->primitiveProcessor().willUseGeoShader();
    VaryingInfo& v = fVaryings.push_back();

    SkASSERT(varying);
    SkASSERT(kVoid_GrSLType != varying->fType);
    v.fType = varying->fType;
    v.fIsFlat = flat;
    fProgramBuilder->nameVariable(&v.fVsOut, 'v', name);
    v.fVisibility = kNone_GrShaderFlags;
    if (varying->isInVertexShader()) {
        varying->fVsOut = v.fVsOut.c_str();
        v.fVisibility |= kVertex_GrShaderFlag;
    }
    if (willUseGeoShader) {
        fProgramBuilder->nameVariable(&v.fGsOut, 'g', name);
        varying->fGsIn = v.fVsOut.c_str();
        varying->fGsOut = v.fGsOut.c_str();
        v.fVisibility |= kGeometry_GrShaderFlag;
    }
    if (varying->isInFragmentShader()) {
        varying->fFsIn = (willUseGeoShader ? v.fGsOut : v.fVsOut).c_str();
        v.fVisibility |= kFragment_GrShaderFlag;
    }
}

void GrGLSLVaryingHandler::emitAttributes(const GrGeometryProcessor& gp) {
    int vaCount = gp.numAttribs();
    for (int i = 0; i < vaCount; i++) {
        const GrGeometryProcessor::Attribute& attr = gp.getAttrib(i);
        this->addAttribute(GrShaderVar(attr.fName,
                                       GrVertexAttribTypeToSLType(attr.fType),
                                       GrShaderVar::kIn_TypeModifier,
                                       GrShaderVar::kNonArray));
    }
}

void GrGLSLVaryingHandler::addAttribute(const GrShaderVar& var) {
    SkASSERT(GrShaderVar::kIn_TypeModifier == var.getTypeModifier());
    for (int j = 0; j < fVertexInputs.count(); ++j) {
        const GrShaderVar& attr = fVertexInputs[j];
        // if attribute already added, don't add it again
        if (attr.getName().equals(var.getName())) {
            return;
        }
    }
    fVertexInputs.push_back(var);
}

void GrGLSLVaryingHandler::setNoPerspective() {
    const GrShaderCaps& caps = *fProgramBuilder->shaderCaps();
    if (!caps.noperspectiveInterpolationSupport()) {
        return;
    }
    if (const char* extension = caps.noperspectiveInterpolationExtensionString()) {
        int bit = 1 << GrGLSLFragmentBuilder::kNoPerspectiveInterpolation_GLSLPrivateFeature;
        fProgramBuilder->fVS.addFeature(bit, extension);
        if (fProgramBuilder->primitiveProcessor().willUseGeoShader()) {
            fProgramBuilder->fGS.addFeature(bit, extension);
        }
        fProgramBuilder->fFS.addFeature(bit, extension);
    }
    fDefaultInterpolationModifier = "noperspective";
}

void GrGLSLVaryingHandler::finalize() {
    for (int i = 0; i < fVaryings.count(); ++i) {
        const VaryingInfo& v = this->fVaryings[i];
        const char* modifier = v.fIsFlat ? "flat" : fDefaultInterpolationModifier;
        if (v.fVisibility & kVertex_GrShaderFlag) {
            fVertexOutputs.push_back().set(v.fType, v.fVsOut, GrShaderVar::kOut_TypeModifier,
                                           kDefault_GrSLPrecision, nullptr, modifier);
            if (v.fVisibility & kGeometry_GrShaderFlag) {
                fGeomInputs.push_back().set(v.fType, v.fVsOut, GrShaderVar::kUnsizedArray,
                                            GrShaderVar::kIn_TypeModifier, kDefault_GrSLPrecision,
                                            nullptr, modifier);
            }
        }
        if (v.fVisibility & kFragment_GrShaderFlag) {
            const char* fsIn = v.fVsOut.c_str();
            if (v.fVisibility & kGeometry_GrShaderFlag) {
                fGeomOutputs.push_back().set(v.fType, v.fGsOut, GrShaderVar::kOut_TypeModifier,
                                             kDefault_GrSLPrecision, nullptr, modifier);
                fsIn = v.fGsOut.c_str();
            }
            fFragInputs.push_back().set(v.fType, fsIn, GrShaderVar::kIn_TypeModifier,
                                        kDefault_GrSLPrecision, nullptr, modifier);
        }
    }
    this->onFinalize();
}

void GrGLSLVaryingHandler::appendDecls(const VarArray& vars, SkString* out) const {
    for (int i = 0; i < vars.count(); ++i) {
        vars[i].appendDecl(fProgramBuilder->shaderCaps(), out);
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
    SkASSERT(k110_GrGLSLGeneration != fProgramBuilder->shaderCaps()->generation() ||
             fFragOutputs.empty());
    this->appendDecls(fFragInputs, inputDecls);
    this->appendDecls(fFragOutputs, outputDecls);
}
