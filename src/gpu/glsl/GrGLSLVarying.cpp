/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "glsl/GrGLSLVarying.h"

#include "glsl/GrGLSLProgramBuilder.h"

void GrGLSLVaryingHandler::addPassThroughAttribute(const GrGeometryProcessor::Attribute* input,
                                                   const char* output, GrSLPrecision precision) {
    GrSLType type = GrVertexAttribTypeToSLType(input->fType);
    GrGLSLVertToFrag v(type);
    this->addVarying(input->fName, &v, precision);
    this->writePassThroughAttribute(input, output, v);
}

void GrGLSLVaryingHandler::addFlatPassThroughAttribute(const GrGeometryProcessor::Attribute* input,
                                                       const char* output,
                                                       GrSLPrecision precision) {
    GrSLType type = GrVertexAttribTypeToSLType(input->fType);
    GrGLSLVertToFrag v(type);
    this->addFlatVarying(input->fName, &v, precision);
    this->writePassThroughAttribute(input, output, v);
}

void GrGLSLVaryingHandler::writePassThroughAttribute(const GrGeometryProcessor::Attribute* input,
                                                     const char* output, const GrGLSLVarying& v) {
    fProgramBuilder->fVS.codeAppendf("%s = %s;", v.vsOut(), input->fName);

    if (fProgramBuilder->primitiveProcessor().willUseGeoShader()) {
        fProgramBuilder->fGS.codeAppendf("%s = %s[0];", v.gsOut(), v.gsIn());
    }

    fProgramBuilder->fFS.codeAppendf("%s = %s;", output, v.fsIn());
}

void GrGLSLVaryingHandler::internalAddVarying(const char* name,
                                              GrGLSLVarying* varying,
                                              GrSLPrecision precision,
                                              bool flat) {
    bool willUseGeoShader = fProgramBuilder->primitiveProcessor().willUseGeoShader();
    VaryingInfo& v = fVaryings.push_back();

    SkASSERT(varying);
    v.fType = varying->fType;
    v.fPrecision = precision;
    v.fIsFlat = flat;
    fProgramBuilder->nameVariable(&v.fVsOut, 'v', name);
    v.fVisibility = kNone_GrShaderFlags;
    if (varying->vsVarying()) {
        varying->fVsOut = v.fVsOut.c_str();
        v.fVisibility |= kVertex_GrShaderFlag;
    }
    if (willUseGeoShader) {
        fProgramBuilder->nameVariable(&v.fGsOut, 'g', name);
        varying->fGsIn = v.fVsOut.c_str();
        varying->fGsOut = v.fGsOut.c_str();
        v.fVisibility |= kGeometry_GrShaderFlag;
    }
    if (varying->fsVarying()) {
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

void GrGLSLVaryingHandler::setNoPerspective() {
    const GrGLSLCaps& caps = *fProgramBuilder->glslCaps();
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
            fVertexOutputs.push_back().set(v.fType, GrShaderVar::kVaryingOut_TypeModifier, v.fVsOut,
                                           v.fPrecision, nullptr, modifier);
            if (v.fVisibility & kGeometry_GrShaderFlag) {
                fGeomInputs.push_back().set(v.fType, GrShaderVar::kVaryingIn_TypeModifier, v.fVsOut,
                                            GrShaderVar::kUnsizedArray, v.fPrecision, nullptr,
                                            modifier);
            }
        }
        if (v.fVisibility & kFragment_GrShaderFlag) {
            const char* fsIn = v.fVsOut.c_str();
            if (v.fVisibility & kGeometry_GrShaderFlag) {
                fGeomOutputs.push_back().set(v.fType, GrGLSLShaderVar::kVaryingOut_TypeModifier,
                                             v.fGsOut, v.fPrecision, nullptr, modifier);
                fsIn = v.fGsOut.c_str();
            }
            fFragInputs.push_back().set(v.fType, GrShaderVar::kVaryingIn_TypeModifier, fsIn,
                                        v.fPrecision, nullptr, modifier);
        }
    }
    this->onFinalize();
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
