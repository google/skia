/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrShaderVar.h"
#include "glsl/GrGLSLCaps.h"

void GrShaderVar::appendDecl(const GrGLSLCaps* glslCaps, SkString* out) const {
    SkASSERT(kDefault_GrSLPrecision == fPrecision || GrSLTypeAcceptsPrecision(fType));
    if (!fLayoutQualifier.isEmpty()) {
        out->appendf("layout(%s) ", fLayoutQualifier.c_str());
    }
    out->append(fExtraModifiers);
    if (this->getTypeModifier() != kNone_TypeModifier) {
        out->append(TypeModifierString(this->getTypeModifier()));
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
