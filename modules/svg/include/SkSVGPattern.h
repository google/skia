/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGPattern_DEFINED
#define SkSVGPattern_DEFINED

#include "modules/svg/include/SkSVGHiddenContainer.h"
#include "modules/svg/include/SkSVGTypes.h"

class SkSVGRenderContext;

class SkSVGPattern final : public SkSVGHiddenContainer {
public:
    static sk_sp<SkSVGPattern> Make() {
        return sk_sp<SkSVGPattern>(new SkSVGPattern());
    }

    void setX(const SkSVGLength&);
    void setY(const SkSVGLength&);
    void setWidth(const SkSVGLength&);
    void setHeight(const SkSVGLength&);
    void setHref(const SkSVGStringType&);
    void setPatternTransform(const SkSVGTransformType&);

protected:
    SkSVGPattern();

    void onSetAttribute(SkSVGAttribute, const SkSVGValue&) override;

    bool onAsPaint(const SkSVGRenderContext&, SkPaint*) const override;

private:
    struct PatternAttributes {
        SkTLazy<SkSVGLength>        fX,
                                    fY,
                                    fWidth,
                                    fHeight;
        SkTLazy<SkSVGTransformType> fPatternTransform;
    } fAttributes;

    SkSVGStringType    fHref;

    const SkSVGPattern* resolveHref(const SkSVGRenderContext&, PatternAttributes*) const;
    const SkSVGPattern* hrefTarget(const SkSVGRenderContext&) const;

    // TODO:
    //   - patternUnits
    //   - patternContentUnits

    using INHERITED = SkSVGHiddenContainer;
};

#endif // SkSVGPattern_DEFINED
