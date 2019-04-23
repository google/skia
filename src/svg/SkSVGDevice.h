/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGDevice_DEFINED
#define SkSVGDevice_DEFINED

#include "include/private/SkTemplates.h"
#include "src/core/SkClipStackDevice.h"

class SkXMLWriter;

class SkSVGDevice : public SkClipStackDevice {
public:
    static sk_sp<SkBaseDevice> Make(const SkISize& size, std::unique_ptr<SkXMLWriter>);

protected:
    void drawPaint(const SkPaint& paint) override;
    void drawAnnotation(const SkRect& rect, const char key[], SkData* value) override;
    void drawPoints(SkCanvas::PointMode mode, size_t count,
                    const SkPoint[], const SkPaint& paint) override;
    void drawRect(const SkRect& r, const SkPaint& paint) override;
    void drawOval(const SkRect& oval, const SkPaint& paint) override;
    void drawRRect(const SkRRect& rr, const SkPaint& paint) override;
    void drawPath(const SkPath& path,
                  const SkPaint& paint,
                  bool pathIsMutable = false) override;

    void drawSprite(const SkBitmap& bitmap,
                    int x, int y, const SkPaint& paint) override;
    void drawBitmapRect(const SkBitmap&,
                        const SkRect* srcOrNull, const SkRect& dst,
                        const SkPaint& paint, SkCanvas::SrcRectConstraint) override;
    void drawGlyphRunList(const SkGlyphRunList& glyphRunList) override;
    void drawVertices(const SkVertices*, const SkVertices::Bone bones[], int boneCount, SkBlendMode,
                      const SkPaint& paint) override;

    void drawDevice(SkBaseDevice*, int x, int y,
                    const SkPaint&) override;

private:
    SkSVGDevice(const SkISize& size, std::unique_ptr<SkXMLWriter>);
    ~SkSVGDevice() override;

    struct MxCp;
    void drawBitmapCommon(const MxCp&, const SkBitmap& bm, const SkPaint& paint);

    class AutoElement;
    class ResourceBucket;

    std::unique_ptr<SkXMLWriter>    fWriter;
    std::unique_ptr<AutoElement>    fRootElement;
    std::unique_ptr<ResourceBucket> fResourceBucket;

    typedef SkClipStackDevice INHERITED;
};

#endif // SkSVGDevice_DEFINED
