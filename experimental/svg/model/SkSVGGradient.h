/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGGradient_DEFINED
#define SkSVGGradient_DEFINED

#include "SkShader.h"
#include "SkSVGHiddenContainer.h"
#include "SkSVGTypes.h"

class SkMatrix;
class SkSVGRenderContext;

class SkSVGGradient : public SkSVGHiddenContainer {
public:
    ~SkSVGGradient() override = default;

    void setHref(const SkSVGStringType&);
    void setGradientTransform(const SkSVGTransformType&);
    void setSpreadMethod(const SkSVGSpreadMethod&);

protected:
    explicit SkSVGGradient(SkSVGTag t) : INHERITED(t) {}

    void onSetAttribute(SkSVGAttribute, const SkSVGValue&) override;

    bool onAsPaint(const SkSVGRenderContext&, SkPaint*) const final;

    virtual sk_sp<SkShader> onMakeShader(const SkSVGRenderContext&,
                                         const SkColor*, const SkScalar*, int count,
                                         SkTileMode, const SkMatrix& localMatrix) const = 0;

private:
    using StopPositionArray = SkSTArray<2, SkScalar, true>;
    using    StopColorArray = SkSTArray<2,  SkColor, true>;
    void collectColorStops(const SkSVGRenderContext&, StopPositionArray*, StopColorArray*) const;

    SkSVGStringType    fHref;
    SkSVGTransformType fGradientTransform = SkSVGTransformType(SkMatrix::I());
    SkSVGSpreadMethod  fSpreadMethod = SkSVGSpreadMethod(SkSVGSpreadMethod::Type::kPad);

    typedef SkSVGHiddenContainer INHERITED;
};

#endif // SkSVGGradient_DEFINED
