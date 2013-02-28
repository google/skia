/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLShaderVar_DEFINED
#define GrGLShaderVar_DEFINED

#include "GrGLContext.h"
#include "GrGLSL.h"
#include "SkString.h"

#define USE_UNIFORM_FLOAT_ARRAYS true

/**
 * Represents a variable in a shader
 */
class GrGLShaderVar {
public:

    /**
     * Early versions of GLSL have Varying and Attribute; those are later
     * deprecated, but we still need to know whether a Varying variable
     * should be treated as In or Out.
     */
    enum TypeModifier {
        kNone_TypeModifier,
        kOut_TypeModifier,
        kIn_TypeModifier,
        kUniform_TypeModifier,
        kAttribute_TypeModifier
    };

    enum Precision {
        kLow_Precision,         // lowp
        kMedium_Precision,      // mediump
        kHigh_Precision,        // highp
        kDefault_Precision,     // Default for the current context. We make
                                // fragment shaders default to mediump on ES2
                                // because highp support is not guaranteed (and
                                // we haven't been motivated to test for it).
                                // Otherwise, highp.
    };

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
    GrGLShaderVar() {
        fType = kFloat_GrSLType;
        fTypeModifier = kNone_TypeModifier;
        fCount = kNonArray;
        fPrecision = kDefault_Precision;
        fOrigin = kDefault_Origin;
        fUseUniformFloatArrays = USE_UNIFORM_FLOAT_ARRAYS;
    }

    GrGLShaderVar(const char* name, GrSLType type, int arrayCount = kNonArray) {
        GrAssert(kVoid_GrSLType != type);
        fType = type;
        fTypeModifier = kNone_TypeModifier;
        fCount = arrayCount;
        fPrecision = kDefault_Precision;
        fOrigin = kDefault_Origin;
        fUseUniformFloatArrays = USE_UNIFORM_FLOAT_ARRAYS;
        fName = name;
    }

    GrGLShaderVar(const GrGLShaderVar& var)
        : fType(var.fType)
        , fTypeModifier(var.fTypeModifier)
        , fName(var.fName)
        , fCount(var.fCount)
        , fPrecision(var.fPrecision)
        , fOrigin(var.fOrigin)
        , fUseUniformFloatArrays(var.fUseUniformFloatArrays) {
        GrAssert(kVoid_GrSLType != var.fType);
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
             Precision precision = kDefault_Precision,
             Origin origin = kDefault_Origin,
             bool useUniformFloatArrays = USE_UNIFORM_FLOAT_ARRAYS) {
        GrAssert(kVoid_GrSLType != type);
        fType = type;
        fTypeModifier = typeModifier;
        fName = name;
        fCount = kNonArray;
        fPrecision = precision;
        fOrigin = origin;
        fUseUniformFloatArrays = useUniformFloatArrays;
    }

    /**
     * Sets as a non-array.
     */
    void set(GrSLType type,
             TypeModifier typeModifier,
             const char* name,
             Precision precision = kDefault_Precision,
             Origin origin = kDefault_Origin,
             bool useUniformFloatArrays = USE_UNIFORM_FLOAT_ARRAYS) {
        GrAssert(kVoid_GrSLType != type);
        fType = type;
        fTypeModifier = typeModifier;
        fName = name;
        fCount = kNonArray;
        fPrecision = precision;
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
             Precision precision = kDefault_Precision,
             Origin origin = kDefault_Origin,
             bool useUniformFloatArrays = USE_UNIFORM_FLOAT_ARRAYS) {
        GrAssert(kVoid_GrSLType != type);
        fType = type;
        fTypeModifier = typeModifier;
        fName = name;
        fCount = count;
        fPrecision = precision;
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
             Precision precision = kDefault_Precision,
             Origin origin = kDefault_Origin,
             bool useUniformFloatArrays = USE_UNIFORM_FLOAT_ARRAYS) {
        GrAssert(kVoid_GrSLType != type);
        fType = type;
        fTypeModifier = typeModifier;
        fName = name;
        fCount = count;
        fPrecision = precision;
        fOrigin = origin;
        fUseUniformFloatArrays = useUniformFloatArrays;
    }

    /**
     * Is the var an array.
     */
    bool isArray() const { return kNonArray != fCount; }
    /**
     * Is this an unsized array, (i.e. declared with []).
     */
    bool isUnsizedArray() const { return kUnsizedArray == fCount; }
    /**
     * Get the array length of the var.
     */
    int getArrayCount() const { return fCount; }
    /**
     * Set the array length of the var
     */
    void setArrayCount(int count) { fCount = count; }
    /**
     * Set to be a non-array.
     */
    void setNonArray() { fCount = kNonArray; }
    /**
     * Set to be an unsized array.
     */
    void setUnsizedArray() { fCount = kUnsizedArray; }

    /**
     * Access the var name as a writable string
     */
    SkString* accessName() { return &fName; }
    /**
     * Set the var name
     */
    void setName(const SkString& n) { fName = n; }
    void setName(const char* n) { fName = n; }

    /**
     * Get the var name.
     */
    const SkString& getName() const { return fName; }

    /**
     * Shortcut for this->getName().c_str();
     */
    const char* c_str() const { return this->getName().c_str(); }

    /**
     * Get the type of the var
     */
    GrSLType getType() const { return fType; }
    /**
     * Set the type of the var
     */
    void setType(GrSLType type) { fType = type; }

    TypeModifier getTypeModifier() const { return fTypeModifier; }
    void setTypeModifier(TypeModifier type) { fTypeModifier = type; }

    /**
     * Get the precision of the var
     */
    Precision getPrecision() const { return fPrecision; }

    /**
     * Set the precision of the var
     */
    void setPrecision(Precision p) { fPrecision = p; }

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
    void appendDecl(const GrGLContextInfo& ctxInfo, SkString* out) const {
        if (kUpperLeft_Origin == fOrigin) {
            // this is the only place where we specify a layout modifier. If we use other layout
            // modifiers in the future then they should be placed in a list.
            out->append("layout(origin_upper_left) ");
        }
        if (this->getTypeModifier() != kNone_TypeModifier) {
           out->append(TypeModifierString(this->getTypeModifier(),
                                          ctxInfo.glslGeneration()));
           out->append(" ");
        }
        out->append(PrecisionString(fPrecision, ctxInfo.binding()));
        GrSLType effectiveType = this->getType();
        if (this->isArray()) {
            if (this->isUnsizedArray()) {
                out->appendf("%s %s[]",
                             TypeString(effectiveType),
                             this->getName().c_str());
            } else {
                GrAssert(this->getArrayCount() > 0);
                out->appendf("%s %s[%d]",
                             TypeString(effectiveType),
                             this->getName().c_str(),
                             this->getArrayCount());
            }
        } else {
            out->appendf("%s %s",
                         TypeString(effectiveType),
                         this->getName().c_str());
        }
    }

    static const char* TypeString(GrSLType t) {
        switch (t) {
            case kVoid_GrSLType:
                return "void";
            case kFloat_GrSLType:
                return "float";
            case kVec2f_GrSLType:
                return "vec2";
            case kVec3f_GrSLType:
                return "vec3";
            case kVec4f_GrSLType:
                return "vec4";
            case kMat33f_GrSLType:
                return "mat3";
            case kMat44f_GrSLType:
                return "mat4";
            case kSampler2D_GrSLType:
                return "sampler2D";
            default:
                GrCrash("Unknown shader var type.");
                return ""; // suppress warning
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

private:
    static const char* TypeModifierString(TypeModifier t, GrGLSLGeneration gen) {
        switch (t) {
            case kNone_TypeModifier:
                return "";
            case kOut_TypeModifier:
                return k110_GrGLSLGeneration == gen ? "varying" : "out";
            case kIn_TypeModifier:
                return k110_GrGLSLGeneration == gen ? "varying" : "in";
            case kUniform_TypeModifier:
                return "uniform";
            case kAttribute_TypeModifier:
                return k110_GrGLSLGeneration == gen ? "attribute" : "in";
            default:
                GrCrash("Unknown shader variable type modifier.");
                return ""; // suppress warning
        }
    }

    static const char* PrecisionString(Precision p, GrGLBinding binding) {
        // Desktop GLSL has added precision qualifiers but they don't do anything.
        if (kES2_GrGLBinding == binding) {
            switch (p) {
                case kLow_Precision:
                    return "lowp ";
                case kMedium_Precision:
                    return "mediump ";
                case kHigh_Precision:
                    return "highp ";
                case kDefault_Precision:
                    return "";
                default:
                    GrCrash("Unexpected precision type.");
            }
        }
        return "";
    }

    GrSLType        fType;
    TypeModifier    fTypeModifier;
    SkString        fName;
    int             fCount;
    Precision       fPrecision;
    Origin          fOrigin;
    /// Work around driver bugs on some hardware that don't correctly
    /// support uniform float []
    bool            fUseUniformFloatArrays;
};

#endif
