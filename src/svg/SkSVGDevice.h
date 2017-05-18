/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGDevice_DEFINED
#define SkSVGDevice_DEFINED

#include "SkClipStackDevice.h"
#include "SkTemplates.h"

class SkXMLWriter;

class SkSVGDevice : public SkClipStackDevice {
public:
    static SkBaseDevice* Create(const SkISize& size, SkXMLWriter* writer);

protected:
    void drawPaint(const SkPaint& paint) override;
    void drawPoints(SkCanvas::PointMode mode, size_t count,
                    const SkPoint[], const SkPaint& paint) override;
    void drawRect(const SkRect& r, const SkPaint& paint) override;
    void drawOval(const SkRect& oval, const SkPaint& paint) override;
    void drawRRect(const SkRRect& rr, const SkPaint& paint) override;
    void drawPath(const SkPath& path,
                  const SkPaint& paint,
                  const SkMatrix* prePathMatrix = nullptr,
                  bool pathIsMutable = false) override;

    void drawBitmap(const SkBitmap& bitmap,
                    const SkMatrix& matrix, const SkPaint& paint) override;
    void drawSprite(const SkBitmap& bitmap,
                    int x, int y, const SkPaint& paint) override;
    void drawBitmapRect(const SkBitmap&,
                        const SkRect* srcOrNull, const SkRect& dst,
                        const SkPaint& paint, SkCanvas::SrcRectConstraint) override;

    void drawText(const void* text, size_t len,
                  SkScalar x, SkScalar y, const SkPaint& paint) override;
    void drawPosText(const void* text, size_t len,
                     const SkScalar pos[], int scalarsPerPos,
                     const SkPoint& offset, const SkPaint& paint) override;
    void drawTextOnPath(const void* text, size_t len,
                        const SkPath& path, const SkMatrix* matrix,
                        const SkPaint& paint) override;
    void drawVertices(const SkVertices*, SkBlendMode, const SkPaint& paint) override;

    void drawDevice(SkBaseDevice*, int x, int y,
                    const SkPaint&) override;

private:
    SkSVGDevice(const SkISize& size, SkXMLWriter* writer);
    ~SkSVGDevice() override;

    struct MxCp;
    void drawBitmapCommon(const MxCp&, const SkBitmap& bm, const SkPaint& paint);

    class AutoElement;
    class ResourceBucket;

    SkXMLWriter*                    fWriter;
    std::unique_ptr<AutoElement>    fRootElement;
    std::unique_ptr<ResourceBucket> fResourceBucket;

    typedef SkClipStackDevice INHERITED;
};

#endif // SkSVGDevice_DEFINED
