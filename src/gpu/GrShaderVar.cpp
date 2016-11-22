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
    SkFAIL("Unknown shader variable type modifier.");
    return "";
}

static const char* image_storage_format_string(GrShaderVar::ImageStorageFormat f) {
    switch (f) {
        case GrShaderVar::ImageStorageFormat::kNone: return "";
        case GrShaderVar::ImageStorageFormat::kRGBA8: return "rgba8";
        case GrShaderVar::ImageStorageFormat::kRGBA8i: return "rgba8i";
        case GrShaderVar::ImageStorageFormat::kRGBA16f: return "rgba16f";
        case GrShaderVar::ImageStorageFormat::kRGBA32f: return "rgba32f";
    }
    SkFAIL("Unknown image storage format");
    return "";
}

void GrShaderVar::appendDecl(const GrShaderCaps* glslCaps, SkString* out) const {
    SkASSERT(kDefault_GrSLPrecision == fPrecision || GrSLTypeAcceptsPrecision(fType));
    SkString layout = fLayoutQualifier;
    if (ImageStorageFormat::kNone != fImageStorageFormat) {
        if (layout.isEmpty()) {
            layout = image_storage_format_string(fImageStorageFormat);
        } else {
            layout.appendf(", %s", image_storage_format_string(fImageStorageFormat));
        }
    }
    if (!layout.isEmpty()) {
        out->appendf("layout(%s) ", layout.c_str());
    }
    out->append(fExtraModifiers);
    if (this->getTypeModifier() != kNone_TypeModifier) {
        out->append(type_modifier_string(this->getTypeModifier()));
        out->append(" ");
    }
    GrSLType effectiveType = this->getType();
    if (glslCaps->usesPrecisionModifiers() && GrSLTypeAcceptsPrecision(effectiveType)) {
        // Desktop GLSL has added precision qualifiers but they don't do anything.
        out->appendf("%s ", GrGLSLPrecisionString(fPrecision));
    }
    if (this->isArray()) {
        if (this->isUnsizedArray()) {
            out->appendf("%s %s[]",
                         GrGLSLTypeString(effectiveType),
                         this->getName().c_str());
        } else {
            SkASSERT(this->getArrayCount() > 0);
            out->appendf("%s %s[%d]",
                         GrGLSLTypeString(effectiveType),
                         this->getName().c_str(),
                         this->getArrayCount());
        }
    } else {
        out->appendf("%s %s",
                     GrGLSLTypeString(effectiveType),
                     this->getName().c_str());
    }
}
