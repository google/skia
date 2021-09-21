/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Device_DEFINED
#define Device_DEFINED

#include "src/core/SkDevice.h"

namespace sktbd {

class Device final : public SkBaseDevice  {
public:
    Device(const SkImageInfo&);

protected:
    bool onClipIsAA() const override { return false; }
    bool onClipIsWideOpen() const override { return false; }
    void onAsRgnClip(SkRegion*) const override { }
    ClipType onGetClipType() const override { return ClipType::kEmpty; }
    SkIRect onDevClipBounds() const override { return {}; }

    void drawPaint(const SkPaint& paint) override {}
    void drawPoints(SkCanvas::PointMode mode, size_t count,
                    const SkPoint[], const SkPaint& paint) override {}
    void drawRect(const SkRect& r, const SkPaint& paint) override {}
    void drawOval(const SkRect& oval, const SkPaint& paint) override {}
    void drawRRect(const SkRRect& rr, const SkPaint& paint) override {}
    void drawPath(const SkPath& path,
                  const SkPaint& paint,
                  bool pathIsMutable = false) override {}

    void drawImageRect(const SkImage*, const SkRect* src, const SkRect& dst,
                       const SkSamplingOptions&, const SkPaint&,
                       SkCanvas::SrcRectConstraint) override {}

    void drawVertices(const SkVertices*, SkBlendMode, const SkPaint&) override {}
    void onDrawGlyphRunList(const SkGlyphRunList& glyphRunList, const SkPaint& paint) override {}

private:
};

} // namespace sktbd

#endif // Device_DEFINED
