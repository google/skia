/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGText_DEFINED
#define SkSGText_DEFINED

#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/utils/SkTextUtils.h"
#include "modules/sksg/include/SkSGGeometryNode.h"
#include "modules/sksg/include/SkSGNode.h"

class SkCanvas;
class SkMatrix;
class SkPaint;
class SkTextBlob;
class SkTypeface;

namespace sksg {
class InvalidationController;

/**
 * Concrete Geometry node, wrapping a (shaped) SkTextBlob.
 */
class Text final : public GeometryNode {
public:
    static sk_sp<Text> Make(sk_sp<SkTypeface> tf, const SkString& text);
    ~Text() override;

    SG_ATTRIBUTE(Typeface, sk_sp<SkTypeface> , fTypeface)
    SG_ATTRIBUTE(Text    , SkString          , fText    )
    SG_ATTRIBUTE(Position, SkPoint           , fPosition)
    SG_ATTRIBUTE(Size    , SkScalar          , fSize    )
    SG_ATTRIBUTE(ScaleX  , SkScalar          , fScaleX  )
    SG_ATTRIBUTE(SkewX   , SkScalar          , fSkewX   )
    SG_ATTRIBUTE(Align   , SkTextUtils::Align, fAlign   )
    SG_ATTRIBUTE(Edging  , SkFont::Edging    , fEdging  )
    SG_ATTRIBUTE(Hinting , SkFontHinting     , fHinting )

    // TODO: add shaping functionality.

protected:
    void onClip(SkCanvas*, bool antiAlias) const override;
    void onDraw(SkCanvas*, const SkPaint&) const override;
    bool onContains(const SkPoint&)        const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;
    SkPath onAsPath() const override;

private:
    Text(sk_sp<SkTypeface>, const SkString&);

    SkPoint alignedPosition(SkScalar advance) const;

    sk_sp<SkTypeface> fTypeface;
    SkString                fText;
    SkPoint                 fPosition = SkPoint::Make(0, 0);
    SkScalar                fSize     = 12;
    SkScalar                fScaleX   = 1;
    SkScalar                fSkewX    = 0;
    SkTextUtils::Align      fAlign    = SkTextUtils::kLeft_Align;
    SkFont::Edging          fEdging   = SkFont::Edging::kAntiAlias;
    SkFontHinting           fHinting  = SkFontHinting::kNormal;

    sk_sp<SkTextBlob> fBlob; // cached text blob

    using INHERITED = GeometryNode;
};

} // namespace sksg

#endif // SkSGText_DEFINED
