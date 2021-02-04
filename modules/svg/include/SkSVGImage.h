/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGImage_DEFINED
#define SkSVGImage_DEFINED

#include "modules/svg/include/SkSVGTransformableNode.h"
#include "modules/svg/include/SkSVGTypes.h"

class SkSVGImage final : public SkSVGTransformableNode {
public:
    static sk_sp<SkSVGImage> Make() {
        return sk_sp<SkSVGImage>(new SkSVGImage());
    }

    void appendChild(sk_sp<SkSVGNode>) override {
        SkDebugf("cannot append child nodes to this element.\n");
    }

    bool onPrepareToRender(SkSVGRenderContext*) const override;
    void onRender(const SkSVGRenderContext&) const override;
    SkPath onAsPath(const SkSVGRenderContext&) const override;
    SkRect onObjectBoundingBox(const SkSVGRenderContext&) const override;

    SVG_ATTR(X                  , SkSVGLength             , SkSVGLength(0))
    SVG_ATTR(Y                  , SkSVGLength             , SkSVGLength(0))
    SVG_ATTR(Width              , SkSVGLength             , SkSVGLength(0))
    SVG_ATTR(Height             , SkSVGLength             , SkSVGLength(0))
    SVG_ATTR(Href               , SkSVGIRI                , SkSVGIRI())
    SVG_ATTR(PreserveAspectRatio, SkSVGPreserveAspectRatio, SkSVGPreserveAspectRatio())

protected:
    bool parseAndSetAttribute(const char*, const char*) override;

private:
    SkSVGImage() : INHERITED(SkSVGTag::kImage) {}

    SkRect resolveImageRect(const SkRect&, const SkRect&) const;

    using INHERITED = SkSVGTransformableNode;
};

#endif  // SkSVGImage_DEFINED
