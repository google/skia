/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGDevice_DEFINED
#define SkSVGDevice_DEFINED

#include "include/private/SkTArray.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkClipStackDevice.h"

class SkXMLWriter;

class SkSVGDevice final : public SkClipStackDevice {
public:
    static sk_sp<SkBaseDevice> Make(const SkISize& size, std::unique_ptr<SkXMLWriter>,
                                    uint32_t flags);

protected:
    void drawPaint(const SkPaint& paint) override;
    void drawAnnotation(const SkRect& rect, const char key[], SkData* value) override;
    void drawPoints(SkCanvas::PointMode mode, size_t count,
                    const SkPoint[], const SkPaint& paint) override;
    void drawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                       const SkSamplingOptions&, const SkPaint& paint,
                       SkCanvas::SrcRectConstraint constraint) override;
    void drawRect(const SkRect& r, const SkPaint& paint) override;
    void drawOval(const SkRect& oval, const SkPaint& paint) override;
    void drawRRect(const SkRRect& rr, const SkPaint& paint) override;
    void drawPath(const SkPath& path,
                  const SkPaint& paint,
                  bool pathIsMutable = false) override;

    void drawGlyphRunList(const SkGlyphRunList& glyphRunList) override;
    void drawVertices(const SkVertices*, SkBlendMode, const SkPaint&) override;

private:
    SkSVGDevice(const SkISize& size, std::unique_ptr<SkXMLWriter>, uint32_t);
    ~SkSVGDevice() override;

    struct MxCp;
    void drawBitmapCommon(const MxCp&, const SkBitmap& bm, const SkPaint& paint);

    void syncClipStack(const SkClipStack&);

    class AutoElement;
    class ResourceBucket;

    const std::unique_ptr<SkXMLWriter>    fWriter;
    const std::unique_ptr<ResourceBucket> fResourceBucket;
    const uint32_t                        fFlags;

    struct ClipRec {
        std::unique_ptr<AutoElement> fClipPathElem;
        uint32_t                     fGenID;
    };

    std::unique_ptr<AutoElement>    fRootElement;
    SkTArray<ClipRec>               fClipStack;

    using INHERITED = SkClipStackDevice;
};

#endif // SkSVGDevice_DEFINED
