/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGTypes_DEFINED
#define SkSVGTypes_DEFINED

#include "SkColor.h"
#include "SkMatrix.h"
#include "SkPoint.h"
#include "SkRect.h"
#include "SkScalar.h"
#include "SkTDArray.h"
#include "SkTypes.h"

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
    };

    constexpr SkSVGPaint() : fType(Type::kInherit), fColor(SK_ColorBLACK) {}
    explicit constexpr SkSVGPaint(Type t) : fType(t), fColor(SK_ColorBLACK) {}
    explicit constexpr SkSVGPaint(const SkSVGColorType& c) : fType(Type::kColor), fColor(c) {}

    SkSVGPaint(const SkSVGPaint&)            = default;
    SkSVGPaint& operator=(const SkSVGPaint&) = default;

    bool operator==(const SkSVGPaint& other) const {
        return fType == other.fType && fColor == other.fColor;
    }
    bool operator!=(const SkSVGPaint& other) const { return !(*this == other); }

    Type type() const { return fType; }
    const SkSVGColorType& color() const { SkASSERT(fType == Type::kColor); return fColor; }

private:
    Type fType;

    SkSVGColorType fColor;
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

#endif // SkSVGTypes_DEFINED
