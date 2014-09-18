/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrShaderVar_DEFINED
#define GrShaderVar_DEFINED

#include "GrTypesPriv.h"
#include "SkString.h"

class GrShaderVar {
public:
    /**
     * Early versions of GLSL have Varying and Attribute; those are later
     * deprecated, but we still need to know whether a Varying variable
     * should be treated as In or Out.
     *
     * TODO This really shouldn't live here, but until we have c++11, there is really no good way
     * to write extensible enums.  In reality, only none, out, in, inout, and uniform really
     * make sense on this base class
     */
    enum TypeModifier {
        kNone_TypeModifier,
        kOut_TypeModifier,
        kIn_TypeModifier,
        kInOut_TypeModifier,
        kUniform_TypeModifier,
        // GL Specific types below
        kAttribute_TypeModifier,
        kVaryingIn_TypeModifier,
        kVaryingOut_TypeModifier
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
     * Defaults to a float with no precision specifier
     */
    GrShaderVar()
        : fType(kFloat_GrSLType)
        , fTypeModifier(kNone_TypeModifier)
        , fCount(kNonArray)
        , fPrecision(kDefault_Precision) {
    }

    GrShaderVar(const SkString& name, GrSLType type, int arrayCount = kNonArray,
                Precision precision = kDefault_Precision)
        : fType(type)
        , fTypeModifier(kNone_TypeModifier)
        , fName(name)
        , fCount(arrayCount)
        , fPrecision(precision) {
        SkASSERT(kVoid_GrSLType != type);
    }

    GrShaderVar(const char* name, GrSLType type, int arrayCount = kNonArray,
                  Precision precision = kDefault_Precision)
        : fType(type)
        , fTypeModifier(kNone_TypeModifier)
        , fName(name)
        , fCount(arrayCount)
        , fPrecision(precision) {
        SkASSERT(kVoid_GrSLType != type);
    }

    GrShaderVar(const char* name, GrSLType type, TypeModifier typeModifier,
                  int arrayCount = kNonArray, Precision precision = kDefault_Precision)
        : fType(type)
        , fTypeModifier(typeModifier)
        , fName(name)
        , fCount(arrayCount)
        , fPrecision(precision) {
        SkASSERT(kVoid_GrSLType != type);
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
             Precision precision = kDefault_Precision) {
        SkASSERT(kVoid_GrSLType != type);
        fType = type;
        fTypeModifier = typeModifier;
        fName = name;
        fCount = kNonArray;
        fPrecision = precision;
    }

    /**
     * Sets as a non-array.
     */
    void set(GrSLType type,
             TypeModifier typeModifier,
             const char* name,
             Precision precision = kDefault_Precision) {
        SkASSERT(kVoid_GrSLType != type);
        fType = type;
        fTypeModifier = typeModifier;
        fName = name;
        fCount = kNonArray;
        fPrecision = precision;
    }

    /**
     * Set all var options
     */
    void set(GrSLType type,
             TypeModifier typeModifier,
             const SkString& name,
             int count,
             Precision precision = kDefault_Precision) {
        SkASSERT(kVoid_GrSLType != type);
        fType = type;
        fTypeModifier = typeModifier;
        fName = name;
        fCount = count;
        fPrecision = precision;
    }

    /**
     * Set all var options
     */
    void set(GrSLType type,
             TypeModifier typeModifier,
             const char* name,
             int count,
             Precision precision = kDefault_Precision) {
        SkASSERT(kVoid_GrSLType != type);
        fType = type;
        fTypeModifier = typeModifier;
        fName = name;
        fCount = count;
        fPrecision = precision;
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

protected:
    GrSLType        fType;
    TypeModifier    fTypeModifier;
    SkString        fName;
    int             fCount;
    Precision       fPrecision;
};

#endif
