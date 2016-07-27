/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGValue_DEFINED
#define SkSVGValue_DEFINED

#include "SkColor.h"
#include "SkMatrix.h"
#include "SkPath.h"
#include "SkTypes.h"

class SkSVGValue : public SkNoncopyable {
public:
    enum class Type {
        Color,
        Path,
        Transform,
    };

    Type type() const { return fType; }

    template <typename T>
    const T* as() const {
        return fType == T::TYPE ? static_cast<const T*>(this) : nullptr;
    }

protected:
    SkSVGValue(Type t) : fType(t) { }

private:
    Type fType;
};

template <typename SkiaType, SkSVGValue::Type ValueType>
class SkSVGWrapperValue final : public SkSVGValue {
public:
    static constexpr Type TYPE = ValueType;

    explicit SkSVGWrapperValue(const SkiaType& v)
        : INHERITED(ValueType)
        , fWrappedValue(v) { }

    operator const SkiaType&() const { return fWrappedValue; }

private:
    SkiaType fWrappedValue;

    using INHERITED = SkSVGValue;
};

using SkSVGColorValue     = SkSVGWrapperValue<SkColor , SkSVGValue::Type::Color    >;
using SkSVGPathValue      = SkSVGWrapperValue<SkPath  , SkSVGValue::Type::Path     >;
using SkSVGTransformValue = SkSVGWrapperValue<SkMatrix, SkSVGValue::Type::Transform>;

#endif // SkSVGValue_DEFINED
