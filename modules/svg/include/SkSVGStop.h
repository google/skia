/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGStop_DEFINED
#define SkSVGStop_DEFINED

#include "modules/svg/include/SkSVGHiddenContainer.h"
#include "modules/svg/include/SkSVGTypes.h"

class SkSVGLengthContext;

class SkSVGStop : public SkSVGHiddenContainer {
public:
    ~SkSVGStop() override = default;
    static sk_sp<SkSVGStop> Make() {
        return sk_sp<SkSVGStop>(new SkSVGStop());
    }

    const SkSVGLength& offset() const { return fOffset; }
    const SkSVGStopColor& stopColor() const { return fStopColor; }
    const SkSVGNumberType& stopOpacity() const { return fStopOpacity; }

    void setOffset(const SkSVGLength&);
    void setStopColor(const SkSVGStopColor&);
    void setStopOpacity(const SkSVGNumberType&);

protected:
    void onSetAttribute(SkSVGAttribute, const SkSVGValue&) override;

private:
    SkSVGStop();

    SkSVGLength          fOffset = SkSVGLength(0  , SkSVGLength::Unit::kPercentage);
    SkSVGStopColor    fStopColor = SkSVGStopColor(SK_ColorBLACK);
    SkSVGNumberType fStopOpacity = SkSVGNumberType(1);

    using INHERITED = SkSVGHiddenContainer;
};

#endif // SkSVGStop_DEFINED
