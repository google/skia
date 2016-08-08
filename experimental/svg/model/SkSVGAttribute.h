/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGAttribute_DEFINED
#define SkSVGAttribute_DEFINED

#include "SkSVGTypes.h"
#include "SkTLazy.h"

class SkSVGRenderContext;

enum class SkSVGAttribute {
    kD,
    kFill,
    kHeight,
    kStroke,
    kTransform,
    kViewBox,
    kWidth,
    kX,
    kY,

    kUnknown,
};

class SkSVGPresentationAttributes {
public:
    SkSVGPresentationAttributes();

    void setFill(const SkSVGColorType&);
    void setStroke(const SkSVGColorType&);

    void applyTo(SkSVGRenderContext*) const;

private:
    // Color only for now.
    SkSVGColorType fFill;
    SkSVGColorType fStroke;

    unsigned fFillIsSet   : 1;
    unsigned fStrokeIsSet : 1;
};

#endif // SkSVGAttribute_DEFINED
