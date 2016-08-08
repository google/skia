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
#include "SkRect.h"
#include "SkScalar.h"
#include "SkTypes.h"

template <typename T>
class SkSVGPrimitiveTypeWrapper {
public:
    SkSVGPrimitiveTypeWrapper() = default;
    explicit constexpr SkSVGPrimitiveTypeWrapper(T v) : fValue(v) {}

    SkSVGPrimitiveTypeWrapper(const SkSVGPrimitiveTypeWrapper&)            = default;
    SkSVGPrimitiveTypeWrapper& operator=(const SkSVGPrimitiveTypeWrapper&) = default;

    const T& value() const { return fValue; }
    operator const T&() const { return fValue; }

private:
    T fValue;
};

using SkSVGColorType      = SkSVGPrimitiveTypeWrapper<SkColor >;
using SkSVGNumberType     = SkSVGPrimitiveTypeWrapper<SkScalar>;
using SkSVGViewBoxType    = SkSVGPrimitiveTypeWrapper<SkRect  >;
using SkSVGTransformType  = SkSVGPrimitiveTypeWrapper<SkMatrix>;

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

    const SkScalar& value() const { return fValue; }
    const Unit&     unit()  const { return fUnit;  }

private:
    SkScalar fValue;
    Unit     fUnit;
};

#endif // SkSVGTypes_DEFINED
