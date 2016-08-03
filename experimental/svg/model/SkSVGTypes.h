/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGTypes_DEFINED
#define SkSVGTypes_DEFINED

#include "SkColor.h"
#include "SkScalar.h"
#include "SkTypes.h"

class SkSVGNumber {
public:
    constexpr SkSVGNumber()                    : fValue(0) {}
    explicit constexpr SkSVGNumber(SkScalar v) : fValue(v) {}
    SkSVGNumber(const SkSVGNumber&)            = default;
    SkSVGNumber& operator=(const SkSVGNumber&) = default;


    const SkScalar& value() const { return fValue; }

    operator const SkScalar&() const { return fValue; }

private:
    SkScalar fValue;
};

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

class SkSVGColor {
public:
    constexpr SkSVGColor()                   : fValue(SK_ColorBLACK) {}
    explicit constexpr SkSVGColor(SkColor c) : fValue(c) {}

    operator const SkColor&() const { return fValue; }

private:
    SkColor fValue;
};

#endif // SkSVGTypes_DEFINED
