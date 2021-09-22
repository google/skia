/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Device_DEFINED
#define skgpu_Device_DEFINED

#include "src/core/SkDevice.h"

namespace skgpu {

class SurfaceDrawContext;

class Device final : public SkBaseDevice  {
public:
    static sk_sp<Device> Make(const SkImageInfo&);

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

    sk_sp<SkSurface> makeSurface(const SkImageInfo&, const SkSurfaceProps&) override;
    bool onReadPixels(const SkPixmap&, int x, int y) override;

private:
    Device(sk_sp<SurfaceDrawContext>);

    sk_sp<SurfaceDrawContext> fSDC;
};

} // namespace skgpu

#endif // skgpu_Device_DEFINED
