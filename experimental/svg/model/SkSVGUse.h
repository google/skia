/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGUse_DEFINED
#define SkSVGUse_DEFINED

#include "experimental/svg/model/SkSVGTransformableNode.h"
#include "experimental/svg/model/SkSVGTypes.h"

/**
 * Implements support for <use> (reference) elements.
 * (https://www.w3.org/TR/SVG/struct.html#UseElement)
 */
class SkSVGUse final : public SkSVGTransformableNode {
public:
    ~SkSVGUse() override = default;

    static sk_sp<SkSVGUse> Make() { return sk_sp<SkSVGUse>(new SkSVGUse()); }

    void appendChild(sk_sp<SkSVGNode>) override;

    void setHref(const SkSVGStringType&);
    void setX(const SkSVGLength&);
    void setY(const SkSVGLength&);

protected:
    void onSetAttribute(SkSVGAttribute, const SkSVGValue&) override;

    bool onPrepareToRender(SkSVGRenderContext*) const override;
    void onRender(const SkSVGRenderContext&) const override;
    SkPath onAsPath(const SkSVGRenderContext&) const override;

private:
    SkSVGUse();

    SkSVGStringType    fHref;
    SkSVGLength        fX = SkSVGLength(0);
    SkSVGLength        fY = SkSVGLength(0);

    typedef SkSVGTransformableNode INHERITED;
};

#endif // SkSVGUse_DEFINED
