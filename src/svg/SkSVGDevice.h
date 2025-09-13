/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGDevice_DEFINED
#define SkSVGDevice_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTypeTraits.h"
#include "include/utils/SkParsePath.h"
#include "src/core/SkClipStackDevice.h"

#include <cstdint>
#include <memory>

namespace sktext {
class GlyphRunList;
}

class SkDevice;
class SkBitmap;
class SkBlender;
class SkClipStack;
class SkData;
class SkImage;
class SkMesh;
class SkPaint;
class SkPath;
class SkRRect;
class SkVertices;
class SkXMLWriter;
struct SkISize;
struct SkPoint;
struct SkRect;
struct SkSamplingOptions;

class SkSVGDevice final : public SkClipStackDevice {
public:
    static sk_sp<SkDevice> Make(const SkISize& size, std::unique_ptr<SkXMLWriter>, uint32_t flags);

    void drawPaint(const SkPaint& paint) override;
    void drawAnnotation(const SkRect& rect, const char key[], SkData* value) override;
    void drawPoints(SkCanvas::PointMode, SkSpan<const SkPoint>, const SkPaint&) override;
    void drawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                       const SkSamplingOptions&, const SkPaint& paint,
                       SkCanvas::SrcRectConstraint constraint) override;
    void drawRect(const SkRect& r, const SkPaint& paint) override;
    void drawOval(const SkRect& oval, const SkPaint& paint) override;
    void drawRRect(const SkRRect& rr, const SkPaint& paint) override;
    void drawPath(const SkPath& path,
                  const SkPaint& paint,
                  bool pathIsMutable = false) override;

    void drawVertices(const SkVertices*, sk_sp<SkBlender>, const SkPaint&, bool) override;
    void drawMesh(const SkMesh&, sk_sp<SkBlender>, const SkPaint&) override;

private:
    SkSVGDevice(const SkISize& size, std::unique_ptr<SkXMLWriter>, uint32_t);
    ~SkSVGDevice() override;

    void onDrawGlyphRunList(SkCanvas*, const sktext::GlyphRunList&, const SkPaint& paint) override;

    struct MxCp;
    void drawBitmapCommon(const MxCp&, const SkBitmap& bm, const SkPaint& paint);

    void syncClipStack(const SkClipStack&);

    SkParsePath::PathEncoding pathEncoding() const;

    class AutoElement;
    class ResourceBucket;

    const std::unique_ptr<SkXMLWriter>    fWriter;
    const std::unique_ptr<ResourceBucket> fResourceBucket;
    const uint32_t                        fFlags;

    struct ClipRec {
        std::unique_ptr<AutoElement> fClipPathElem;
        uint32_t                     fGenID;

        static_assert(::sk_is_trivially_relocatable<decltype(fClipPathElem)>::value);

        using sk_is_trivially_relocatable = std::true_type;
    };

    std::unique_ptr<AutoElement> fRootElement;
    skia_private::TArray<ClipRec> fClipStack;
};

#endif // SkSVGDevice_DEFINED
