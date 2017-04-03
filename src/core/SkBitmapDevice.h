/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapDevice_DEFINED
#define SkBitmapDevice_DEFINED

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkDevice.h"
#include "SkImageInfo.h"
#include "SkPixelRef.h"
#include "SkRasterClip.h"
#include "SkRasterClipStack.h"
#include "SkRect.h"
#include "SkScalar.h"
#include "SkSize.h"
#include "SkSurfaceProps.h"

class SkImageFilterCache;
class SkMatrix;
class SkPaint;
class SkPath;
class SkPixelRef;
class SkPixmap;
class SkRasterHandleAllocator;
class SkRRect;
class SkSurface;
struct SkPoint;

///////////////////////////////////////////////////////////////////////////////
class SK_API SkBitmapDevice : public SkBaseDevice {
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
                   void* externalHandle = nullptr);

    static SkBitmapDevice* Create(const SkImageInfo&, const SkSurfaceProps&,
                                  SkRasterHandleAllocator* = nullptr);

protected:
    bool onShouldDisableLCD(const SkPaint&) const override;
    void* getRasterHandle() const override { return fRasterHandle; }

    /** These are called inside the per-device-layer loop for each draw call.
     When these are called, we have already applied any saveLayer operations,
     and are handling any looping from the paint, and any effects from the
     DrawFilter.
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
     *
     *  If prePathMatrix is not null, it should logically be applied before any
     *  stroking or other effects. If there are no effects on the paint that
     *  affect the geometry/rasterization, then the pre matrix can just be
     *  pre-concated with the current matrix.
     */
    void drawPath(const SkPath&, const SkPaint&, const SkMatrix* prePathMatrix,
                          bool pathIsMutable) override;
    void drawBitmap(const SkBitmap&, const SkMatrix&, const SkPaint&) override;
    void drawSprite(const SkBitmap&, int x, int y, const SkPaint&) override;

    /**
     *  The default impl. will create a bitmap-shader from the bitmap,
     *  and call drawRect with it.
     */
    void drawBitmapRect(const SkBitmap&, const SkRect*, const SkRect&,
                        const SkPaint&, SkCanvas::SrcRectConstraint) override;

    /**
     *  Does not handle text decoration.
     *  Decorations (underline and stike-thru) will be handled by SkCanvas.
     */
    void drawText(const void* text, size_t len, SkScalar x, SkScalar y,
                  const SkPaint&) override;
    void drawPosText(const void* text, size_t len, const SkScalar pos[],
                     int scalarsPerPos, const SkPoint& offset, const SkPaint& paint) override;
    void drawVertices(const SkVertices*, SkBlendMode, const SkPaint&) override;
    void drawDevice(SkBaseDevice*, int x, int y, const SkPaint&) override;

    ///////////////////////////////////////////////////////////////////////////

    void drawSpecial(SkSpecialImage*, int x, int y, const SkPaint&) override;
    sk_sp<SkSpecialImage> makeSpecial(const SkBitmap&) override;
    sk_sp<SkSpecialImage> makeSpecial(const SkImage*) override;
    sk_sp<SkSpecialImage> snapSpecial() override;

    ///////////////////////////////////////////////////////////////////////////

    bool onReadPixels(const SkImageInfo&, void*, size_t, int x, int y) override;
    bool onWritePixels(const SkImageInfo&, const void*, size_t, int, int) override;
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

private:
    friend class SkCanvas;
    friend struct DeviceCM; //for setMatrixClip
    friend class SkDraw;
    friend class SkDrawIter;
    friend class SkDeviceFilteredPaint;
    friend class SkSurface_Raster;
    friend class SkThreadedBMPDevice; // to copy fRCStack

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

    typedef SkBaseDevice INHERITED;
};

#endif // SkBitmapDevice_DEFINED
