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
#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurfaceProps.h"
#include "src/core/SkDevice.h"
#include "src/core/SkGlyphRunPainter.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkRasterClipStack.h"

class SkImageFilterCache;
class SkMatrix;
class SkPaint;
class SkPath;
class SkPixmap;
class SkRasterHandleAllocator;
class SkRRect;
class SkSurface;
struct SkPoint;

///////////////////////////////////////////////////////////////////////////////
class SkBitmapDevice : public SkBaseDevice {
public:
    /**
     *  Construct a new device with the specified bitmap as its backend. It is
     *  valid for the bitmap to have no pixels associated with it. In that case,
     *  any drawing to this device will have no effect.
     */
    SkBitmapDevice(const SkBitmap& bitmap);

    /**
     * Create a new device along with its requisite pixel memory using
     * default SkSurfaceProps (i.e., kLegacyFontHost_InitType-style).
     * Note: this entry point is slated for removal - no one should call it.
     */
    static SkBitmapDevice* Create(const SkImageInfo& info);

    /**
     *  Construct a new device with the specified bitmap as its backend. It is
     *  valid for the bitmap to have no pixels associated with it. In that case,
     *  any drawing to this device will have no effect.
     */
    SkBitmapDevice(const SkBitmap& bitmap, const SkSurfaceProps& surfaceProps,
                   void* externalHandle, const SkBitmap* coverage);

    static SkBitmapDevice* Create(const SkImageInfo&, const SkSurfaceProps&,
                                  bool trackCoverage,
                                  SkRasterHandleAllocator*);

    static SkBitmapDevice* Create(const SkImageInfo& info, const SkSurfaceProps& props) {
        return Create(info, props, false, nullptr);
    }

    const SkPixmap* accessCoverage() const {
        return fCoverage ? &fCoverage->pixmap() : nullptr;
    }

protected:
    void* getRasterHandle() const override { return fRasterHandle; }

    /** These are called inside the per-device-layer loop for each draw call.
     When these are called, we have already applied any saveLayer operations,
     and are handling any looping from the paint.
     */
    void drawPaint(const SkPaint& paint) override;
    void drawPoints(SkCanvas::PointMode mode, size_t count,
                            const SkPoint[], const SkPaint& paint) override;
    void drawRect(const SkRect& r, const SkPaint& paint) override;
    void drawOval(const SkRect& oval, const SkPaint& paint) override;
    void drawRRect(const SkRRect& rr, const SkPaint& paint) override;

    /**
     *  If pathIsMutable, then the implementation is allowed to cast path to a
     *  non-const pointer and modify it in place (as an optimization). Canvas
     *  may do this to implement helpers such as drawOval, by placing a temp
     *  path on the stack to hold the representation of the oval.
     */
    void drawPath(const SkPath&, const SkPaint&, bool pathIsMutable) override;
    void drawSprite(const SkBitmap&, int x, int y, const SkPaint&) override;

    /**
     *  The default impl. will create a bitmap-shader from the bitmap,
     *  and call drawRect with it.
     */
    void drawBitmapRect(const SkBitmap&, const SkRect*, const SkRect&,
                        const SkPaint&, SkCanvas::SrcRectConstraint) override;

    void drawGlyphRunList(const SkGlyphRunList& glyphRunList) override;
    void drawVertices(const SkVertices*, const SkVertices::Bone bones[], int boneCount, SkBlendMode,
                      const SkPaint& paint) override;
    void drawDevice(SkBaseDevice*, int x, int y, const SkPaint&) override;

    ///////////////////////////////////////////////////////////////////////////

    void drawSpecial(SkSpecialImage*, int x, int y, const SkPaint&,
                     SkImage*, const SkMatrix&) override;
    sk_sp<SkSpecialImage> makeSpecial(const SkBitmap&) override;
    sk_sp<SkSpecialImage> makeSpecial(const SkImage*) override;
    sk_sp<SkSpecialImage> snapSpecial() override;
    void setImmutable() override { fBitmap.setImmutable(); }

    sk_sp<SkSpecialImage> snapBackImage(const SkIRect&) override;

    ///////////////////////////////////////////////////////////////////////////

    bool onReadPixels(const SkPixmap&, int x, int y) override;
    bool onWritePixels(const SkPixmap&, int, int) override;
    bool onPeekPixels(SkPixmap*) override;
    bool onAccessPixels(SkPixmap*) override;

    void onSave() override;
    void onRestore() override;
    void onClipRect(const SkRect& rect, SkClipOp, bool aa) override;
    void onClipRRect(const SkRRect& rrect, SkClipOp, bool aa) override;
    void onClipPath(const SkPath& path, SkClipOp, bool aa) override;
    void onClipRegion(const SkRegion& deviceRgn, SkClipOp) override;
    void onSetDeviceClipRestriction(SkIRect* mutableClipRestriction) override;
    bool onClipIsAA() const override;
    void onAsRgnClip(SkRegion*) const override;
    void validateDevBounds(const SkIRect& r) override;
    ClipType onGetClipType() const override;

    virtual void drawBitmap(const SkBitmap&, const SkMatrix&, const SkRect* dstOrNull,
                            const SkPaint&);

private:
    friend class SkCanvas;
    friend struct DeviceCM; //for setMatrixClip
    friend class SkDraw;
    friend class SkDrawIter;
    friend class SkDrawTiler;
    friend class SkSurface_Raster;

    class BDDraw;

    // used to change the backend's pixels (and possibly config/rowbytes)
    // but cannot change the width/height, so there should be no change to
    // any clip information.
    void replaceBitmapBackendForRasterSurface(const SkBitmap&) override;

    SkBaseDevice* onCreateDevice(const CreateInfo&, const SkPaint*) override;

    sk_sp<SkSurface> makeSurface(const SkImageInfo&, const SkSurfaceProps&) override;

    SkImageFilterCache* getImageFilterCache() override;

    SkBitmap    fBitmap;
    void*       fRasterHandle = nullptr;
    SkRasterClipStack  fRCStack;
    std::unique_ptr<SkBitmap> fCoverage;    // if non-null, will have the same dimensions as fBitmap
    SkGlyphRunListPainter fGlyphPainter;


    typedef SkBaseDevice INHERITED;
};

class SkBitmapDeviceFilteredSurfaceProps {
public:
    SkBitmapDeviceFilteredSurfaceProps(const SkBitmap& bitmap, const SkPaint& paint,
                                       const SkSurfaceProps& surfaceProps)
        : fSurfaceProps((kN32_SkColorType != bitmap.colorType() || !paint.isSrcOver())
                        ? fLazy.init(surfaceProps.flags(), kUnknown_SkPixelGeometry)
                        : &surfaceProps)
    { }

    SkBitmapDeviceFilteredSurfaceProps(const SkBitmapDeviceFilteredSurfaceProps&) = delete;
    SkBitmapDeviceFilteredSurfaceProps& operator=(const SkBitmapDeviceFilteredSurfaceProps&) = delete;
    SkBitmapDeviceFilteredSurfaceProps(SkBitmapDeviceFilteredSurfaceProps&&) = delete;
    SkBitmapDeviceFilteredSurfaceProps& operator=(SkBitmapDeviceFilteredSurfaceProps&&) = delete;

    const SkSurfaceProps& operator()() const { return *fSurfaceProps; }

private:
    SkTLazy<SkSurfaceProps> fLazy;
    SkSurfaceProps const * const fSurfaceProps;
};

#endif // SkBitmapDevice_DEFINED
