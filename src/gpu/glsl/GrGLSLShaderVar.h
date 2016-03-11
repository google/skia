/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLShaderVar_DEFINED
#define GrGLSLShaderVar_DEFINED

#include "GrShaderVar.h"
#include "../glsl/GrGLSL.h"
#include "../glsl/GrGLSLCaps.h"

#define USE_UNIFORM_FLOAT_ARRAYS true

/**
 * Represents a variable in a shader
 */
class GrGLSLShaderVar : public GrShaderVar {
public:
    /**
     * Defaults to a float with no precision specifier
     */
    GrGLSLShaderVar()
        : GrShaderVar()
        , fUseUniformFloatArrays(USE_UNIFORM_FLOAT_ARRAYS) {
    }

    GrGLSLShaderVar(const char* name, GrSLType type, int arrayCount = kNonArray,
                    GrSLPrecision precision = kDefault_GrSLPrecision)
        : GrShaderVar(name, type, arrayCount, precision)
        , fUseUniformFloatArrays(USE_UNIFORM_FLOAT_ARRAYS) {
        SkASSERT(kVoid_GrSLType != type);
        fUseUniformFloatArrays = USE_UNIFORM_FLOAT_ARRAYS;
    }

    GrGLSLShaderVar(const char* name, GrSLType type, TypeModifier typeModifier,
                    int arrayCount = kNonArray, GrSLPrecision precision = kDefault_GrSLPrecision)
        : GrShaderVar(name, type, typeModifier, arrayCount, precision)
        , fUseUniformFloatArrays(USE_UNIFORM_FLOAT_ARRAYS) {
        SkASSERT(kVoid_GrSLType != type);
    }

    GrGLSLShaderVar(const GrShaderVar& var)
        : GrShaderVar(var)
        , fUseUniformFloatArrays(USE_UNIFORM_FLOAT_ARRAYS) {
        SkASSERT(kVoid_GrSLType != var.getType());
    }

    GrGLSLShaderVar(const GrGLSLShaderVar& var)
        : GrShaderVar(var.c_str(), var.getType(), var.getTypeModifier(),
                      var.getArrayCount(), var.getPrecision())
        , fUseUniformFloatArrays(var.fUseUniformFloatArrays)
        , fLayoutQualifier(var.fLayoutQualifier)
        , fExtraModifiers(var.fExtraModifiers) {
        SkASSERT(kVoid_GrSLType != var.getType());
    }

    /**
     * Values for array count that have special meaning. We allow 1-sized arrays.
     */
    enum {
        kNonArray     =  0, // not an array
        kUnsizedArray = -1, // an unsized array (declared with [])
    };

    /**
     * Sets as a non-array.
     */
    void set(GrSLType type,
             TypeModifier typeModifier,
             const SkString& name,
             GrSLPrecision precision = kDefault_GrSLPrecision,
             const char* layoutQualifier = nullptr,
             const char* extraModifiers = nullptr,
             bool useUniformFloatArrays = USE_UNIFORM_FLOAT_ARRAYS) {
        SkASSERT(kVoid_GrSLType != type);
        SkASSERT(kDefault_GrSLPrecision == precision || GrSLTypeAcceptsPrecision(type));
        INHERITED::set(type, name, typeModifier, precision);
        fLayoutQualifier = layoutQualifier;
        if (extraModifiers) {
            fExtraModifiers.printf("%s ", extraModifiers);
        }
        fUseUniformFloatArrays = useUniformFloatArrays;
    }

    /**
     * Sets as a non-array.
     */
    void set(GrSLType type,
             TypeModifier typeModifier,
             const char* name,
             GrSLPrecision precision = kDefault_GrSLPrecision,
             const char* layoutQualifier = nullptr,
             const char* extraModifiers = nullptr,
             bool useUniformFloatArrays = USE_UNIFORM_FLOAT_ARRAYS) {
        SkASSERT(kVoid_GrSLType != type);
        SkASSERT(kDefault_GrSLPrecision == precision || GrSLTypeAcceptsPrecision(type));
        INHERITED::set(type, name, typeModifier, precision);
        fLayoutQualifier = layoutQualifier;
        if (extraModifiers) {
            fExtraModifiers.printf("%s ", extraModifiers);
        }
        fUseUniformFloatArrays = useUniformFloatArrays;
    }

    /**
     * Set all var options
     */
    void set(GrSLType type,
             TypeModifier typeModifier,
             const SkString& name,
             int count,
             GrSLPrecision precision = kDefault_GrSLPrecision,
             const char* layoutQualifier = nullptr,
             const char* extraModifiers = nullptr,
             bool useUniformFloatArrays = USE_UNIFORM_FLOAT_ARRAYS) {
        SkASSERT(kVoid_GrSLType != type);
        SkASSERT(kDefault_GrSLPrecision == precision || GrSLTypeAcceptsPrecision(type));
        INHERITED::set(type, name, typeModifier, precision, count);
        fLayoutQualifier = layoutQualifier;
        if (extraModifiers) {
            fExtraModifiers.printf("%s ", extraModifiers);
        }
        fUseUniformFloatArrays = useUniformFloatArrays;
    }

    /**
     * Set all var options
     */
    void set(GrSLType type,
             TypeModifier typeModifier,
             const char* name,
             int count,
             GrSLPrecision precision = kDefault_GrSLPrecision,
             const char* layoutQualifier = nullptr,
             const char* extraModifiers = nullptr,
             bool useUniformFloatArrays = USE_UNIFORM_FLOAT_ARRAYS) {
        SkASSERT(kVoid_GrSLType != type);
        SkASSERT(kDefault_GrSLPrecision == precision || GrSLTypeAcceptsPrecision(type));
        INHERITED::set(type, name, typeModifier, precision, count);
        fLayoutQualifier = layoutQualifier;
        if (extraModifiers) {
            fExtraModifiers.printf("%s ", extraModifiers);
        }
        fUseUniformFloatArrays = useUniformFloatArrays;
    }

    /**
     * Set the layout qualifier
     */
    void setLayoutQualifier(const char* layoutQualifier) {
        fLayoutQualifier = layoutQualifier;
    }

    void addModifier(const char* modifier) {
        if (modifier) {
            fExtraModifiers.appendf("%s ", modifier);
        }
    }

    /**
     * Write a declaration of this variable to out.
     */
    void appendDecl(const GrGLSLCaps* glslCaps, SkString* out) const {
        SkASSERT(kDefault_GrSLPrecision == fPrecision || GrSLTypeAcceptsPrecision(fType));
        if (!fLayoutQualifier.isEmpty()) {
            out->appendf("layout(%s) ", fLayoutQualifier.c_str());
        }
        out->append(fExtraModifiers);
        if (this->getTypeModifier() != kNone_TypeModifier) {
            out->append(TypeModifierString(glslCaps, this->getTypeModifier()));
            out->append(" ");
        }
        GrSLType effectiveType = this->getType();
        if (GrSLTypeAcceptsPrecision(effectiveType)) {
            out->append(PrecisionString(glslCaps, fPrecision));
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

    void appendArrayAccess(int index, SkString* out) const {
        out->appendf("%s[%d]%s",
                     this->getName().c_str(),
                     index,
                     fUseUniformFloatArrays ? "" : ".x");
    }

    void appendArrayAccess(const char* indexName, SkString* out) const {
        out->appendf("%s[%s]%s",
                     this->getName().c_str(),
                     indexName,
                     fUseUniformFloatArrays ? "" : ".x");
    }

    static const char* PrecisionString(const GrGLSLCaps* glslCaps, GrSLPrecision p) {
        // Desktop GLSL has added precision qualifiers but they don't do anything.
        if (glslCaps->usesPrecisionModifiers()) {
            switch (p) {
                case kLow_GrSLPrecision:
                    return "lowp ";
                case kMedium_GrSLPrecision:
                    return "mediump ";
                case kHigh_GrSLPrecision:
                    return "highp ";
                default:
                    SkFAIL("Unexpected precision type.");
            }
        }
        return "";
    }

private:
    static const char* TypeModifierString(const GrGLSLCaps* glslCaps, TypeModifier t) {
        GrGLSLGeneration gen = glslCaps->generation();
        switch (t) {
            case kNone_TypeModifier:
                return "";
            case kIn_TypeModifier:
                return "in";
            case kInOut_TypeModifier:
                return "inout";
            case kOut_TypeModifier:
                return "out";
            case kUniform_TypeModifier:
                return "uniform";
            case kAttribute_TypeModifier:
                return k110_GrGLSLGeneration == gen ? "attribute" : "in";
            case kVaryingIn_TypeModifier:
                return k110_GrGLSLGeneration == gen ? "varying" : "in";
            case kVaryingOut_TypeModifier:
                return k110_GrGLSLGeneration == gen ? "varying" : "out";
            default:
                SkFAIL("Unknown shader variable type modifier.");
                return ""; // suppress warning
        }
    }

    /// Work around driver bugs on some hardware that don't correctly
    /// support uniform float []
    bool        fUseUniformFloatArrays;

    SkString    fLayoutQualifier;
    SkString    fExtraModifiers;

    typedef GrShaderVar INHERITED;
};

#endif
