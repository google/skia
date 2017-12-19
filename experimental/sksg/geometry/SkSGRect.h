/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGRect_DEFINED
#define SkSGRect_DEFINED

#include "SkSGGeometryNode.h"

#include "SkRect.h"

class SkCanvas;
class SkPaint;

namespace sksg {

class Rect : public GeometryNode {
public:
    static sk_sp<Rect> Make()                { return sk_sp<Rect>(new Rect(SkRect::MakeEmpty())); }
    static sk_sp<Rect> Make(const SkRect& r) { return sk_sp<Rect>(new Rect(r)); }

    MAPPED_ATTRIBUTE(l, SkScalar, fRect.fLeft  )
    MAPPED_ATTRIBUTE(t, SkScalar, fRect.fTop   )
    MAPPED_ATTRIBUTE(r, SkScalar, fRect.fRight )
    MAPPED_ATTRIBUTE(b, SkScalar, fRect.fBottom)

protected:
    void onDraw(SkCanvas*, const SkPaint&) const override;

    SkRect onComputeBounds() const override;

private:
    explicit Rect(const SkRect&);

    SkRect fRect;
};

} // namespace sksg

#endif // SkSGRect_DEFINED
