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
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTypeTraits.h"
#include "include/utils/SkParsePath.h"
#include "src/core/SkClipStackDevice.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>

namespace sktext {
class GlyphRunList;
}

class SkBaseDevice;
class SkBitmap;
class SkBlender;
class SkClipStack;
class SkData;
class SkImage;
class SkPaint;
class SkPath;
class SkRRect;
class SkVertices;
class SkXMLWriter;
struct SkISize;
struct SkPoint;
struct SkRect;
struct SkSamplingOptions;
#ifdef SK_ENABLE_SKSL
class SkMesh;
#endif

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

    void onDrawGlyphRunList(SkCanvas*,
                            const sktext::GlyphRunList&,
                            const SkPaint& initialPaint,
                            const SkPaint& drawingPaint) override;
    void drawVertices(const SkVertices*, sk_sp<SkBlender>, const SkPaint&, bool) override;
#ifdef SK_ENABLE_SKSL
    void drawMesh(const SkMesh&, sk_sp<SkBlender>, const SkPaint&) override;
#endif
private:
    SkSVGDevice(const SkISize& size, std::unique_ptr<SkXMLWriter>, uint32_t);
    ~SkSVGDevice() override;

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
    SkTArray<ClipRec>            fClipStack;

    using INHERITED = SkClipStackDevice;
};

#endif // SkSVGDevice_DEFINED
