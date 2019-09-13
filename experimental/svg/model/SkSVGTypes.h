/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGTypes_DEFINED
#define SkSVGTypes_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTDArray.h"

template <typename T>
class SkSVGPrimitiveTypeWrapper {
public:
    SkSVGPrimitiveTypeWrapper() = default;
    explicit constexpr SkSVGPrimitiveTypeWrapper(T v) : fValue(v) {}

    SkSVGPrimitiveTypeWrapper(const SkSVGPrimitiveTypeWrapper&)            = default;
    SkSVGPrimitiveTypeWrapper& operator=(const SkSVGPrimitiveTypeWrapper&) = default;
    SkSVGPrimitiveTypeWrapper& operator=(const T& v) { fValue = v; return *this; }

    bool operator==(const SkSVGPrimitiveTypeWrapper<T>& other) const {
        return fValue == other.fValue;
    }
    bool operator!=(const SkSVGPrimitiveTypeWrapper<T>& other) const {
        return !(*this == other);
    }

    const T& value() const { return fValue; }
    operator const T&() const { return fValue; }

private:
    T fValue;
};

using SkSVGColorType      = SkSVGPrimitiveTypeWrapper<SkColor >;
using SkSVGNumberType     = SkSVGPrimitiveTypeWrapper<SkScalar>;
using SkSVGStringType     = SkSVGPrimitiveTypeWrapper<SkString>;
using SkSVGViewBoxType    = SkSVGPrimitiveTypeWrapper<SkRect  >;
using SkSVGTransformType  = SkSVGPrimitiveTypeWrapper<SkMatrix>;
using SkSVGPointsType     = SkSVGPrimitiveTypeWrapper<SkTDArray<SkPoint>>;

class SkSVGLength {
public:
    enum class Unit {
        kUnknown,
        kNumber,
        kPercentage,
        kEMS,
        kEXS,
        kPX,
        kCM,
        kMM,
        kIN,
        kPT,
        kPC,
    };

    constexpr SkSVGLength()                    : fValue(0), fUnit(Unit::kUnknown) {}
    explicit constexpr SkSVGLength(SkScalar v, Unit u = Unit::kNumber)
        : fValue(v), fUnit(u) {}
    SkSVGLength(const SkSVGLength&)            = default;
    SkSVGLength& operator=(const SkSVGLength&) = default;

    bool operator==(const SkSVGLength& other) const {
        return fUnit == other.fUnit && fValue == other.fValue;
    }
    bool operator!=(const SkSVGLength& other) const { return !(*this == other); }

    const SkScalar& value() const { return fValue; }
    const Unit&     unit()  const { return fUnit;  }

private:
    SkScalar fValue;
    Unit     fUnit;
};

class SkSVGPaint {
public:
    enum class Type {
        kNone,
        kCurrentColor,
        kColor,
        kInherit,
        kIRI,
    };

    SkSVGPaint() : fType(Type::kInherit), fColor(SK_ColorBLACK) {}
    explicit SkSVGPaint(Type t) : fType(t), fColor(SK_ColorBLACK) {}
    explicit SkSVGPaint(const SkSVGColorType& c) : fType(Type::kColor), fColor(c) {}
    explicit SkSVGPaint(const SkString& iri)
        : fType(Type::kIRI), fColor(SK_ColorBLACK), fIRI(iri) {}

    SkSVGPaint(const SkSVGPaint&)            = default;
    SkSVGPaint& operator=(const SkSVGPaint&) = default;

    bool operator==(const SkSVGPaint& other) const {
        return fType == other.fType && fColor == other.fColor && fIRI == other.fIRI;
    }
    bool operator!=(const SkSVGPaint& other) const { return !(*this == other); }

    Type type() const { return fType; }
    const SkSVGColorType& color() const { SkASSERT(fType == Type::kColor); return fColor; }
    const SkString& iri() const { SkASSERT(fType == Type::kIRI); return fIRI; }

private:
    Type fType;

    // Logical union.
    SkSVGColorType fColor;
    SkString       fIRI;
};

class SkSVGClip {
public:
    enum class Type {
        kNone,
        kInherit,
        kIRI,
    };

    SkSVGClip() : fType(Type::kNone) {}
    explicit SkSVGClip(Type t) : fType(t)           {}
    explicit SkSVGClip(const SkString& iri) : fType(Type::kIRI), fIRI(iri) {}

    SkSVGClip(const SkSVGClip&)            = default;
    SkSVGClip& operator=(const SkSVGClip&) = default;

    bool operator==(const SkSVGClip& other) const {
        return fType == other.fType && fIRI == other.fIRI;
    }
    bool operator!=(const SkSVGClip& other) const { return !(*this == other); }

    Type type() const { return fType; }
    const SkString& iri() const { SkASSERT(fType == Type::kIRI); return fIRI; }

private:
    Type           fType;
    SkString       fIRI;
};

class SkSVGLineCap {
public:
    enum class Type {
        kButt,
        kRound,
        kSquare,
        kInherit,
    };

    constexpr SkSVGLineCap() : fType(Type::kInherit) {}
    constexpr explicit SkSVGLineCap(Type t) : fType(t) {}

    SkSVGLineCap(const SkSVGLineCap&)            = default;
    SkSVGLineCap& operator=(const SkSVGLineCap&) = default;

    bool operator==(const SkSVGLineCap& other) const { return fType == other.fType; }
    bool operator!=(const SkSVGLineCap& other) const { return !(*this == other); }

    Type type() const { return fType; }

private:
    Type fType;
};

class SkSVGLineJoin {
public:
    enum class Type {
        kMiter,
        kRound,
        kBevel,
        kInherit,
    };

    constexpr SkSVGLineJoin() : fType(Type::kInherit) {}
    constexpr explicit SkSVGLineJoin(Type t) : fType(t) {}

    SkSVGLineJoin(const SkSVGLineJoin&)            = default;
    SkSVGLineJoin& operator=(const SkSVGLineJoin&) = default;

    bool operator==(const SkSVGLineJoin& other) const { return fType == other.fType; }
    bool operator!=(const SkSVGLineJoin& other) const { return !(*this == other); }

    Type type() const { return fType; }

private:
    Type fType;
};

class SkSVGSpreadMethod {
public:
    // These values must match Skia's SkShader::TileMode enum.
    enum class Type {
        kPad,       // kClamp_TileMode
        kRepeat,    // kRepeat_TileMode
        kReflect,   // kMirror_TileMode
    };

    constexpr SkSVGSpreadMethod() : fType(Type::kPad) {}
    constexpr explicit SkSVGSpreadMethod(Type t) : fType(t) {}

    SkSVGSpreadMethod(const SkSVGSpreadMethod&)            = default;
    SkSVGSpreadMethod& operator=(const SkSVGSpreadMethod&) = default;

    bool operator==(const SkSVGSpreadMethod& other) const { return fType == other.fType; }
    bool operator!=(const SkSVGSpreadMethod& other) const { return !(*this == other); }

    Type type() const { return fType; }

private:
    Type fType;
};

class SkSVGFillRule {
public:
    enum class Type {
        kNonZero,
        kEvenOdd,
        kInherit,
    };

    constexpr SkSVGFillRule() : fType(Type::kInherit) {}
    constexpr explicit SkSVGFillRule(Type t) : fType(t) {}

    SkSVGFillRule(const SkSVGFillRule&)            = default;
    SkSVGFillRule& operator=(const SkSVGFillRule&) = default;

    bool operator==(const SkSVGFillRule& other) const { return fType == other.fType; }
    bool operator!=(const SkSVGFillRule& other) const { return !(*this == other); }

    Type type() const { return fType; }

    SkPathFillType asFillType() const {
        SkASSERT(fType != Type::kInherit); // should never be called for unresolved values.
        return fType == Type::kEvenOdd ? SkPathFillType::kEvenOdd : SkPathFillType::kWinding;
    }

private:
    Type fType;
};

class SkSVGVisibility {
public:
    enum class Type {
        kVisible,
        kHidden,
        kCollapse,
        kInherit,
    };

    constexpr SkSVGVisibility() : fType(Type::kVisible) {}
    constexpr explicit SkSVGVisibility(Type t) : fType(t) {}

    SkSVGVisibility(const SkSVGVisibility&)            = default;
    SkSVGVisibility& operator=(const SkSVGVisibility&) = default;

    bool operator==(const SkSVGVisibility& other) const { return fType == other.fType; }
    bool operator!=(const SkSVGVisibility& other) const { return !(*this == other); }

    Type type() const { return fType; }

private:
    Type fType;
};

class SkSVGDashArray {
public:
    enum class Type {
        kNone,
        kDashArray,
        kInherit,
    };

    SkSVGDashArray()                : fType(Type::kNone) {}
    explicit SkSVGDashArray(Type t) : fType(t) {}
    explicit SkSVGDashArray(SkTDArray<SkSVGLength>&& dashArray)
        : fType(Type::kDashArray)
        , fDashArray(std::move(dashArray)) {}

    SkSVGDashArray(const SkSVGDashArray&)            = default;
    SkSVGDashArray& operator=(const SkSVGDashArray&) = default;

    bool operator==(const SkSVGDashArray& other) const {
        return fType == other.fType && fDashArray == other.fDashArray;
    }
    bool operator!=(const SkSVGDashArray& other) const { return !(*this == other); }

    Type type() const { return fType; }

    const SkTDArray<SkSVGLength>& dashArray() const { return fDashArray; }

private:
    Type fType;
    SkTDArray<SkSVGLength> fDashArray;
};

#endif // SkSVGTypes_DEFINED
