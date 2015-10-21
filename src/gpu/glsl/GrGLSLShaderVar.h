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
     * See GL_ARB_fragment_coord_conventions.
     */
    enum Origin {
        kDefault_Origin,        // when set to kDefault the origin field is ignored.
        kUpperLeft_Origin,      // only used to declare vec4 in gl_FragCoord.
    };

    /**
     * Defaults to a float with no precision specifier
     */
    GrGLSLShaderVar()
        : GrShaderVar()
        , fOrigin(kDefault_Origin)
        , fUseUniformFloatArrays(USE_UNIFORM_FLOAT_ARRAYS) {
    }

    GrGLSLShaderVar(const char* name, GrSLType type, int arrayCount = kNonArray,
                    GrSLPrecision precision = kDefault_GrSLPrecision)
        : GrShaderVar(name, type, arrayCount, precision)
        , fOrigin(kDefault_Origin)
        , fUseUniformFloatArrays(USE_UNIFORM_FLOAT_ARRAYS) {
        SkASSERT(kVoid_GrSLType != type);
        fOrigin = kDefault_Origin;
        fUseUniformFloatArrays = USE_UNIFORM_FLOAT_ARRAYS;
    }

    GrGLSLShaderVar(const char* name, GrSLType type, TypeModifier typeModifier,
                    int arrayCount = kNonArray, GrSLPrecision precision = kDefault_GrSLPrecision)
        : GrShaderVar(name, type, typeModifier, arrayCount, precision)
        , fOrigin(kDefault_Origin)
        , fUseUniformFloatArrays(USE_UNIFORM_FLOAT_ARRAYS) {
        SkASSERT(kVoid_GrSLType != type);
    }

    GrGLSLShaderVar(const GrShaderVar& var)
        : GrShaderVar(var)
        , fOrigin(kDefault_Origin)
        , fUseUniformFloatArrays(USE_UNIFORM_FLOAT_ARRAYS) {
        SkASSERT(kVoid_GrSLType != var.getType());
    }

    GrGLSLShaderVar(const GrGLSLShaderVar& var)
        : GrShaderVar(var.c_str(), var.getType(), var.getTypeModifier(),
                      var.getArrayCount(), var.getPrecision())
        , fOrigin(var.fOrigin)
        , fUseUniformFloatArrays(var.fUseUniformFloatArrays) {
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
             Origin origin = kDefault_Origin,
             bool useUniformFloatArrays = USE_UNIFORM_FLOAT_ARRAYS) {
        SkASSERT(kVoid_GrSLType != type);
        SkASSERT(kDefault_GrSLPrecision == precision || GrSLTypeIsFloatType(type));
        INHERITED::set(type, name, typeModifier, precision);
        fOrigin = origin;
        fUseUniformFloatArrays = useUniformFloatArrays;
    }

    /**
     * Sets as a non-array.
     */
    void set(GrSLType type,
             TypeModifier typeModifier,
             const char* name,
             GrSLPrecision precision = kDefault_GrSLPrecision,
             Origin origin = kDefault_Origin,
             bool useUniformFloatArrays = USE_UNIFORM_FLOAT_ARRAYS) {
        SkASSERT(kVoid_GrSLType != type);
        SkASSERT(kDefault_GrSLPrecision == precision || GrSLTypeIsFloatType(type));
        INHERITED::set(type, name, typeModifier, precision);
        fOrigin = origin;
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
             Origin origin = kDefault_Origin,
             bool useUniformFloatArrays = USE_UNIFORM_FLOAT_ARRAYS) {
        SkASSERT(kVoid_GrSLType != type);
        SkASSERT(kDefault_GrSLPrecision == precision || GrSLTypeIsFloatType(type));
        INHERITED::set(type, name, typeModifier, precision, count);
        fOrigin = origin;
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
             Origin origin = kDefault_Origin,
             bool useUniformFloatArrays = USE_UNIFORM_FLOAT_ARRAYS) {
        SkASSERT(kVoid_GrSLType != type);
        SkASSERT(kDefault_GrSLPrecision == precision || GrSLTypeIsFloatType(type));
        INHERITED::set(type, name, typeModifier, precision, count);
        fOrigin = origin;
        fUseUniformFloatArrays = useUniformFloatArrays;
    }

    /**
     * Get the origin of the var
     */
    Origin getOrigin() const { return fOrigin; }

    /**
     * Set the origin of the var
     */
    void setOrigin(Origin origin) { fOrigin = origin; }

    /**
     * Write a declaration of this variable to out.
     */
    void appendDecl(const GrGLSLCaps* glslCaps, SkString* out) const {
        SkASSERT(kDefault_GrSLPrecision == fPrecision || GrSLTypeIsFloatType(fType));
        if (kUpperLeft_Origin == fOrigin) {
            // this is the only place where we specify a layout modifier. If we use other layout
            // modifiers in the future then they should be placed in a list.
            out->append("layout(origin_upper_left) ");
        }
        if (this->getTypeModifier() != kNone_TypeModifier) {
           out->append(TypeModifierString(glslCaps, this->getTypeModifier()));
           out->append(" ");
        }
        out->append(PrecisionString(glslCaps, fPrecision));
        GrSLType effectiveType = this->getType();
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

    Origin          fOrigin;
    /// Work around driver bugs on some hardware that don't correctly
    /// support uniform float []
    bool            fUseUniformFloatArrays;

    typedef GrShaderVar INHERITED;
};

#endif
