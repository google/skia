/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGSVG_DEFINED
#define SkSVGSVG_DEFINED

#include "modules/svg/include/SkSVGContainer.h"
#include "modules/svg/include/SkSVGTypes.h"
#include "src/core/SkTLazy.h"

class SkSVGLengthContext;

class SkSVGSVG : public SkSVGContainer {
public:
    ~SkSVGSVG() override = default;

    static sk_sp<SkSVGSVG> Make() { return sk_sp<SkSVGSVG>(new SkSVGSVG()); }

    SVG_ATTR(X                  , SkSVGLength, SkSVGLength(0))
    SVG_ATTR(Y                  , SkSVGLength, SkSVGLength(0))
    SVG_ATTR(Width              , SkSVGLength, SkSVGLength(100, SkSVGLength::Unit::kPercentage))
    SVG_ATTR(Height             , SkSVGLength, SkSVGLength(100, SkSVGLength::Unit::kPercentage))
    SVG_ATTR(PreserveAspectRatio, SkSVGPreserveAspectRatio, SkSVGPreserveAspectRatio())

    // TODO: SVG_ATTR is not smart enough to handle SkTLazy<T>
    void setViewBox(const SkSVGViewBoxType&);

    SkSize intrinsicSize(const SkSVGLengthContext&) const;

protected:
    bool onPrepareToRender(SkSVGRenderContext*) const override;

    void onSetAttribute(SkSVGAttribute, const SkSVGValue&) override;

private:
    SkSVGSVG();

    SkTLazy<SkSVGViewBoxType> fViewBox;

    using INHERITED = SkSVGContainer;
};

#endif // SkSVGSVG_DEFINED
