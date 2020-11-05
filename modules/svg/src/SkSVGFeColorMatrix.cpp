/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/effects/SkImageFilters.h"
#include "modules/svg/include/SkSVGAttributeParser.h"
#include "modules/svg/include/SkSVGFeColorMatrix.h"
#include "modules/svg/include/SkSVGFilterContext.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGValue.h"

bool SkSVGFeColorMatrix::parseAndSetAttribute(const char* name, const char* value) {
    return INHERITED::parseAndSetAttribute(name, value) ||
           this->setType(
                   SkSVGAttributeParser::parse<SkSVGFeColorMatrixType>("type", name, value)) ||
           this->setValues(
                   SkSVGAttributeParser::parse<SkSVGFeColorMatrixValues>("values", name, value));
}

SkSVGFeColorMatrixValues SkSVGFeColorMatrix::makeMatrixForType() const {
    if (fValues.values().empty()) {
        return SkSVGFeColorMatrixValues::MakeIdentity();
    }

    switch (fType.fType) {
        case SkSVGFeColorMatrixType::Type::kMatrix:
            SkASSERT(fValues.values().count() == 20);
            return this->fValues;
        case SkSVGFeColorMatrixType::Type::kSaturate:
            SkASSERT(fValues.values().count() == 1);
            return SkSVGFeColorMatrixValues::MakeSaturate(fValues.values()[0]);
        case SkSVGFeColorMatrixType::Type::kHueRotate:
            SkASSERT(fValues.values().count() == 1);
            return SkSVGFeColorMatrixValues::MakeHueRotate(fValues.values()[0]);
        case SkSVGFeColorMatrixType::Type::kLuminanceToAlpha:
            return SkSVGFeColorMatrixValues::MakeLuminanceToAlpha();
    }

    SkUNREACHABLE;
}

sk_sp<SkImageFilter> SkSVGFeColorMatrix::onMakeImageFilter(const SkSVGRenderContext& ctx,
                                                           const SkSVGFilterContext& fctx) const {
    // TODO: "in" param should supply filter source
    const sk_sp<SkImageFilter> input = nullptr;
    const SkSVGFeColorMatrixValues matrix = makeMatrixForType();
    return SkImageFilters::ColorFilter(
            SkColorFilters::Matrix(matrix.values().begin()), input, fctx.filterEffectsRegion());
}

template <>
bool SkSVGAttributeParser::parse(SkSVGFeColorMatrixValues* values) {
    SkSVGNumberType value;
    if (!this->parseNumber(&value)) {
        return false;
    }

    SkTDArray<SkSVGNumberType> numbers;
    numbers.push_back(value);
    while (true) {
        if (!this->parseNumber(&value) ||
            numbers.count() >= SkSVGFeColorMatrixValues::kN * SkSVGFeColorMatrixValues::kM) {
            break;
        }
        numbers.push_back(value);
    }

    *values = SkSVGFeColorMatrixValues(std::move(numbers));
    return this->parseEOSToken();
}

template <>
bool SkSVGAttributeParser::parse(SkSVGFeColorMatrixType* type) {
    static constexpr std::tuple<const char*, SkSVGFeColorMatrixType::Type> gTypeMap[] = {
            {"matrix", SkSVGFeColorMatrixType::Type::kMatrix},
            {"saturate", SkSVGFeColorMatrixType::Type::kSaturate},
            {"hueRotate", SkSVGFeColorMatrixType::Type::kHueRotate},
            {"luminanceToAlpha", SkSVGFeColorMatrixType::Type::kLuminanceToAlpha},
    };

    SkSVGFeColorMatrixType::Type t;
    bool parsedValue = false;
    if (this->parseEnumMap(gTypeMap, &t)) {
        parsedValue = true;
        *type = SkSVGFeColorMatrixType(t);
    }

    return parsedValue && this->parseEOSToken();
}
