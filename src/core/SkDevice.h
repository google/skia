/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDevice_DEFINED
#define SkDevice_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkRegion.h"
#include "include/core/SkSurfaceProps.h"
#include "include/private/SkNoncopyable.h"

class SkBitmap;
struct SkDrawShadowRec;
class SkGlyphRun;
class SkGlyphRunList;
class SkImageFilterCache;
struct SkIRect;
class SkMatrix;
class SkRasterHandleAllocator;
class SkSpecialImage;

class SkBaseDevice : public SkRefCnt {
public:
    SkBaseDevice(const SkImageInfo&, const SkSurfaceProps&);

    /**
     *  Return ImageInfo for this device. If the canvas is not backed by pixels
     *  (cpu or gpu), then the info's ColorType will be kUnknown_SkColorType.
     */
    const SkImageInfo& imageInfo() const { return fInfo; }

    /**
     *  Return SurfaceProps for this device.
     */
    const SkSurfaceProps& surfaceProps() const {
        return fSurfaceProps;
    }

    /**
     *  Return the bounds of the device in the coordinate space of the root
     *  canvas. The root device will have its top-left at 0,0, but other devices
     *  such as those associated with saveLayer may have a non-zero origin.
     */
    void getGlobalBounds(SkIRect* bounds) const {
        SkASSERT(bounds);
        SkRect localBounds = SkRect::MakeIWH(this->width(), this->height());
        fDeviceToGlobal.mapRect(&localBounds);
        *bounds = localBounds.roundOut();
    }

    SkIRect getGlobalBounds() const {
        SkIRect bounds;
        this->getGlobalBounds(&bounds);
        return bounds;
    }

    /**
     *  Returns the bounding box of the current clip, in this device's
     *  coordinate space. No pixels outside of these bounds will be touched by
     *  draws unless the clip is further modified (at which point this will
     *  return the updated bounds).
     */
    SkIRect devClipBounds() const { return this->onDevClipBounds(); }

    int width() const {
        return this->imageInfo().width();
    }

    int height() const {
        return this->imageInfo().height();
    }

    bool isOpaque() const {
        return this->imageInfo().isOpaque();
    }

    bool writePixels(const SkPixmap&, int x, int y);

    /**
     *  Try to get write-access to the pixels behind the device. If successful, this returns true
     *  and fills-out the pixmap parameter. On success it also bumps the genID of the underlying
     *  bitmap.
     *
     *  On failure, returns false and ignores the pixmap parameter.
     */
    bool accessPixels(SkPixmap* pmap);

    /**
     *  Try to get read-only-access to the pixels behind the device. If successful, this returns
     *  true and fills-out the pixmap parameter.
     *
     *  On failure, returns false and ignores the pixmap parameter.
     */
    bool peekPixels(SkPixmap*);

    /**
     *  Return the device's coordinate space transform: this maps from the device's coordinate space
     *  into the global canvas' space (or root device space). This includes the translation
     *  necessary to account for the device's origin.
     */
    const SkMatrix& deviceToGlobal() const { return fDeviceToGlobal; }
    /**
     *  Return the inverse of getDeviceToGlobal(), mapping from the global canvas' space (or root
     *  device space) into this device's coordinate space.
     */
    const SkMatrix& globalToDevice() const { return fGlobalToDevice; }
    /**
     *  DEPRECATED: This asserts that 'getDeviceToGlobal' is a translation matrix with integer
     *  components. In the future some SkDevices will have more complex device-to-global transforms,
     *  so getDeviceToGlobal() or getRelativeTransform() should be used instead.
     */
    SkIPoint getOrigin() const;
    /**
     * Returns true when this device's pixel grid is axis aligned with the global coordinate space,
     * and any relative translation between the two spaces is in integer pixel units.
     */
    bool isPixelAlignedToGlobal() const;
    /**
     *  Get the transformation from the input device's to this device's coordinate space. This
     *  transform can be used to draw the input device into this device, such that once this device
     *  is drawn to the root device, the net effect will have the input device's content drawn
     *  transformed by the global CTM.
     */
    SkMatrix getRelativeTransform(const SkBaseDevice&) const;

    virtual void* getRasterHandle() const { return nullptr; }

    void save() { this->onSave(); }
    void restore(const SkMatrix& ctm) {
        this->onRestore();
        this->setGlobalCTM(ctm);
    }
    void restoreLocal(const SkMatrix& localToDevice) {
        this->onRestore();
        this->setLocalToDevice(localToDevice);
    }
    void clipRect(const SkRect& rect, SkClipOp op, bool aa) {
        this->onClipRect(rect, op, aa);
    }
    void clipRRect(const SkRRect& rrect, SkClipOp op, bool aa) {
        this->onClipRRect(rrect, op, aa);
    }
    void clipPath(const SkPath& path, SkClipOp op, bool aa) {
        this->onClipPath(path, op, aa);
    }
    void clipRegion(const SkRegion& region, SkClipOp op) {
        this->onClipRegion(region, op);
    }
    void androidFramework_setDeviceClipRestriction(SkIRect* mutableClipRestriction) {
        this->onSetDeviceClipRestriction(mutableClipRestriction);
    }
    bool clipIsWideOpen() const;

    const SkMatrix& localToDevice() const { return fLocalToDevice; }
    void setLocalToDevice(const SkMatrix& localToDevice) {
        fLocalToDevice = localToDevice;
    }
    void setGlobalCTM(const SkMatrix& ctm);
    virtual void validateDevBounds(const SkIRect&) {}

protected:
    enum TileUsage {
        kPossible_TileUsage,    //!< the created device may be drawn tiled
        kNever_TileUsage,       //!< the created device will never be drawn tiled
    };

    struct TextFlags {
        uint32_t    fFlags;     // SkPaint::getFlags()
    };

    virtual void onSave() {}
    virtual void onRestore() {}
    virtual void onClipRect(const SkRect& rect, SkClipOp, bool aa) {}
    virtual void onClipRRect(const SkRRect& rrect, SkClipOp, bool aa) {}
    virtual void onClipPath(const SkPath& path, SkClipOp, bool aa) {}
    virtual void onClipRegion(const SkRegion& deviceRgn, SkClipOp) {}
    virtual void onSetDeviceClipRestriction(SkIRect* mutableClipRestriction) {}
    virtual bool onClipIsAA() const = 0;
    virtual void onAsRgnClip(SkRegion*) const = 0;
    enum class ClipType {
        kEmpty,
        kRect,
        kComplex
    };
    virtual ClipType onGetClipType() const = 0;

    // This should strive to be as tight as possible, ideally not just mapping
    // the global clip bounds by fToGlobal^-1.
    virtual SkIRect onDevClipBounds() const = 0;

    /** These are called inside the per-device-layer loop for each draw call.
     When these are called, we have already applied any saveLayer operations,
     and are handling any looping from the paint.
     */
    virtual void drawPaint(const SkPaint& paint) = 0;
    virtual void drawPoints(SkCanvas::PointMode mode, size_t count,
                            const SkPoint[], const SkPaint& paint) = 0;
    virtual void drawRect(const SkRect& r,
                          const SkPaint& paint) = 0;
    virtual void drawRegion(const SkRegion& r,
                            const SkPaint& paint);
    virtual void drawOval(const SkRect& oval,
                          const SkPaint& paint) = 0;
    /** By the time this is called we know that abs(sweepAngle) is in the range [0, 360). */
    virtual void drawArc(const SkRect& oval, SkScalar startAngle,
                         SkScalar sweepAngle, bool useCenter, const SkPaint& paint);
    virtual void drawRRect(const SkRRect& rr,
                           const SkPaint& paint) = 0;

    // Default impl calls drawPath()
    virtual void drawDRRect(const SkRRect& outer,
                            const SkRRect& inner, const SkPaint&);

    /**
     *  If pathIsMutable, then the implementation is allowed to cast path to a
     *  non-const pointer and modify it in place (as an optimization). Canvas
     *  may do this to implement helpers such as drawOval, by placing a temp
     *  path on the stack to hold the representation of the oval.
     */
    virtual void drawPath(const SkPath& path,
                          const SkPaint& paint,
                          bool pathIsMutable = false) = 0;
    virtual void drawSprite(const SkBitmap& bitmap,
                            int x, int y, const SkPaint& paint) = 0;

    /**
     *  The default impl. will create a bitmap-shader from the bitmap,
     *  and call drawRect with it.
     */
    virtual void drawBitmapRect(const SkBitmap&,
                                const SkRect* srcOrNull, const SkRect& dst,
                                const SkPaint& paint,
                                SkCanvas::SrcRectConstraint) = 0;
    virtual void drawBitmapNine(const SkBitmap&, const SkIRect& center,
                                const SkRect& dst, const SkPaint&);
    virtual void drawBitmapLattice(const SkBitmap&, const SkCanvas::Lattice&,
                                   const SkRect& dst, const SkPaint&);

    virtual void drawImageRect(const SkImage*, const SkRect* src, const SkRect& dst,
                               const SkPaint&, SkCanvas::SrcRectConstraint);
    virtual void drawImageNine(const SkImage*, const SkIRect& center,
                               const SkRect& dst, const SkPaint&);
    virtual void drawImageLattice(const SkImage*, const SkCanvas::Lattice&,
                                  const SkRect& dst, const SkPaint&);

    virtual void drawVertices(const SkVertices*, const SkVertices::Bone bones[], int boneCount,
                              SkBlendMode, const SkPaint&) = 0;
    virtual void drawShadow(const SkPath&, const SkDrawShadowRec&);

    virtual void drawGlyphRunList(const SkGlyphRunList& glyphRunList) = 0;
    // default implementation calls drawVertices
    virtual void drawPatch(const SkPoint cubics[12], const SkColor colors[4],
                           const SkPoint texCoords[4], SkBlendMode, const SkPaint& paint);

    // default implementation calls drawPath
    virtual void drawAtlas(const SkImage* atlas, const SkRSXform[], const SkRect[],
                           const SkColor[], int count, SkBlendMode, const SkPaint&);

    virtual void drawAnnotation(const SkRect&, const char[], SkData*) {}

    // Default impl always calls drawRect() with a solid-color paint, setting it to anti-aliased
    // only when all edge flags are set. If there's a clip region, it draws that using drawPath,
    // or uses clipPath().
    virtual void drawEdgeAAQuad(const SkRect& rect, const SkPoint clip[4],
                                SkCanvas::QuadAAFlags aaFlags, const SkColor4f& color,
                                SkBlendMode mode);
    // Default impl uses drawImageRect per entry, being anti-aliased only when an entry's edge flags
    // are all set. If there's a clip region, it will be applied using clipPath().
    virtual void drawEdgeAAImageSet(const SkCanvas::ImageSetEntry[], int count,
                                    const SkPoint dstClips[], const SkMatrix preViewMatrices[],
                                    const SkPaint& paint, SkCanvas::SrcRectConstraint);

    /** The SkDevice passed will be an SkDevice which was returned by a call to
        onCreateDevice on this device with kNeverTile_TileExpectation.
     */
    virtual void drawDevice(SkBaseDevice*, int x, int y, const SkPaint&) = 0;

    void drawGlyphRunRSXform(const SkFont&, const SkGlyphID[], const SkRSXform[], int count,
                             SkPoint origin, const SkPaint& paint);

    virtual void drawDrawable(SkDrawable*, const SkMatrix*, SkCanvas*);

    virtual void drawSpecial(SkSpecialImage*, int x, int y, const SkPaint&,
                             SkImage* clipImage, const SkMatrix& clipMatrix);
    virtual sk_sp<SkSpecialImage> makeSpecial(const SkBitmap&);
    virtual sk_sp<SkSpecialImage> makeSpecial(const SkImage*);
    // Get a view of the entire device's current contents as an image.
    sk_sp<SkSpecialImage> snapSpecial();
    // Snap the 'subset' contents from this device, possibly as a read-only view. If 'forceCopy'
    // is true then the returned image's pixels must not be affected by subsequent draws into the
    // device. When 'forceCopy' is false, the image can be a view into the device's pixels
    // (avoiding a copy for performance, at the expense of safety). Default returns null.
    virtual sk_sp<SkSpecialImage> snapSpecial(const SkIRect& subset, bool forceCopy = false);

    virtual void setImmutable() {}

    bool readPixels(const SkPixmap&, int x, int y);

    ///////////////////////////////////////////////////////////////////////////

    virtual GrContext* context() const { return nullptr; }

    virtual sk_sp<SkSurface> makeSurface(const SkImageInfo&, const SkSurfaceProps&);
    virtual bool onPeekPixels(SkPixmap*) { return false; }

    /**
     *  The caller is responsible for "pre-clipping" the dst. The impl can assume that the dst
     *  image at the specified x,y offset will fit within the device's bounds.
     *
     *  This is explicitly asserted in readPixels(), the public way to call this.
     */
    virtual bool onReadPixels(const SkPixmap&, int x, int y);

    /**
     *  The caller is responsible for "pre-clipping" the src. The impl can assume that the src
     *  image at the specified x,y offset will fit within the device's bounds.
     *
     *  This is explicitly asserted in writePixelsDirect(), the public way to call this.
     */
    virtual bool onWritePixels(const SkPixmap&, int x, int y);

    virtual bool onAccessPixels(SkPixmap*) { return false; }

    struct CreateInfo {
        static SkPixelGeometry AdjustGeometry(TileUsage, SkPixelGeometry);

        // The constructor may change the pixel geometry based on other parameters.
        CreateInfo(const SkImageInfo& info,
                   TileUsage tileUsage,
                   SkPixelGeometry geo,
                   bool trackCoverage,
                   SkRasterHandleAllocator* allocator)
            : fInfo(info)
            , fTileUsage(tileUsage)
            , fPixelGeometry(AdjustGeometry(tileUsage, geo))
            , fTrackCoverage(trackCoverage)
            , fAllocator(allocator)
        {}

        const SkImageInfo       fInfo;
        const TileUsage         fTileUsage;
        const SkPixelGeometry   fPixelGeometry;
        const bool              fTrackCoverage = false;
        SkRasterHandleAllocator* fAllocator = nullptr;
    };

    /**
     *  Create a new device based on CreateInfo. If the paint is not null, then it represents a
     *  preview of how the new device will be composed with its creator device (this).
     *
     *  The subclass may be handed this device in drawDevice(), so it must always return
     *  a device that it knows how to draw, and that it knows how to identify if it is not of the
     *  same subclass (since drawDevice is passed a SkBaseDevice*). If the subclass cannot fulfill
     *  that contract (e.g. PDF cannot support some settings on the paint) it should return NULL,
     *  and the caller may then decide to explicitly create a bitmapdevice, knowing that later
     *  it could not call drawDevice with it (but it could call drawSprite or drawBitmap).
     */
    virtual SkBaseDevice* onCreateDevice(const CreateInfo&, const SkPaint*) {
        return nullptr;
    }

    // A helper function used by derived classes to log the scale factor of a bitmap or image draw.
    static void LogDrawScaleFactor(const SkMatrix& view, const SkMatrix& srcToDst, SkFilterQuality);

private:
    friend class SkAndroidFrameworkUtils;
    friend class SkCanvas;
    friend struct DeviceCM; //for setMatrixClip
    friend class SkDraw;
    friend class SkDrawIter;
    friend class SkSurface_Raster;
    friend class DeviceTestingAccess;

    // Temporarily friend the SkGlyphRunBuilder until drawPosText is gone.
    friend class SkGlyphRun;
    friend class SkGlyphRunList;
    friend class SkGlyphRunBuilder;

    // used to change the backend's pixels (and possibly config/rowbytes)
    // but cannot change the width/height, so there should be no change to
    // any clip information.
    // TODO: move to SkBitmapDevice
    virtual void replaceBitmapBackendForRasterSurface(const SkBitmap&) {}

    virtual bool forceConservativeRasterClip() const { return false; }

    /**
     * Don't call this!
     */
    virtual GrRenderTargetContext* accessRenderTargetContext() { return nullptr; }

    // Configure the device's coordinate spaces, specifying both how its device image maps back to
    // the global space (via 'deviceToGlobal') and the initial CTM of the device (via
    // 'localToDevice', i.e. what geometry drawn into this device will be transformed with).
    //
    // (deviceOriginX, deviceOriginY) represents the top left corner of the device-space content
    // that maps to the (0, 0) pixel of its backing buffer. This point is in device space. The final
    // device-to-global matrix stored by the SkDevice will include a pre-translation by
    // T(deviceOriginX, deviceOriginY), and the final local-to-device matrix will have a
    // post-translation of T(-deviceOriginX, -deviceOriginY).
    void setDeviceCoordinateSystem(const SkMatrix& deviceToGlobal, const SkMatrix& localToDevice,
                                   int deviceOriginX, int deviceOriginY);
    // Convenience to configure the device to be axis-aligned with the root canvas, but with a
    // unique origin.
    void setOrigin(const SkMatrix& globalCTM, int x, int y) {
        this->setDeviceCoordinateSystem(SkMatrix::I(), globalCTM, x, y);
    }

    /** Causes any deferred drawing to the device to be completed.
     */
    virtual void flush() {}

    virtual SkImageFilterCache* getImageFilterCache() { return nullptr; }

    friend class SkNoPixelsDevice;
    friend class SkBitmapDevice;
    void privateResize(int w, int h) {
        *const_cast<SkImageInfo*>(&fInfo) = fInfo.makeWH(w, h);
    }

    const SkImageInfo    fInfo;
    const SkSurfaceProps fSurfaceProps;
    // fDeviceToGlobal and fGlobalToDevice are inverses of each other; there are never that many
    // SkDevices, so pay the memory cost to avoid recalculating the inverse.
    SkMatrix             fDeviceToGlobal;
    SkMatrix             fGlobalToDevice;
    // This is the device CTM, not the global CTM. This transform maps from local space to the
    // device's coordinate space; fDeviceToGlobal * fLocalToDevice will match the canvas' CTM.
    SkMatrix             fLocalToDevice;

    typedef SkRefCnt INHERITED;
};

class SkNoPixelsDevice : public SkBaseDevice {
public:
    SkNoPixelsDevice(const SkIRect& bounds, const SkSurfaceProps& props,
                     sk_sp<SkColorSpace> colorSpace = nullptr)
            : SkBaseDevice(SkImageInfo::Make(bounds.size(), kUnknown_SkColorType,
                                             kUnknown_SkAlphaType, std::move(colorSpace)),
                           props) {
        // this fails if we enable this assert: DiscardableImageMapTest.GetDiscardableImagesInRectMaxImage
        //SkASSERT(bounds.width() >= 0 && bounds.height() >= 0);

        this->setOrigin(SkMatrix::I(), bounds.left(), bounds.top());
    }

    void resetForNextPicture(const SkIRect& bounds) {
        //SkASSERT(bounds.width() >= 0 && bounds.height() >= 0);
        this->privateResize(bounds.width(), bounds.height());
        this->setOrigin(SkMatrix::I(), bounds.left(), bounds.top());
    }

protected:
    // We don't track the clip at all (for performance), but we have to respond to some queries.
    // We pretend to be wide-open. We could pretend to always be empty, but that *seems* worse.
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
        return ClipType::kRect;
    }
    SkIRect onDevClipBounds() const override {
        return SkIRect::MakeWH(this->width(), this->height());
    }

    void drawPaint(const SkPaint& paint) override {}
    void drawPoints(SkCanvas::PointMode, size_t, const SkPoint[], const SkPaint&) override {}
    void drawRect(const SkRect&, const SkPaint&) override {}
    void drawOval(const SkRect&, const SkPaint&) override {}
    void drawRRect(const SkRRect&, const SkPaint&) override {}
    void drawPath(const SkPath&, const SkPaint&, bool) override {}
    void drawSprite(const SkBitmap&, int, int, const SkPaint&) override {}
    void drawBitmapRect(const SkBitmap&, const SkRect*, const SkRect&, const SkPaint&,
                        SkCanvas::SrcRectConstraint) override {}
    void drawDevice(SkBaseDevice*, int, int, const SkPaint&) override {}
    void drawGlyphRunList(const SkGlyphRunList& glyphRunList) override {}
    void drawVertices(const SkVertices*, const SkVertices::Bone[], int, SkBlendMode,
                      const SkPaint&) override {}

private:
    typedef SkBaseDevice INHERITED;
};

class SkAutoDeviceTransformRestore : SkNoncopyable {
public:
    SkAutoDeviceTransformRestore(SkBaseDevice* device, const SkMatrix& localToDevice)
        : fDevice(device)
        , fPrevLocalToDevice(device->localToDevice())
    {
        fDevice->setLocalToDevice(localToDevice);
    }
    ~SkAutoDeviceTransformRestore() {
        fDevice->setLocalToDevice(fPrevLocalToDevice);
    }

private:
    SkBaseDevice*   fDevice;
    const SkMatrix  fPrevLocalToDevice;
};

#endif
