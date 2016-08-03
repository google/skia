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
#include "SkSVGTypes.h"
#include "SkTypes.h"

class SkSVGValue : public SkNoncopyable {
public:
    enum class Type {
        kColor,
        kLength,
        kPath,
        kTransform,
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

    typedef SkNoncopyable INHERITED;
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

    typedef SkSVGValue INHERITED;
};

using SkSVGColorValue     = SkSVGWrapperValue<SkSVGColor , SkSVGValue::Type::kColor    >;
using SkSVGLengthValue    = SkSVGWrapperValue<SkSVGLength, SkSVGValue::Type::kLength   >;
using SkSVGPathValue      = SkSVGWrapperValue<SkPath     , SkSVGValue::Type::kPath     >;
using SkSVGTransformValue = SkSVGWrapperValue<SkMatrix   , SkSVGValue::Type::kTransform>;

#endif // SkSVGValue_DEFINED
