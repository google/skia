/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrShaderVar.h"

#include "src/core/SkSLTypeShared.h"

static const char* type_modifier_string(GrShaderVar::TypeModifier t) {
    switch (t) {
        case GrShaderVar::TypeModifier::None: return "";
        case GrShaderVar::TypeModifier::In: return "in";
        case GrShaderVar::TypeModifier::InOut: return "inout";
        case GrShaderVar::TypeModifier::Out: return "out";
        case GrShaderVar::TypeModifier::Uniform: return "uniform";
    }
    SK_ABORT("Unknown shader variable type modifier.");
}

void GrShaderVar::appendDecl(const GrShaderCaps* shaderCaps, SkString* out) const {
    if (!fLayoutQualifier.isEmpty()) {
        out->appendf("layout(%s) ", fLayoutQualifier.c_str());
    }
    if (!fExtraModifiers.isEmpty()) {
        out->appendf("%s ", fExtraModifiers.c_str());
    }
    if (this->getTypeModifier() != TypeModifier::None) {
        out->appendf("%s ", type_modifier_string(this->getTypeModifier()));
    }
    SkSLType effectiveType = this->getType();
    if (this->isArray()) {
        SkASSERT(this->getArrayCount() > 0);
        out->appendf("%s %s[%d]",
                     SkSLTypeString(effectiveType),
                     this->getName().c_str(),
                     this->getArrayCount());
    } else {
        out->appendf("%s %s", SkSLTypeString(effectiveType), this->getName().c_str());
    }
}
