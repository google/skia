/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrShaderVar_DEFINED
#define GrShaderVar_DEFINED

#include "include/core/SkString.h"
#include "include/private/GrTypesPriv.h"

class GrShaderCaps;

#define USE_UNIFORM_FLOAT_ARRAYS true

/**
 * Represents a variable in a shader
 */
class GrShaderVar {
public:
    enum TypeModifier {
        kNone_TypeModifier,
        kOut_TypeModifier,
        kIn_TypeModifier,
        kInOut_TypeModifier,
        kUniform_TypeModifier,
    };

    /**
     * Values for array count that have special meaning. We allow 1-sized arrays.git
     */
    enum {
        kNonArray     =  0, // not an array
        kUnsizedArray = -1, // an unsized array (declared with [])
    };

    /**
     * Defaults to a non-arry half with no type modifier or layout qualifier.
     */
    GrShaderVar()
        : fType(kHalf_GrSLType)
        , fTypeModifier(kNone_TypeModifier)
        , fCount(kNonArray)
        , fUseUniformFloatArrays(USE_UNIFORM_FLOAT_ARRAYS) {
    }

    GrShaderVar(const SkString& name, GrSLType type, int arrayCount = kNonArray)
        : fType(type)
        , fTypeModifier(kNone_TypeModifier)
        , fCount(arrayCount)
        , fUseUniformFloatArrays(USE_UNIFORM_FLOAT_ARRAYS)
        , fName(name) {
        SkASSERT(kVoid_GrSLType != type);
        fUseUniformFloatArrays = USE_UNIFORM_FLOAT_ARRAYS;
    }

    GrShaderVar(const char* name, GrSLType type, int arrayCount = kNonArray)
        : fType(type)
        , fTypeModifier(kNone_TypeModifier)
        , fCount(arrayCount)
        , fUseUniformFloatArrays(USE_UNIFORM_FLOAT_ARRAYS)
        , fName(name) {
        SkASSERT(kVoid_GrSLType != type);
        fUseUniformFloatArrays = USE_UNIFORM_FLOAT_ARRAYS;
    }

    GrShaderVar(const char* name, GrSLType type, TypeModifier typeModifier)
        : fType(type)
        , fTypeModifier(typeModifier)
        , fCount(kNonArray)
        , fUseUniformFloatArrays(USE_UNIFORM_FLOAT_ARRAYS)
        , fName(name) {
        SkASSERT(kVoid_GrSLType != type);
    }

    GrShaderVar(const char* name, GrSLType type, TypeModifier typeModifier, int arrayCount)
        : fType(type)
        , fTypeModifier(typeModifier)
        , fCount(arrayCount)
        , fUseUniformFloatArrays(USE_UNIFORM_FLOAT_ARRAYS)
        , fName(name) {
        SkASSERT(kVoid_GrSLType != type);
    }

    GrShaderVar(const GrShaderVar& that)
        : fType(that.fType)
        , fTypeModifier(that.fTypeModifier)
        , fCount(that.fCount)
        , fUseUniformFloatArrays(USE_UNIFORM_FLOAT_ARRAYS)
        , fName(that.fName)
        , fLayoutQualifier(that.fLayoutQualifier)
        , fExtraModifiers(that.fExtraModifiers) {
        SkASSERT(kVoid_GrSLType != that.getType());
    }

    /**
     * Sets as a non-array.
     */
    void set(GrSLType type,
             const SkString& name,
             TypeModifier typeModifier = kNone_TypeModifier,
             const char* layoutQualifier = nullptr,
             const char* extraModifiers = nullptr,
             bool useUniformFloatArrays = USE_UNIFORM_FLOAT_ARRAYS) {
        SkASSERT(kVoid_GrSLType != type);
        fType = type;
        fTypeModifier = typeModifier;
        fName = name;
        fCount = kNonArray;
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
             const char* name,
             TypeModifier typeModifier = kNone_TypeModifier,
             const char* layoutQualifier = nullptr,
             const char* extraModifiers = nullptr,
             bool useUniformFloatArrays = USE_UNIFORM_FLOAT_ARRAYS) {
        SkASSERT(kVoid_GrSLType != type);
        fType = type;
        fTypeModifier = typeModifier;
        fName = name;
        fCount = kNonArray;
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
             const SkString& name,
             int count,
             TypeModifier typeModifier,
             const char* layoutQualifier = nullptr,
             const char* extraModifiers = nullptr,
             bool useUniformFloatArrays = USE_UNIFORM_FLOAT_ARRAYS) {
        SkASSERT(kVoid_GrSLType != type);
        fType = type;
        fTypeModifier = typeModifier;
        fName = name;
        fCount = count;
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
             const char* name,
             int count,
             TypeModifier typeModifier,
             const char* layoutQualifier = nullptr,
             const char* extraModifiers = nullptr,
             bool useUniformFloatArrays = USE_UNIFORM_FLOAT_ARRAYS) {
        SkASSERT(kVoid_GrSLType != type);
        fType = type;
        fTypeModifier = typeModifier;
        fName = name;
        fCount = count;
        fLayoutQualifier = layoutQualifier;
        if (extraModifiers) {
            fExtraModifiers.printf("%s ", extraModifiers);
        }
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
     * Appends to the layout qualifier
     */
    void addLayoutQualifier(const char* layoutQualifier) {
        if (!layoutQualifier || !strlen(layoutQualifier)) {
            return;
        }
        if (fLayoutQualifier.isEmpty()) {
            fLayoutQualifier = layoutQualifier;
        } else {
            fLayoutQualifier.appendf(", %s", layoutQualifier);
        }
    }

    void setIOType(GrIOType);

    void addModifier(const char* modifier) {
        if (modifier) {
            fExtraModifiers.appendf("%s ", modifier);
        }
    }

    /**
     * Write a declaration of this variable to out.
     */
    void appendDecl(const GrShaderCaps*, SkString* out) const;

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
    GrSLType        fType;
    TypeModifier    fTypeModifier;
    int             fCount;
    /// Work around driver bugs on some hardware that don't correctly
    /// support uniform float []
    bool            fUseUniformFloatArrays;

    SkString        fName;
    SkString        fLayoutQualifier;
    SkString        fExtraModifiers;
};

#endif
