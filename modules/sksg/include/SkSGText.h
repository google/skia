/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGText_DEFINED
#define SkSGText_DEFINED

#include "SkSGGeometryNode.h"

#include "SkPaintDefaults.h"
#include "SkPoint.h"
#include "SkString.h"

class SkCanvas;
class SkPaint;
class SkTextBlob;
class SkTypeface;

namespace sksg {

/**
 * Concrete Geometry node, wrapping a (shaped) SkTextBlob.
 */
class Text final : public GeometryNode {
public:
    static sk_sp<Text> Make(sk_sp<SkTypeface> tf, const SkString& text);
    ~Text() override;

    SG_ATTRIBUTE(Text    , SkString      , fText    )
    SG_ATTRIBUTE(Flags   , uint32_t      , fFlags   )
    SG_ATTRIBUTE(Position, SkPoint       , fPosition)
    SG_ATTRIBUTE(Size    , SkScalar      , fSize    )
    SG_ATTRIBUTE(ScaleX  , SkScalar      , fScaleX  )
    SG_ATTRIBUTE(SkewX   , SkScalar      , fSkewX   )
    SG_ATTRIBUTE(Align   , SkPaint::Align, fAlign   )

    // TODO: add shaping functionality.

protected:
    void onClip(SkCanvas*, bool antiAlias) const override;
    void onDraw(SkCanvas*, const SkPaint&) const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;
    SkPath onAsPath() const override;

private:
    explicit Text(sk_sp<SkTypeface>, const SkString&);

    const sk_sp<SkTypeface> fTypeface;
    SkString                fText;
    uint32_t                fFlags    = SkPaintDefaults_Flags;
    SkPoint                 fPosition = SkPoint::Make(0, 0);
    SkScalar                fSize     = SkPaintDefaults_TextSize;
    SkScalar                fScaleX   = 1;
    SkScalar                fSkewX    = 0;
    SkPaint::Align          fAlign    = SkPaint::kLeft_Align;
    SkPaint::Hinting        fHinting  = SkPaintDefaults_Hinting;

    sk_sp<SkTextBlob> fBlob; // cached text blob

    using INHERITED = GeometryNode;
};

} // namespace sksg

#endif // SkSGText_DEFINED
