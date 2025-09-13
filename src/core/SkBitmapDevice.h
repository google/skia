/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapDevice_DEFINED
#define SkBitmapDevice_DEFINED

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSpan.h"
#include "src/core/SkCPURecorderImpl.h"
#include "src/core/SkDevice.h"
#include "src/core/SkGlyphRunPainter.h"
#include "src/core/SkRasterClipStack.h"

class SkBlender;
class SkImage;
class SkMatrix;
class SkMesh;
class SkPaint;
class SkPath;
class SkPixmap;
class SkRRect;
class SkRasterHandleAllocator;
class SkRecorder;
class SkRegion;
class SkShader;
class SkSpecialImage;
class SkSurface;
class SkSurfaceProps;
class SkVertices;
enum class SkClipOp;
struct SkImageInfo;
struct SkPoint;
struct SkRSXform;
namespace sktext { class GlyphRunList; }

///////////////////////////////////////////////////////////////////////////////
class SkBitmapDevice final : public SkDevice {
public:
    /**
     *  Construct a new device with the specified bitmap as its backend. It is
     *  valid for the bitmap to have no pixels associated with it. In that case,
     *  any drawing to this device will have no effect.
     */
    SkBitmapDevice(const SkBitmap& bitmap);

    /**
     *  Construct a new device with the specified bitmap as its backend. It is
     *  valid for the bitmap to have no pixels associated with it. In that case,
     *  any drawing to this device will have no effect.
     */
    SkBitmapDevice(const SkBitmap& bitmap, const SkSurfaceProps& surfaceProps,
                   void* externalHandle = nullptr);

    SkBitmapDevice(skcpu::RecorderImpl*, const SkBitmap& bitmap);
    SkBitmapDevice(skcpu::RecorderImpl*,
                   const SkBitmap& bitmap,
                   const SkSurfaceProps& surfaceProps,
                   void* externalHandle = nullptr);

    static sk_sp<SkBitmapDevice> Create(const SkImageInfo&, const SkSurfaceProps&,
                                        SkRasterHandleAllocator* = nullptr);

    void drawPaint(const SkPaint& paint) override;
    void drawPoints(SkCanvas::PointMode, SkSpan<const SkPoint>, const SkPaint&) override;
    void drawRect(const SkRect& r, const SkPaint& paint) override;
    void drawOval(const SkRect& oval, const SkPaint& paint) override;
    void drawRRect(const SkRRect& rr, const SkPaint& paint) override;

    void drawPath(const SkPath&, const SkPaint&, bool pathIsMutable) override;

    void drawImageRect(const SkImage*, const SkRect* src, const SkRect& dst,
                       const SkSamplingOptions&, const SkPaint&,
                       SkCanvas::SrcRectConstraint) override;

    void drawVertices(const SkVertices*, sk_sp<SkBlender>, const SkPaint&, bool) override;
    // Implemented in src/sksl/SkBitmapDevice_mesh.cpp
    void drawMesh(const SkMesh&, sk_sp<SkBlender>, const SkPaint&) override;

    void drawAtlas(SkSpan<const SkRSXform>, SkSpan<const SkRect>, SkSpan<const SkColor>,
                   sk_sp<SkBlender>, const SkPaint&) override;

    ///////////////////////////////////////////////////////////////////////////

    void pushClipStack() override;
    void popClipStack() override;
    void clipRect(const SkRect& rect, SkClipOp, bool aa) override;
    void clipRRect(const SkRRect& rrect, SkClipOp, bool aa) override;
    void clipPath(const SkPath& path, SkClipOp, bool aa) override;
    void clipRegion(const SkRegion& deviceRgn, SkClipOp) override;
    void replaceClip(const SkIRect& rect) override;
    bool isClipAntiAliased() const override;
    bool isClipEmpty() const override;
    bool isClipRect() const override;
    bool isClipWideOpen() const override;
    void android_utils_clipAsRgn(SkRegion*) const override;
    SkIRect devClipBounds() const override;

    ///////////////////////////////////////////////////////////////////////////

    void drawSpecial(SkSpecialImage*, const SkMatrix&, const SkSamplingOptions&,
                     const SkPaint&, SkCanvas::SrcRectConstraint) override;

    void drawCoverageMask(const SkSpecialImage*, const SkMatrix&, const SkSamplingOptions&,
                          const SkPaint&) override;

    bool drawBlurredRRect(const SkRRect&, const SkPaint&, float) override;

    sk_sp<SkSpecialImage> snapSpecial(const SkIRect&, bool forceCopy = false) override;

    sk_sp<SkDevice> createDevice(const CreateInfo&, const SkPaint*) override;

    sk_sp<SkSurface> makeSurface(const SkImageInfo&, const SkSurfaceProps&) override;

    void setImmutable() override { fBitmap.setImmutable(); }

    void* getRasterHandle() const override { return fRasterHandle; }

    SkRecorder* baseRecorder() const override { return fRecorder; }

private:
    friend class SkDrawTiler;
    friend class SkSurface_Raster;

    class BDDraw;

    // Used to change the backend's pixels (and possibly config/rowbytes) but cannot change the
    // width/height, so there should be no change to any clip information.
    void replaceBitmapBackendForRasterSurface(const SkBitmap&);

    void onClipShader(sk_sp<SkShader>) override;

    void onDrawGlyphRunList(SkCanvas*, const sktext::GlyphRunList&, const SkPaint& paint) override;

    bool onReadPixels(const SkPixmap&, int x, int y) override;
    bool onWritePixels(const SkPixmap&, int, int) override;
    bool onPeekPixels(SkPixmap*) override;
    bool onAccessPixels(SkPixmap*) override;

    void drawBitmap(const SkBitmap&, const SkMatrix&, const SkRect* dstOrNull,
                    const SkSamplingOptions&, const SkPaint&);

    void* fRasterHandle = nullptr;
    skcpu::RecorderImpl* fRecorder = nullptr;
    SkBitmap fBitmap;
    SkRasterClipStack fRCStack;
    skcpu::GlyphRunListPainter fGlyphPainter;
};

#endif // SkBitmapDevice_DEFINED
