
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLShaderVar_DEFINED
#define GrGLShaderVar_DEFINED

#include "GrGLInterface.h"
#include "GrStringBuilder.h"

#define USE_UNIFORM_FLOAT_ARRAYS true

/**
 * Represents a variable in a shader
 */
class GrGLShaderVar {
public:

    enum Type {
        kFloat_Type,
        kVec2f_Type,
        kVec3f_Type,
        kVec4f_Type,
        kMat33f_Type,
        kSampler2D_Type,
    };

    /**
     * Defaults to a float with no precision specifier
     */
    GrGLShaderVar() {
        fType = kFloat_Type;
        fCount = kNonArray;
        fEmitPrecision = false;
        fUseUniformFloatArrays = USE_UNIFORM_FLOAT_ARRAYS;
    }

    GrGLShaderVar(const GrGLShaderVar& var)
        : fType(var.fType)
        , fName(var.fName)
        , fCount(var.fCount)
        , fEmitPrecision(var.fEmitPrecision)
        , fUseUniformFloatArrays(var.fUseUniformFloatArrays) {}

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
    void set(Type type,
             const GrStringBuilder& name,
             bool emitPrecision = false,
             bool useUniformFloatArrays = USE_UNIFORM_FLOAT_ARRAYS) {
        fType = type;
        fName = name;
        fCount = kNonArray;
        fEmitPrecision = emitPrecision;
        fUseUniformFloatArrays = useUniformFloatArrays;
    }

    /**
     * Sets as a non-array.
     */
    void set(Type type,
             const char* name,
             bool specifyPrecision = false,
             bool useUniformFloatArrays = USE_UNIFORM_FLOAT_ARRAYS) {
        fType = type;
        fName = name;
        fCount = kNonArray;
        fEmitPrecision = specifyPrecision;
        fUseUniformFloatArrays = useUniformFloatArrays;
    }

    /**
     * Set all var options
     */
    void set(Type type,
             const GrStringBuilder& name,
             int count,
             bool specifyPrecision = false,
             bool useUniformFloatArrays = USE_UNIFORM_FLOAT_ARRAYS) {
        fType = type;
        fName = name;
        fCount = count;
        fEmitPrecision = specifyPrecision;
        fUseUniformFloatArrays = useUniformFloatArrays;
    }

    /**
     * Set all var options
     */
    void set(Type type,
             const char* name,
             int count,
             bool specifyPrecision = false,
             bool useUniformFloatArrays = USE_UNIFORM_FLOAT_ARRAYS) {
        fType = type;
        fName = name;
        fCount = count;
        fEmitPrecision = specifyPrecision;
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
    GrStringBuilder* accessName() { return &fName; }
    /**
     * Set the var name
     */
    void setName(const GrStringBuilder& n) { fName = n; }
    void setName(const char* n) { fName = n; }
    /**
     * Get the var name.
     */
    const GrStringBuilder& getName() const { return fName; }

    /**
     * Get the type of the var
     */
    Type getType() const { return fType; }
    /**
     * Set the type of the var
     */
    void setType(Type type) { fType = type; }

    /**
     * Must the variable declaration emit a precision specifier
     */
    bool emitsPrecision() const { return fEmitPrecision; }
    /**
     * Specify whether the declaration should specify precision
     */
    void setEmitPrecision(bool p) { fEmitPrecision = p; }

    /**
     * Write a declaration of this variable to out.
     */
    void appendDecl(const GrGLInterface* gl, GrStringBuilder* out) const {
        if (this->emitsPrecision()) {
            out->append(PrecisionString(gl));
            out->append(" ");
        }
        Type effectiveType = this->getType();
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

    static const char* TypeString(Type t) {
        switch (t) {
            case kFloat_Type:
                return "float";
            case kVec2f_Type:
                return "vec2";
            case kVec3f_Type:
                return "vec3";
            case kVec4f_Type:
                return "vec4";
            case kMat33f_Type:
                return "mat3";
            case kSampler2D_Type:
                return "sampler2D";
            default:
                GrCrash("Unknown shader var type.");
                return ""; // suppress warning
        }
    }

    void appendArrayAccess(int index, GrStringBuilder* out) {
        out->appendf("%s[%d]%s",
                     this->getName().c_str(),
                     index,
                     fUseUniformFloatArrays ? "" : ".x");
    }

    void appendArrayAccess(const char* indexName, GrStringBuilder* out) {
        out->appendf("%s[%s]%s",
                     this->getName().c_str(),
                     indexName,
                     fUseUniformFloatArrays ? "" : ".x");
    }

private:
    static const char* PrecisionString(const GrGLInterface* gl) {
        return gl->supportsDesktop() ? "" : "mediump";
    }

    Type            fType;
    GrStringBuilder fName;
    int             fCount;
    bool            fEmitPrecision;
    /// Work around driver bugs on some hardware that don't correctly
    /// support uniform float []
    bool            fUseUniformFloatArrays;
};

#endif
