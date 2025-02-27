/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGFeColorMatrix.h"

#include "include/core/SkColorFilter.h"
#include "include/core/SkScalar.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/base/SkAssert.h"
#include "modules/svg/include/SkSVGAttributeParser.h"
#include "modules/svg/include/SkSVGFilterContext.h"
#include "src/core/SkColorData.h"

#include <tuple>

class SkImageFilter;
class SkSVGRenderContext;

bool SkSVGFeColorMatrix::parseAndSetAttribute(const char* name, const char* value) {
    return INHERITED::parseAndSetAttribute(name, value) ||
           this->setType(
                   SkSVGAttributeParser::parse<SkSVGFeColorMatrixType>("type", name, value)) ||
           this->setValues(
                   SkSVGAttributeParser::parse<SkSVGFeColorMatrixValues>("values", name, value));
}

SkColorMatrix SkSVGFeColorMatrix::makeMatrixForType() const {
    if (fValues.empty() && fType != SkSVGFeColorMatrixType::kLuminanceToAlpha) {
        return SkColorMatrix();
    }

    switch (fType) {
        case SkSVGFeColorMatrixType::kMatrix: {
            if (fValues.size() < 20) {
                return SkColorMatrix();
            }
            SkColorMatrix m;
            m.setRowMajor(fValues.data());
            return m;
        }
        case SkSVGFeColorMatrixType::kSaturate:
            return MakeSaturate(!fValues.empty() ? fValues[0] : 1);
        case SkSVGFeColorMatrixType::kHueRotate:
            return MakeHueRotate(!fValues.empty() ? fValues[0] : 0);
        case SkSVGFeColorMatrixType::kLuminanceToAlpha:
            return MakeLuminanceToAlpha();
    }

    SkUNREACHABLE;
}

SkColorMatrix SkSVGFeColorMatrix::MakeSaturate(SkSVGNumberType s) {
    SkColorMatrix m;
    m.setSaturation(s);
    return m;
}

SkColorMatrix SkSVGFeColorMatrix::MakeHueRotate(SkSVGNumberType degrees) {
    const SkScalar theta = SkDegreesToRadians(degrees);
    const SkSVGNumberType c = SkScalarCos(theta);
    const SkSVGNumberType s = SkScalarSin(theta);
    return SkColorMatrix(
        0.213f + c* 0.787f + s*-0.213f,
        0.715f + c*-0.715f + s*-0.715f,
        0.072f + c*-0.072f + s* 0.928f,
        0,
        0,

        0.213f + c*-0.213f + s* 0.143f,
        0.715f + c* 0.285f + s* 0.140f,
        0.072f + c*-0.072f + s*-0.283f,
        0,
        0,

        0.213f + c*-0.213f + s*-0.787f,
        0.715f + c*-0.715f + s* 0.715f,
        0.072f + c* 0.928f + s* 0.072f,
        0,
        0,

        0,0,0,1,0
    );
}

SkColorMatrix SkSVGFeColorMatrix::MakeLuminanceToAlpha() {
    return SkColorMatrix(
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        SK_LUM_COEFF_R, SK_LUM_COEFF_G, SK_LUM_COEFF_B, 0, 0
    );
}

sk_sp<SkImageFilter> SkSVGFeColorMatrix::onMakeImageFilter(const SkSVGRenderContext& ctx,
                                                           const SkSVGFilterContext& fctx) const {
    return SkImageFilters::ColorFilter(
            SkColorFilters::Matrix(makeMatrixForType()),
            fctx.resolveInput(ctx, this->getIn(), this->resolveColorspace(ctx, fctx)),
            this->resolveFilterSubregion(ctx, fctx));
}

template <> bool SkSVGAttributeParser::parse(SkSVGFeColorMatrixType* type) {
    static constexpr std::tuple<const char*, SkSVGFeColorMatrixType> gTypeMap[] = {
            {"matrix", SkSVGFeColorMatrixType::kMatrix},
            {"saturate", SkSVGFeColorMatrixType::kSaturate},
            {"hueRotate", SkSVGFeColorMatrixType::kHueRotate},
            {"luminanceToAlpha", SkSVGFeColorMatrixType::kLuminanceToAlpha},
    };

    return this->parseEnumMap(gTypeMap, type) && this->parseEOSToken();
}
