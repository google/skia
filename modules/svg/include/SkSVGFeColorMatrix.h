/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGFeColorMatrix_DEFINED
#define SkSVGFeColorMatrix_DEFINED

#include "modules/svg/include/SkSVGFe.h"
#include "modules/svg/include/SkSVGTypes.h"

class SkSVGFeColorMatrix : public SkSVGFe {
public:
    ~SkSVGFeColorMatrix() override = default;
    static sk_sp<SkSVGFeColorMatrix> Make() {
        return sk_sp<SkSVGFeColorMatrix>(new SkSVGFeColorMatrix());
    }

    SVG_ATTR(Type,
             SkSVGFeColorMatrixType,
             SkSVGFeColorMatrixType(SkSVGFeColorMatrixType::Type::kMatrix))
    SVG_ATTR(Values, SkSVGFeColorMatrixValues, SkSVGFeColorMatrixValues())

protected:
    sk_sp<SkImageFilter> onMakeImageFilter(const SkSVGRenderContext&,
                                           const SkSVGFilterContext&) const override;

    bool parseAndSetAttribute(const char*, const char*) override;

private:
    SkSVGFeColorMatrix() : INHERITED(SkSVGTag::kFeColorMatrix) {}

    SkSVGFeColorMatrixValues makeMatrixForType() const;

    using INHERITED = SkSVGFe;
};

#endif  // SkSVGStop_DEFINED
