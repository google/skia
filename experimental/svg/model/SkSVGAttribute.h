/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGAttribute_DEFINED
#define SkSVGAttribute_DEFINED

#include "SkColor.h"
#include "SkTLazy.h"

enum class SkSVGAttribute {
    d,
    fill,
    stroke,
    transform,
};

class SkSVGRenderContext;

class SkSVGPresentationAttributes {
public:
    SkSVGPresentationAttributes();

    void setFill(SkColor);
    void setStroke(SkColor);

    void applyTo(SkTCopyOnFirstWrite<SkSVGRenderContext>&) const;

private:
    // Color only for now.
    SkColor fFill;
    SkColor fStroke;

    unsigned fFillIsSet   : 1;
    unsigned fStrokeIsSet : 1;
};

#endif // SkSVGAttribute_DEFINED
