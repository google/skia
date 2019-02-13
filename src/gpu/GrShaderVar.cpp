/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrShaderVar.h"
#include "GrShaderCaps.h"

static const char* type_modifier_string(GrShaderVar::TypeModifier t) {
    switch (t) {
        case GrShaderVar::kNone_TypeModifier: return "";
        case GrShaderVar::kIn_TypeModifier: return "in";
        case GrShaderVar::kInOut_TypeModifier: return "inout";
        case GrShaderVar::kOut_TypeModifier: return "out";
        case GrShaderVar::kUniform_TypeModifier: return "uniform";
    }
    SK_ABORT("Unknown shader variable type modifier.");
    return "";
}

void GrShaderVar::setIOType(GrIOType ioType) {
    switch (ioType) {
        case kRW_GrIOType:
            return;
        case kRead_GrIOType:
            this->addModifier("readonly");
            return;
        case kWrite_GrIOType:
            this->addModifier("writeonly");
            return;
    }
    SK_ABORT("Unknown io type.");
}

// Converts a GrSLPrecision to its corresponding GLSL precision qualifier. TODO: Remove this as we
// shouldn't need it with SkSL.
static inline const char* glsl_precision_string(GrSLPrecision p) {
    switch (p) {
        case kLow_GrSLPrecision:
            return "lowp";
        case kMedium_GrSLPrecision:
            return "mediump";
        case kHigh_GrSLPrecision:
            return "highp";
        case kDefault_GrSLPrecision:
            return "";
    }
    SK_ABORT("Unexpected precision type.");
    return "";
}

void GrShaderVar::appendDecl(const GrShaderCaps* shaderCaps, SkString* out) const {
    SkASSERT(kDefault_GrSLPrecision == fPrecision || GrSLTypeTemporarilyAcceptsPrecision(fType));
    SkString layout = fLayoutQualifier;
    if (!fLayoutQualifier.isEmpty()) {
        out->appendf("layout(%s) ", fLayoutQualifier.c_str());
    }
    out->append(fExtraModifiers);
    if (this->getTypeModifier() != kNone_TypeModifier) {
        out->append(type_modifier_string(this->getTypeModifier()));
        out->append(" ");
    }
    GrSLType effectiveType = this->getType();
    if (shaderCaps->usesPrecisionModifiers() && GrSLTypeAcceptsPrecision(effectiveType)) {
        // Desktop GLSL has added precision qualifiers but they don't do anything.
        out->appendf("%s ", glsl_precision_string(fPrecision));
    }
    if (this->isArray()) {
        if (this->isUnsizedArray()) {
            out->appendf("%s %s[]", GrGLSLTypeString(effectiveType), this->getName().c_str());
        } else {
            SkASSERT(this->getArrayCount() > 0);
            out->appendf("%s %s[%d]",
                         GrGLSLTypeString(effectiveType),
                         this->getName().c_str(),
                         this->getArrayCount());
        }
    } else {
        out->appendf("%s %s", GrGLSLTypeString(effectiveType), this->getName().c_str());
    }
}
