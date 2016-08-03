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

enum class SkSVGAttribute {
    kD,
    kFill,
    kHeight,
    kStroke,
    kTransform,
    kWidth,
    kX,
    kY,

    kUnknown,
};

class SkSVGRenderContext;

class SkSVGPresentationAttributes {
public:
    SkSVGPresentationAttributes();

    void setFill(const SkSVGColor&);
    void setStroke(const SkSVGColor&);

    void applyTo(SkTCopyOnFirstWrite<SkSVGRenderContext>&) const;

private:
    // Color only for now.
    SkSVGColor fFill;
    SkSVGColor fStroke;

    unsigned fFillIsSet   : 1;
    unsigned fStrokeIsSet : 1;
};

#endif // SkSVGAttribute_DEFINED
