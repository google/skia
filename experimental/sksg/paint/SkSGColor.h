/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGColor_DEFINED
#define SkSGColor_DEFINED

#include "SkSGPaintNode.h"

#include "SkColor.h"

namespace sksg {

class Color : public PaintNode {
public:
    static sk_sp<Color> Make(SkColor c) { return sk_sp<Color>(new Color(c)); }

    MAPPED_ATTRIBUTE(color, SkColor, fColor)

protected:
    SkPaint onMakePaint() const override;

private:
    explicit Color(SkColor);

    SkColor fColor;
};

} // namespace sksg

#endif // SkSGColor_DEFINED
