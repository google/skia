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

struct GrShaderCaps;

/**
 * Represents a variable in a shader
 */
class GrShaderVar {
public:
    enum class TypeModifier {
        None,
        Out,
        In,
        InOut,
        Uniform,
    };

    /** Values for array count that have special meaning. We allow 1-sized arrays. */
    enum {
        kNonArray     =  0, // not an array
    };

    /** Defaults to a void with no type modifier or layout qualifier. */
    GrShaderVar()
            : fType(kVoid_GrSLType)
            , fTypeModifier(TypeModifier::None)
            , fCount(kNonArray) {}

    GrShaderVar(SkString name, GrSLType type, int arrayCount = kNonArray)
            : fType(type)
            , fTypeModifier(TypeModifier::None)
            , fCount(arrayCount)
            , fName(std::move(name)) {}
    GrShaderVar(const char* name, GrSLType type, int arrayCount = kNonArray)
        : GrShaderVar(SkString(name), type, arrayCount) {}

    GrShaderVar(SkString name, GrSLType type, TypeModifier typeModifier)
            : fType(type)
            , fTypeModifier(typeModifier)
            , fCount(kNonArray)
            , fName(std::move(name)) {}
    GrShaderVar(const char* name, GrSLType type, TypeModifier typeModifier)
        : GrShaderVar(SkString(name), type, typeModifier) {}

    GrShaderVar(SkString name, GrSLType type, TypeModifier typeModifier, int arrayCount)
            : fType(type)
            , fTypeModifier(typeModifier)
            , fCount(arrayCount)
            , fName(std::move(name)) {}

    GrShaderVar(SkString name, GrSLType type, TypeModifier typeModifier, int arrayCount,
                SkString layoutQualifier, SkString extraModifier)
            : fType(type)
            , fTypeModifier(typeModifier)
            , fCount(arrayCount)
            , fName(std::move(name))
            , fLayoutQualifier(std::move(layoutQualifier))
            , fExtraModifiers(std::move(extraModifier)) {}

    GrShaderVar(const GrShaderVar&) = default;
    GrShaderVar& operator=(const GrShaderVar&) = default;
    GrShaderVar(GrShaderVar&&) = default;
    GrShaderVar& operator=(GrShaderVar&&) = default;

    /** Sets as a non-array. */
    void set(GrSLType type,
             const char* name) {
        SkASSERT(kVoid_GrSLType != type);
        fType = type;
        fName = name;
    }

    /** Is the var an array. */
    bool isArray() const { return kNonArray != fCount; }

    /** Get the array length. */
    int getArrayCount() const { return fCount; }

    /** Get the name. */
    const SkString& getName() const { return fName; }

    /** Shortcut for this->getName().c_str(); */
    const char* c_str() const { return this->getName().c_str(); }

    /** Get the type. */
    GrSLType getType() const { return fType; }

    TypeModifier getTypeModifier() const { return fTypeModifier; }
    void setTypeModifier(TypeModifier type) { fTypeModifier = type; }

    /** Appends to the layout qualifier. */
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

    /** Appends to the modifiers. */
    void addModifier(const char* modifier) {
        if (!modifier || !strlen(modifier)) {
            return;
        }
        if (fExtraModifiers.isEmpty()) {
            fExtraModifiers = modifier;
        } else {
            fExtraModifiers.appendf(" %s", modifier);
        }
    }

    /** Write a declaration of this variable to out. */
    void appendDecl(const GrShaderCaps*, SkString* out) const;

private:
    GrSLType        fType;
    TypeModifier    fTypeModifier;
    int             fCount;

    SkString        fName;
    SkString        fLayoutQualifier;
    SkString        fExtraModifiers;
};

#endif
