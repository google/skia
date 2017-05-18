/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGSVG_DEFINED
#define SkSVGSVG_DEFINED

#include "SkSVGContainer.h"
#include "SkSVGTypes.h"
#include "SkTLazy.h"

class SkSVGLengthContext;

class SkSVGSVG : public SkSVGContainer {
public:
    ~SkSVGSVG() override = default;

    static sk_sp<SkSVGSVG> Make() { return sk_sp<SkSVGSVG>(new SkSVGSVG()); }

    void setX(const SkSVGLength&);
    void setY(const SkSVGLength&);
    void setWidth(const SkSVGLength&);
    void setHeight(const SkSVGLength&);
    void setViewBox(const SkSVGViewBoxType&);

    SkSize intrinsicSize(const SkSVGLengthContext&) const;

protected:
    bool onPrepareToRender(SkSVGRenderContext*) const override;

    void onSetAttribute(SkSVGAttribute, const SkSVGValue&) override;

private:
    SkSVGSVG();

    SkSVGLength fX      = SkSVGLength(0);
    SkSVGLength fY      = SkSVGLength(0);
    SkSVGLength fWidth  = SkSVGLength(100, SkSVGLength::Unit::kPercentage);
    SkSVGLength fHeight = SkSVGLength(100, SkSVGLength::Unit::kPercentage);

    SkTLazy<SkSVGViewBoxType> fViewBox;

    typedef SkSVGContainer INHERITED;
};

#endif // SkSVGSVG_DEFINED
