/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDevice.h"

class SkDeviceRecorder : public SkBaseDevice {
protected:
    void onSave() override {}
    void onRestore() override {}
    void onClipRect(const SkRect& rect, SkClipOp, bool aa) override {}
    void onClipRRect(const SkRRect& rrect, SkClipOp, bool aa) override {}
    void onClipPath(const SkPath& path, SkClipOp, bool aa) override {}
    void onClipRegion(const SkRegion& deviceRgn, SkClipOp) override {}
    void onSetDeviceClipRestriction(SkIRect* mutableClipRestriction) override {}
    bool onClipIsAA() const override { return false; }
    void onAsRgnClip(SkRegion* rgn) const override {
        rgn->setRect(SkIRect::MakeWH(this->width(), this->height()));
    }
    ClipType onGetClipType() const override {
        return kRect_ClipType;
    }

    void drawPaint(const SkPaint& paint) override {}
    void drawPoints(SkCanvas::PointMode, size_t, const SkPoint[], const SkPaint&) override {}
    void drawRect(const SkRect&, const SkPaint&) override {}
    void drawOval(const SkRect&, const SkPaint&) override {}
    void drawRRect(const SkRRect&, const SkPaint&) override {}
    void drawPath(const SkPath&, const SkPaint&, bool) override {}
    void drawBitmap(const SkBitmap&, SkScalar x, SkScalar y, const SkPaint&) override {}
    void drawSprite(const SkBitmap&, int, int, const SkPaint&) override {}
    void drawBitmapRect(const SkBitmap&, const SkRect*, const SkRect&, const SkPaint&,
                        SkCanvas::SrcRectConstraint) override {}
    void drawPosText(const void*, size_t, const SkScalar[], int, const SkPoint&,
                     const SkPaint&) override {}
    void drawDevice(SkBaseDevice*, int, int, const SkPaint&) override {}
    void drawVertices(const SkVertices*, const SkVertices::Bone[], int, SkBlendMode,
                      const SkPaint&) override {}
};

#endif
