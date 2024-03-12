/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDevice_DEFINED
#define SkDevice_DEFINED

#include "include/core/SkBlender.h"  // IWYU pragma: keep
#include "include/core/SkCanvas.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkRegion.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurfaceProps.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkNoncopyable.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkMatrixPriv.h"
#include "src/shaders/SkShaderBase.h"

#include <cstddef>
#include <cstdint>
#include <utility>

class SkBitmap;
class SkColorSpace;
class SkMesh;
struct SkDrawShadowRec;
class SkImageFilter;
class SkRasterHandleAllocator;
class SkSpecialImage;
class GrRecordingContext;
class SkData;
class SkDrawable;
class SkImage;
class SkPaint;
class SkPath;
class SkPixmap;
class SkRRect;
class SkSurface;
class SkVertices;
enum SkColorType : int;
enum class SkBlendMode;
enum class SkScalerContextFlags : uint32_t;
struct SkRSXform;

namespace sktext {
class GlyphRunList;
}

namespace skif {
class Backend;
class Mapping;
}
namespace skgpu::ganesh {
class Device;
}
namespace skgpu::graphite {
class Device;
class Recorder;
}
namespace sktext::gpu {
class SDFTControl;
class Slug;
}

struct SkStrikeDeviceInfo {
    const SkSurfaceProps fSurfaceProps;
    const SkScalerContextFlags fScalerContextFlags;
    // This is a pointer so this can be compiled without SK_GPU_SUPPORT.
    const sktext::gpu::SDFTControl* const fSDFTControl;
};

/**
 * SkDevice is the internal API and implementation that SkCanvas will use to perform rendering and
 * implement the saveLayer abstraction. A device wraps some pixel allocation (for non-document based
 * devices) or wraps some other container that stores rendering operations. The drawing operations
 * perform equivalently to their corresponding functions in SkCanvas except that the canvas is
 * responsible for all SkImageFilters. An image filter is applied by automatically creating a layer,
 * drawing the filter-less paint into the layer, and then evaluating the filter on the layer's
 * image.
 *
 * Each layer in an SkCanvas stack is represented by an SkDevice instance that was created by the
 * parent SkDevice (up to the canvas's base device). In most cases these devices will be pixel
 * aligned with one another but may differ in size based on the known extent of the active clip. In
 * complex image filtering scenarios, they may not be axis aligned, although the effective pixel
 * size should remain approximately equal across all devices in a canvas.
 *
 * While SkCanvas manages a single stack of layers and canvas transforms, SkDevice does not have a
 * stack of transforms. Instead, it has a single active transform that is modified as needed by
 * SkCanvas. However, SkDevices are the means by which SkCanvas manages the clip stack because each
 * layer's clip stack starts anew (although the layer's results are then clipped by its parent's
 * stack when it is restored).
 */
class SkDevice : public SkRefCnt {
public:
    SkDevice(const SkImageInfo&, const SkSurfaceProps&);

    // -- Surface properties and metadata

    /**
     *  Return ImageInfo for this device. If the canvas is not backed by pixels
     *  (cpu or gpu), then the info's ColorType will be kUnknown_SkColorType.
     */
    const SkImageInfo& imageInfo() const { return fInfo; }

    int width() const { return this->imageInfo().width(); }
    int height() const { return this->imageInfo().height(); }

    bool isOpaque() const { return this->imageInfo().isOpaque(); }

    // NOTE: Image dimensions as a rect, *not* the current restricted clip bounds.
    SkIRect bounds() const { return SkIRect::MakeWH(this->width(), this->height()); }
    SkISize size() const { return this->imageInfo().dimensions(); }

    /**
     *  Return SurfaceProps for this device.
     */
    const SkSurfaceProps& surfaceProps() const {
        return fSurfaceProps;
    }

    SkScalerContextFlags scalerContextFlags() const;

    virtual SkStrikeDeviceInfo strikeDeviceInfo() const {
        return {fSurfaceProps, this->scalerContextFlags(), nullptr};
    }

    // -- Direct pixel manipulation

    /**
     *  Write the pixels in 'src' into this Device at the specified x,y offset. The caller is
     *  responsible for "pre-clipping" the src.
     */
    bool writePixels(const SkPixmap& src, int x, int y) { return this->onWritePixels(src, x, y); }

    /**
     *  Read pixels from this Device at the specified x,y offset into dst. The caller is
     *  responsible for "pre-clipping" the dst
     */
    bool readPixels(const SkPixmap& dst, int x, int y) { return this->onReadPixels(dst, x, y); }

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


    // -- Device's transform (both current transform affecting draws, and its fixed global mapping)

    /**
     *  Returns the transformation that maps from the local space to the device's coordinate space.
     */
    const SkM44& localToDevice44() const { return fLocalToDevice; }
    const SkMatrix& localToDevice() const { return fLocalToDevice33; }

    /**
     *  Return the device's coordinate space transform: this maps from the device's coordinate space
     *  into the global canvas' space (or root device space). This includes the translation
     *  necessary to account for the device's origin.
     */
    const SkM44& deviceToGlobal() const { return fDeviceToGlobal; }
    /**
     *  Return the inverse of getDeviceToGlobal(), mapping from the global canvas' space (or root
     *  device space) into this device's coordinate space.
     */
    const SkM44& globalToDevice() const { return fGlobalToDevice; }
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
     * Get the transformation from this device's coordinate system to the provided device space.
     * This transform can be used to draw this device into the provided device, such that once
     * that device is drawn to the root device, the net effect will be that this device's contents
     * have been transformed by the global CTM.
     */
    SkMatrix getRelativeTransform(const SkDevice&) const;

    void setLocalToDevice(const SkM44& localToDevice) {
        fLocalToDevice = localToDevice;
        fLocalToDevice33 = fLocalToDevice.asM33();
        fLocalToDeviceDirty = true;
    }
    void setGlobalCTM(const SkM44& ctm);

    // -- Device's clip bounds and stack manipulation

    /**
     *  Return the bounds of the device in the coordinate space of the root canvas. The root device
     *  will have its top-left at 0,0, but other devices such as those associated with saveLayer may
     *  have a non-zero origin.
     */
    void getGlobalBounds(SkIRect* bounds) const {
        SkASSERT(bounds);
        *bounds = SkMatrixPriv::MapRect(fDeviceToGlobal, SkRect::Make(this->bounds())).roundOut();
    }

    SkIRect getGlobalBounds() const {
        SkIRect bounds;
        this->getGlobalBounds(&bounds);
        return bounds;
    }

    /**
     *  Returns the bounding box of the current clip, in this device's coordinate space. No pixels
     *  outside of these bounds will be touched by draws unless the clip is further modified (at
     *  which point this will return the updated bounds).
     */
    virtual SkIRect devClipBounds() const = 0;

    virtual void pushClipStack() = 0;
    virtual void popClipStack() = 0;

    virtual void clipRect(const SkRect& rect, SkClipOp op, bool aa) = 0;
    virtual void clipRRect(const SkRRect& rrect, SkClipOp op, bool aa) = 0;
    virtual void clipPath(const SkPath& path, SkClipOp op, bool aa) = 0;
    virtual void clipRegion(const SkRegion& region, SkClipOp op) = 0;

    void clipShader(sk_sp<SkShader> sh, SkClipOp op) {
        sh = as_SB(sh)->makeWithCTM(this->localToDevice());
        if (op == SkClipOp::kDifference) {
            sh = as_SB(sh)->makeInvertAlpha();
        }
        this->onClipShader(std::move(sh));
    }

    virtual void replaceClip(const SkIRect& rect) = 0;

    virtual bool isClipAntiAliased() const = 0;
    virtual bool isClipEmpty() const = 0;
    virtual bool isClipRect() const = 0;
    virtual bool isClipWideOpen() const = 0;

    virtual void android_utils_clipAsRgn(SkRegion*) const = 0;
    virtual bool android_utils_clipWithStencil() { return false; }

    // -- Device reflection

    // TEMPORARY: Whether or not SkCanvas should use an layer and image filters to simulate
    // mask filters and then draw the filtered mask using drawCoverageMask. Unlike regular
    // layers, the color type passed to SkDevice::createDevice() will always be an alpha-only
    // color type. Eventually this will be the only way that mask filters are handled (barring
    // dedicated fast-paths for blurs on [r]rects and text).
    virtual bool useDrawCoverageMaskForMaskFilters() const { return false; }

    // SkCanvas uses NoPixelsDevice when onCreateDevice fails; but then it needs to be able to
    // inspect a layer's device to know if calling drawDevice() later is allowed.
    virtual bool isNoPixelsDevice() const { return false; }

    virtual void* getRasterHandle() const { return nullptr; }

    virtual GrRecordingContext* recordingContext() const { return nullptr; }
    virtual skgpu::graphite::Recorder* recorder() const { return nullptr; }

    virtual skgpu::ganesh::Device* asGaneshDevice() { return nullptr; }
    virtual skgpu::graphite::Device* asGraphiteDevice() { return nullptr; }

    // Marking an SkDevice immutable declares the intent that rendering to the device is
    // complete, allowing it to be sampled as an image without requiring a copy. Drawing
    // operations may not function and may assert if invoked after setImmutable() is called.
    virtual void setImmutable() {}

    virtual sk_sp<SkSurface> makeSurface(const SkImageInfo&, const SkSurfaceProps&);

    struct CreateInfo {
        CreateInfo(const SkImageInfo& info,
                   SkPixelGeometry geo,
                   SkRasterHandleAllocator* allocator)
            : fInfo(info)
            , fPixelGeometry(geo)
            , fAllocator(allocator)
        {}

        const SkImageInfo        fInfo;
        const SkPixelGeometry    fPixelGeometry;
        SkRasterHandleAllocator* fAllocator = nullptr;
    };

    /**
     *  Create a new device based on CreateInfo. If the paint is not null, then it represents a
     *  preview of how the new device will be composed with its creator device (this).
     *
     *  The subclass may be handed this device in drawDevice(), so it must always return a device
     *  that it knows how to draw, and that it knows how to identify if it is not of the same
     *  subclass (since drawDevice is passed a SkDevice*). If the subclass cannot fulfill that
     *  contract (e.g. PDF cannot support some settings on the paint) it should return NULL, and the
     *  caller may then decide to explicitly create a bitmapdevice, knowing that later it could not
     *  call drawDevice with it (but it could call drawSprite or drawBitmap).
     */
    virtual sk_sp<SkDevice> createDevice(const CreateInfo&, const SkPaint*) { return nullptr; }

    // -- Drawing routines (called after saveLayers and imagefilter operations are applied)

    // Ensure that non-RSXForm runs are passed to onDrawGlyphRunList.
    void drawGlyphRunList(SkCanvas*,
                          const sktext::GlyphRunList& glyphRunList,
                          const SkPaint& paint);
    // Slug handling routines.
    virtual sk_sp<sktext::gpu::Slug> convertGlyphRunListToSlug(
            const sktext::GlyphRunList& glyphRunList, const SkPaint& paint);
    virtual void drawSlug(SkCanvas*, const sktext::gpu::Slug* slug, const SkPaint& paint);

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

    virtual void drawImageRect(const SkImage*, const SkRect* src, const SkRect& dst,
                               const SkSamplingOptions&, const SkPaint&,
                               SkCanvas::SrcRectConstraint) = 0;
    // Return true if canvas calls to drawImage or drawImageRect should try to
    // be drawn in a tiled way.
    virtual bool shouldDrawAsTiledImageRect() const { return false; }
    virtual bool drawAsTiledImageRect(SkCanvas*,
                                      const SkImage*,
                                      const SkRect* src,
                                      const SkRect& dst,
                                      const SkSamplingOptions&,
                                      const SkPaint&,
                                      SkCanvas::SrcRectConstraint) { return false; }

    virtual void drawImageLattice(const SkImage*, const SkCanvas::Lattice&,
                                  const SkRect& dst, SkFilterMode, const SkPaint&);

    /**
     * If skipColorXform is true, then the implementation should assume that the provided
     * vertex colors are already in the destination color space.
     */
    virtual void drawVertices(const SkVertices*,
                              sk_sp<SkBlender>,
                              const SkPaint&,
                              bool skipColorXform = false) = 0;
    virtual void drawMesh(const SkMesh& mesh, sk_sp<SkBlender>, const SkPaint&) = 0;
    virtual void drawShadow(const SkPath&, const SkDrawShadowRec&);

    // default implementation calls drawVertices
    virtual void drawPatch(const SkPoint cubics[12], const SkColor colors[4],
                           const SkPoint texCoords[4], sk_sp<SkBlender>, const SkPaint& paint);

    // default implementation calls drawVertices
    virtual void drawAtlas(const SkRSXform[], const SkRect[], const SkColor[], int count,
                           sk_sp<SkBlender>, const SkPaint&);

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
                                    const SkSamplingOptions&, const SkPaint&,
                                    SkCanvas::SrcRectConstraint);

    virtual void drawDrawable(SkCanvas*, SkDrawable*, const SkMatrix*);

    // -- "Special" drawing and image routines

    // Snap the 'subset' contents from this device, possibly as a read-only view. If 'forceCopy'
    // is true then the returned image's pixels must not be affected by subsequent draws into the
    // device. When 'forceCopy' is false, the image can be a view into the device's pixels
    // (avoiding a copy for performance, at the expense of safety). Default returns null.
    virtual sk_sp<SkSpecialImage> snapSpecial(const SkIRect& subset, bool forceCopy = false);
    // Can return null if unable to perform scaling as part of the copy, even if snapSpecial() w/o
    // scaling would succeed.
    virtual sk_sp<SkSpecialImage> snapSpecialScaled(const SkIRect& subset, const SkISize& dstDims);
    // Get a view of the entire device's current contents as an image.
    sk_sp<SkSpecialImage> snapSpecial();

    /**
     * The SkDevice passed will be an SkDevice which was returned by a call to
     * onCreateDevice on this device with kNeverTile_TileExpectation.
     *
     * The default implementation calls snapSpecial() and drawSpecial() with the relative transform
     * from the input device to this device. The provided SkPaint cannot have a mask filter or
     * image filter, and any shader is ignored.
     */
    virtual void drawDevice(SkDevice*, const SkSamplingOptions&, const SkPaint&);

    /**
     * Draw the special image's subset to this device, subject to the given matrix transform instead
     * of the device's current local to device matrix.
     *
     * If 'constraint' is kFast, the rendered geometry of the image still reflects the extent of
     * the SkSpecialImage's subset, but it's assumed that the pixel data beyond the subset is valid
     * (e.g. SkSpecialImage::makeSubset() was called to crop a larger image).
     */
    virtual void drawSpecial(SkSpecialImage*, const SkMatrix& localToDevice,
                             const SkSamplingOptions&, const SkPaint&,
                             SkCanvas::SrcRectConstraint constraint =
                                    SkCanvas::kStrict_SrcRectConstraint);

    /**
     * Draw the special image's subset to this device, treating its alpha channel as coverage for
     * the draw and ignoring any RGB channels that might be present. This will be drawn using the
     * provided matrix transform instead of the device's current local to device matrix.
     *
     * Coverage values beyond the image's subset are treated as 0 (i.e. kDecal tiling). Color values
     * before coverage are determined as normal by the SkPaint, ignoring style, path effects,
     * mask filters and image filters. The local coords of any SkShader on the paint should be
     * relative to the SkDevice's current matrix (i.e. 'maskToDevice' determines how the coverage
     * mask aligns with device-space, but otherwise shading proceeds like other draws).
    */
    virtual void drawCoverageMask(const SkSpecialImage*, const SkMatrix& maskToDevice,
                                  const SkSamplingOptions&, const SkPaint&);

    /**
     * Evaluate 'filter' and draw the final output into this device using 'paint'. The 'mapping'
     * defines the parameter-to-layer space transform used to evaluate the image filter on 'src',
     * and the layer-to-device space transform that is used to draw the result into this device.
     * Since 'mapping' fully specifies the transform, this draw function ignores the current
     * local-to-device matrix (i.e. just like drawSpecial and drawDevice).
     *
     * The final paint must not have an image filter or mask filter set on it; a shader is ignored.
     * The provided color type will be used for any intermediate surfaces that need to be created as
     * part of filter evaluation. It does not have to be src's color type or this Device's type.
     */
    void drawFilteredImage(const skif::Mapping& mapping, SkSpecialImage* src, SkColorType ct,
                           const SkImageFilter*, const SkSamplingOptions&, const SkPaint&);

protected:
    // DEPRECATED: Can be deleted once SkCanvas::onDrawImage() uses skif::FilterResult so don't
    // bother re-arranging.
    virtual sk_sp<SkSpecialImage> makeSpecial(const SkBitmap&);
    virtual sk_sp<SkSpecialImage> makeSpecial(const SkImage*);

    // Configure the device's coordinate spaces, specifying both how its device image maps back to
    // the global space (via 'deviceToGlobal') and the initial CTM of the device (via
    // 'localToDevice', i.e. what geometry drawn into this device will be transformed with).
    //
    // (bufferOriginX, bufferOriginY) defines where the (0,0) pixel the device's backing buffer
    // is anchored in the device space. The final device-to-global matrix stored by the SkDevice
    // will include a pre-translation by T(deviceOriginX, deviceOriginY), and the final
    // local-to-device matrix will have a post-translation of T(-deviceOriginX, -deviceOriginY).
    void setDeviceCoordinateSystem(const SkM44& deviceToGlobal,
                                   const SkM44& globalToDevice,
                                   const SkM44& localToDevice,
                                   int bufferOriginX,
                                   int bufferOriginY);
    // Convenience to configure the device to be axis-aligned with the root canvas, but with a
    // unique origin.
    void setOrigin(const SkM44& globalCTM, int x, int y) {
        this->setDeviceCoordinateSystem(SkM44(), SkM44(), globalCTM, x, y);
    }

    // Returns whether or not localToDevice() has changed since the last call to this function.
    bool checkLocalToDeviceDirty() {
        bool wasDirty = fLocalToDeviceDirty;
        fLocalToDeviceDirty = false;
        return wasDirty;
    }

private:
    friend class SkCanvas; // for setOrigin/setDeviceCoordinateSystem
    friend class DeviceTestingAccess;

    // Defaults to a CPU image filtering backend.
    virtual sk_sp<skif::Backend> createImageFilteringBackend(const SkSurfaceProps& surfaceProps,
                                                             SkColorType colorType) const;

    // Implementations can assume that the device from (x,y) to (w,h) will fit within dst.
    virtual bool onReadPixels(const SkPixmap&, int x, int y) { return false; }

    // Implementations can assume that the src image placed at 'x,y' will fit within the device.
    virtual bool onWritePixels(const SkPixmap&, int x, int y) { return false; }

    virtual bool onAccessPixels(SkPixmap*) { return false; }

    virtual bool onPeekPixels(SkPixmap*) { return false; }

    virtual void onClipShader(sk_sp<SkShader>) = 0;

    // Only called with glyphRunLists that do not contain RSXForm.
    virtual void onDrawGlyphRunList(SkCanvas*,
                                    const sktext::GlyphRunList&,
                                    const SkPaint& paint) = 0;

    void simplifyGlyphRunRSXFormAndRedraw(SkCanvas*,
                                          const sktext::GlyphRunList&,
                                          const SkPaint& paint);

    const SkImageInfo    fInfo;
    const SkSurfaceProps fSurfaceProps;
    SkM44 fLocalToDevice;
    // fDeviceToGlobal and fGlobalToDevice are inverses of each other; there are never that many
    // SkDevices, so pay the memory cost to avoid recalculating the inverse.
    SkM44 fDeviceToGlobal;
    SkM44 fGlobalToDevice;

    // fLocalToDevice but as a 3x3.
    SkMatrix fLocalToDevice33;

    // fLocalToDevice is the device CTM, not the global CTM.
    // It maps from local space to the device's coordinate space.
    // fDeviceToGlobal * fLocalToDevice will match the canvas' CTM.
    //
    // setGlobalCTM and setLocalToDevice are intentionally not virtual for performance reasons.
    // However, track a dirty bit for subclasses that want to defer local-to-device dependent
    // calculations until needed for a clip or draw.
    bool fLocalToDeviceDirty = true;
};

class SkNoPixelsDevice : public SkDevice {
public:
    SkNoPixelsDevice(const SkIRect& bounds, const SkSurfaceProps& props);
    SkNoPixelsDevice(const SkIRect& bounds, const SkSurfaceProps& props,
                     sk_sp<SkColorSpace> colorSpace);

    // Returns false if the device could not be reset; this should only be called on a root device.
    bool resetForNextPicture(const SkIRect& bounds);

    // SkNoPixelsDevice tracks the clip conservatively in order to respond to some queries as
    // accurately as possible while emphasizing performance
    void pushClipStack() override;
    void popClipStack() override;
    void clipRect(const SkRect& rect, SkClipOp op, bool aa) override;
    void clipRRect(const SkRRect& rrect, SkClipOp op, bool aa) override;
    void clipPath(const SkPath& path, SkClipOp op, bool aa) override;
    void clipRegion(const SkRegion& globalRgn, SkClipOp op) override;
    void replaceClip(const SkIRect& rect) override;
    bool isClipAntiAliased() const override { return this->clip().fIsAA; }
    bool isClipEmpty() const override { return this->devClipBounds().isEmpty(); }
    bool isClipRect() const override { return this->clip().fIsRect && !this->isClipEmpty(); }
    bool isClipWideOpen() const override {
        return this->clip().fIsRect &&
               this->devClipBounds() == this->bounds();
    }
    void android_utils_clipAsRgn(SkRegion* rgn) const override {
        rgn->setRect(this->devClipBounds());
    }
    SkIRect devClipBounds() const override { return this->clip().fClipBounds; }

protected:

    void drawPaint(const SkPaint& paint) override {}
    void drawPoints(SkCanvas::PointMode, size_t, const SkPoint[], const SkPaint&) override {}
    void drawImageRect(const SkImage*, const SkRect*, const SkRect&,
                       const SkSamplingOptions&, const SkPaint&,
                       SkCanvas::SrcRectConstraint) override {}
    void drawRect(const SkRect&, const SkPaint&) override {}
    void drawOval(const SkRect&, const SkPaint&) override {}
    void drawRRect(const SkRRect&, const SkPaint&) override {}
    void drawPath(const SkPath&, const SkPaint&, bool) override {}
    void drawDevice(SkDevice*, const SkSamplingOptions&, const SkPaint&) override {}
    void drawVertices(const SkVertices*, sk_sp<SkBlender>, const SkPaint&, bool) override {}
    void drawMesh(const SkMesh&, sk_sp<SkBlender>, const SkPaint&) override {}

    void drawSlug(SkCanvas*, const sktext::gpu::Slug*, const SkPaint&) override {}
    void onDrawGlyphRunList(SkCanvas*, const sktext::GlyphRunList&, const SkPaint&) override {}

    bool isNoPixelsDevice() const override { return true; }

private:
    struct ClipState {
        SkIRect fClipBounds;
        int fDeferredSaveCount;
        bool fIsAA;
        bool fIsRect;

        ClipState(const SkIRect& bounds, bool isAA, bool isRect)
                : fClipBounds(bounds)
                , fDeferredSaveCount(0)
                , fIsAA(isAA)
                , fIsRect(isRect) {}

        void op(SkClipOp op, const SkM44& transform, const SkRect& bounds,
                bool isAA, bool fillsBounds);
    };

    void onClipShader(sk_sp<SkShader> shader) override;

    const ClipState& clip() const { return fClipStack.back(); }
    ClipState& writableClip();

    skia_private::STArray<4, ClipState> fClipStack;
};

class SkAutoDeviceTransformRestore : SkNoncopyable {
public:
    SkAutoDeviceTransformRestore(SkDevice* device, const SkMatrix& localToDevice)
        : fDevice(device)
        , fPrevLocalToDevice(device->localToDevice())
    {
        fDevice->setLocalToDevice(SkM44(localToDevice));
    }
    ~SkAutoDeviceTransformRestore() {
        fDevice->setLocalToDevice(fPrevLocalToDevice);
    }

private:
    SkDevice* fDevice;
    const SkM44   fPrevLocalToDevice;
};

#endif
