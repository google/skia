/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmap_DEFINED
#define SkBitmap_DEFINED

#include "SkColor.h"
#include "SkColorTable.h"
#include "SkImageInfo.h"
#include "SkPixmap.h"
#include "SkPoint.h"
#include "SkRefCnt.h"

struct SkMask;
struct SkIRect;
struct SkRect;
class SkPaint;
class SkPixelRef;
class SkString;

/** \class SkBitmap

    The SkBitmap class specifies a raster bitmap. A bitmap has an integer width
    and height, and a format (colortype), and a pointer to the actual pixels.
    Bitmaps can be drawn into a SkCanvas, but they are also used to specify the
    target of a SkCanvas' drawing operations.
    A const SkBitmap exposes getAddr(), which lets a caller write its pixels;
    the constness is considered to apply to the bitmap's configuration, not
    its contents.

    SkBitmap is not thread safe.  Each thread must use its own (shallow) copy.
*/
class SK_API SkBitmap {
public:
    class SK_API Allocator;

    /**
     *  Default construct creates a bitmap with zero width and height, and no pixels.
     *  Its colortype is set to kUnknown_SkColorType.
     */
    SkBitmap();

    /**
     *  Copy the settings from the src into this bitmap. If the src has pixels
     *  allocated, they will be shared, not copied, so that the two bitmaps will
     *  reference the same memory for the pixels. If a deep copy is needed,
     *  where the new bitmap has its own separate copy of the pixels, use
     *  deepCopyTo().
     */
    SkBitmap(const SkBitmap& src);

    /**
     *  Copy the settings from the src into this bitmap. If the src has pixels
     *  allocated, ownership of the pixels will be taken.
     */
    SkBitmap(SkBitmap&& src);

    ~SkBitmap();

    /** Copies the src bitmap into this bitmap. Ownership of the src
        bitmap's pixels is shared with the src bitmap.
    */
    SkBitmap& operator=(const SkBitmap& src);

    /** Copies the src bitmap into this bitmap. Takes ownership of the src
        bitmap's pixels.
    */
    SkBitmap& operator=(SkBitmap&& src);

    /** Swap the fields of the two bitmaps. This routine is guaranteed to never fail or throw.
    */
    //  This method is not exported to java.
    void swap(SkBitmap& other);

    ///////////////////////////////////////////////////////////////////////////

    const SkImageInfo& info() const { return fInfo; }

    int width() const { return fInfo.width(); }
    int height() const { return fInfo.height(); }
    SkColorType colorType() const { return fInfo.colorType(); }
    SkAlphaType alphaType() const { return fInfo.alphaType(); }
    SkColorSpace* colorSpace() const { return fInfo.colorSpace(); }
    sk_sp<SkColorSpace> refColorSpace() const { return fInfo.refColorSpace(); }

    /**
     *  Return the number of bytes per pixel based on the colortype. If the colortype is
     *  kUnknown_SkColorType, then 0 is returned.
     */
    int bytesPerPixel() const { return fInfo.bytesPerPixel(); }

    /**
     *  Return the rowbytes expressed as a number of pixels (like width and height).
     *  If the colortype is kUnknown_SkColorType, then 0 is returned.
     */
    int rowBytesAsPixels() const {
        return fRowBytes >> this->shiftPerPixel();
    }

    /**
     *  Return the shift amount per pixel (i.e. 0 for 1-byte per pixel, 1 for 2-bytes per pixel
     *  colortypes, 2 for 4-bytes per pixel colortypes). Return 0 for kUnknown_SkColorType.
     */
    int shiftPerPixel() const { return this->fInfo.shiftPerPixel(); }

    ///////////////////////////////////////////////////////////////////////////

    /** Return true iff the bitmap has empty dimensions.
     *  Hey!  Before you use this, see if you really want to know drawsNothing() instead.
     */
    bool empty() const { return fInfo.isEmpty(); }

    /** Return true iff the bitmap has no pixelref. Note: this can return true even if the
     *  dimensions of the bitmap are > 0 (see empty()).
     *  Hey!  Before you use this, see if you really want to know drawsNothing() instead.
     */
    bool isNull() const { return nullptr == fPixelRef; }

    /** Return true iff drawing this bitmap has no effect.
     */
    bool drawsNothing() const { return this->empty() || this->isNull(); }

    /** Return the number of bytes between subsequent rows of the bitmap. */
    size_t rowBytes() const { return fRowBytes; }

    /**
     *  Set the bitmap's alphaType, returning true on success. If false is
     *  returned, then the specified new alphaType is incompatible with the
     *  colortype, and the current alphaType is unchanged.
     *
     *  Note: this changes the alphatype for the underlying pixels, which means
     *  that all bitmaps that might be sharing (subsets of) the pixels will
     *  be affected.
     */
    bool setAlphaType(SkAlphaType);

    /** Return the address of the pixels for this SkBitmap.
    */
    void* getPixels() const { return fPixels; }

    /** Return the byte size of the pixels, based on the height and rowBytes.
        Note this truncates the result to 32bits. Call getSize64() to detect
        if the real size exceeds 32bits.
    */
    size_t getSize() const { return fInfo.height() * fRowBytes; }

    /** Return the number of bytes from the pointer returned by getPixels()
        to the end of the allocated space in the buffer. Required in
        cases where extractSubset has been called.
    */
    size_t getSafeSize() const { return fInfo.getSafeSize(fRowBytes); }

    /**
     *  Return the full size of the bitmap, in bytes.
     */
    int64_t computeSize64() const {
        return sk_64_mul(fInfo.height(), fRowBytes);
    }

    /**
     *  Return the number of bytes from the pointer returned by getPixels()
     *  to the end of the allocated space in the buffer. This may be smaller
     *  than computeSize64() if there is any rowbytes padding beyond the width.
     */
    int64_t computeSafeSize64() const {
        return fInfo.getSafeSize64(fRowBytes);
    }

    /** Returns true if this bitmap is marked as immutable, meaning that the
        contents of its pixels will not change for the lifetime of the bitmap.
    */
    bool isImmutable() const;

    /** Marks this bitmap as immutable, meaning that the contents of its
        pixels will not change for the lifetime of the bitmap and of the
        underlying pixelref. This state can be set, but it cannot be
        cleared once it is set. This state propagates to all other bitmaps
        that share the same pixelref.
    */
    void setImmutable();

    /** Returns true if the bitmap is opaque (has no translucent/transparent pixels).
    */
    bool isOpaque() const {
        return SkAlphaTypeIsOpaque(this->alphaType());
    }

    /** Returns true if the bitmap is volatile (i.e. should not be cached by devices.)
    */
    bool isVolatile() const;

    /** Specify whether this bitmap is volatile. Bitmaps are not volatile by
        default. Temporary bitmaps that are discarded after use should be
        marked as volatile. This provides a hint to the device that the bitmap
        should not be cached. Providing this hint when appropriate can
        improve performance by avoiding unnecessary overhead and resource
        consumption on the device.
    */
    void setIsVolatile(bool);

    /** Reset the bitmap to its initial state (see default constructor). If we are a (shared)
        owner of the pixels, that ownership is decremented.
    */
    void reset();

    /**
     *  This will brute-force return true if all of the pixels in the bitmap
     *  are opaque. If it fails to read the pixels, or encounters an error,
     *  it will return false.
     *
     *  Since this can be an expensive operation, the bitmap stores a flag for
     *  this (isOpaque). Only call this if you need to compute this value from
     *  "unknown" pixels.
     */
    static bool ComputeIsOpaque(const SkBitmap& bm) {
        SkPixmap pmap;
        return bm.peekPixels(&pmap) && pmap.computeIsOpaque();
    }

    /**
     *  Return the bitmap's bounds [0, 0, width, height] as an SkRect
     */
    void getBounds(SkRect* bounds) const;
    void getBounds(SkIRect* bounds) const;

    SkIRect bounds() const { return fInfo.bounds(); }
    SkISize dimensions() const { return fInfo.dimensions(); }
    // Returns the bounds of this bitmap, offset by its pixelref origin.
    SkIRect getSubset() const {
        return SkIRect::MakeXYWH(fPixelRefOrigin.x(), fPixelRefOrigin.y(),
                                 fInfo.width(), fInfo.height());
    }

    bool setInfo(const SkImageInfo&, size_t rowBytes = 0);

    enum AllocFlags {
        kZeroPixels_AllocFlag   = 1 << 0,
    };
    /**
     *  Allocate the bitmap's pixels to match the requested image info. If the Factory
     *  is non-null, call it to allcoate the pixelref. If the ImageInfo requires
     *  a colortable, then ColorTable must be non-null.
     *
     *  On failure, the bitmap will be set to empty and return false.
     */
    bool SK_WARN_UNUSED_RESULT tryAllocPixels(const SkImageInfo& info, sk_sp<SkColorTable> ctable,
                                              uint32_t flags = 0);
    void allocPixels(const SkImageInfo& info, sk_sp<SkColorTable> ctable, uint32_t flags = 0) {
        if (!this->tryAllocPixels(info, std::move(ctable), flags)) {
            sk_throw();
        }
    }

    /**
     *  Allocate the bitmap's pixels to match the requested image info and
     *  rowBytes. If the request cannot be met (e.g. the info is invalid or
     *  the requested rowBytes are not compatible with the info
     *  (e.g. rowBytes < info.minRowBytes() or rowBytes is not aligned with
     *  the pixel size specified by info.colorType()) then false is returned
     *  and the bitmap is set to empty.
     */
    bool SK_WARN_UNUSED_RESULT tryAllocPixels(const SkImageInfo& info, size_t rowBytes);

    void allocPixels(const SkImageInfo& info, size_t rowBytes) {
        if (!this->tryAllocPixels(info, rowBytes)) {
            sk_throw();
        }
    }

    bool SK_WARN_UNUSED_RESULT tryAllocPixels(const SkImageInfo& info) {
        return this->tryAllocPixels(info, info.minRowBytes());
    }

    void allocPixels(const SkImageInfo& info) {
        this->allocPixels(info, info.minRowBytes());
    }

    bool SK_WARN_UNUSED_RESULT tryAllocN32Pixels(int width, int height, bool isOpaque = false) {
        SkImageInfo info = SkImageInfo::MakeN32(width, height,
                                            isOpaque ? kOpaque_SkAlphaType : kPremul_SkAlphaType);
        return this->tryAllocPixels(info);
    }

    void allocN32Pixels(int width, int height, bool isOpaque = false) {
        SkImageInfo info = SkImageInfo::MakeN32(width, height,
                                            isOpaque ? kOpaque_SkAlphaType : kPremul_SkAlphaType);
        this->allocPixels(info);
    }

    // TEMPORARY -- remove after updating Android BitmapTests.cpp:35
    void allocPixels(const SkImageInfo& info, std::nullptr_t, SkColorTable* ctable) {
        this->allocPixels(info, sk_ref_sp(ctable));
    }

    /**
     *  Install a pixelref that wraps the specified pixels and rowBytes, and
     *  optional ReleaseProc and context. When the pixels are no longer
     *  referenced, if releaseProc is not null, it will be called with the
     *  pixels and context as parameters.
     *  On failure, the bitmap will be set to empty and return false.
     *
     *  If specified, the releaseProc will always be called, even on failure. It is also possible
     *  for success but the releaseProc is immediately called (e.g. valid Info but NULL pixels).
     */
    bool installPixels(const SkImageInfo&, void* pixels, size_t rowBytes, SkColorTable*,
                       void (*releaseProc)(void* addr, void* context), void* context);

    /**
     *  Call installPixels with no ReleaseProc specified. This means that the
     *  caller must ensure that the specified pixels are valid for the lifetime
     *  of the created bitmap (and its pixelRef).
     */
    bool installPixels(const SkImageInfo& info, void* pixels, size_t rowBytes) {
        return this->installPixels(info, pixels, rowBytes, NULL, NULL, NULL);
    }

    /**
     *  Call installPixels with no ReleaseProc specified. This means
     *  that the caller must ensure that the specified pixels and
     *  colortable are valid for the lifetime of the created bitmap
     *  (and its pixelRef).
     */
    bool installPixels(const SkPixmap&);

    /**
     *  Calls installPixels() with the value in the SkMask. The caller must
     *  ensure that the specified mask pixels are valid for the lifetime
     *  of the created bitmap (and its pixelRef).
     */
    bool installMaskPixels(const SkMask&);

    /** Use this to assign a new pixel address for an existing bitmap. This
        will automatically release any pixelref previously installed. Only call
        this if you are handling ownership/lifetime of the pixel memory.

        If the bitmap retains a reference to the colortable (assuming it is
        not null) it will take care of incrementing the reference count.

        @param pixels   Address for the pixels, managed by the caller.
        @param ctable   ColorTable (or null) that matches the specified pixels
    */
    void setPixels(void* p, SkColorTable* ctable = NULL);

    /** Use the standard HeapAllocator to create the pixelref that manages the
        pixel memory. It will be sized based on the current ImageInfo.
        If this is called multiple times, a new pixelref object will be created
        each time.

        If the bitmap retains a reference to the colortable (assuming it is
        not null) it will take care of incrementing the reference count.

        @param ctable   ColorTable (or null) to use with the pixels that will
                        be allocated. Only used if colortype == kIndex_8_SkColorType
        @return true if the allocation succeeds. If not the pixelref field of
                     the bitmap will be unchanged.
    */
    bool SK_WARN_UNUSED_RESULT tryAllocPixels(SkColorTable* ctable = NULL) {
        return this->tryAllocPixels(NULL, ctable);
    }

    void allocPixels(SkColorTable* ctable = NULL) {
        this->allocPixels(NULL, ctable);
    }

    /** Use the specified Allocator to create the pixelref that manages the
        pixel memory. It will be sized based on the current ImageInfo.
        If this is called multiple times, a new pixelref object will be created
        each time.

        If the bitmap retains a reference to the colortable (assuming it is
        not null) it will take care of incrementing the reference count.

        @param allocator The Allocator to use to create a pixelref that can
                         manage the pixel memory for the current ImageInfo.
                         If allocator is NULL, the standard HeapAllocator will be used.
        @param ctable   ColorTable (or null) to use with the pixels that will
                        be allocated. Only used if colortype == kIndex_8_SkColorType.
                        If it is non-null and the colortype is not indexed, it will
                        be ignored.
        @return true if the allocation succeeds. If not the pixelref field of
                     the bitmap will be unchanged.
    */
    bool SK_WARN_UNUSED_RESULT tryAllocPixels(Allocator* allocator, SkColorTable* ctable);

    void allocPixels(Allocator* allocator, SkColorTable* ctable) {
        if (!this->tryAllocPixels(allocator, ctable)) {
            sk_throw();
        }
    }

    /**
     *  Return the current pixelref object or NULL if there is none. This does
     *  not affect the refcount of the pixelref.
     */
    SkPixelRef* pixelRef() const { return fPixelRef.get(); }

    /**
     *  A bitmap can reference a subset of a pixelref's pixels. That means the
     *  bitmap's width/height can be <= the dimensions of the pixelref. The
     *  pixelref origin is the x,y location within the pixelref's pixels for
     *  the bitmap's top/left corner. To be valid the following must be true:
     *
     *  origin_x + bitmap_width  <= pixelref_width
     *  origin_y + bitmap_height <= pixelref_height
     *
     *  pixelRefOrigin() returns this origin, or (0,0) if there is no pixelRef.
     */
    SkIPoint pixelRefOrigin() const { return fPixelRefOrigin; }

    /**
     * Assign a pixelref and origin to the bitmap.  (dx,dy) specify the offset
     * within the pixelref's pixels for the top/left corner of the bitmap. For
     * a bitmap that encompases the entire pixels of the pixelref, these will
     * be (0,0).
     */
    void setPixelRef(sk_sp<SkPixelRef>, int dx, int dy);

    /** Call this to be sure that the bitmap is valid enough to be drawn (i.e.
        it has non-null pixels, and if required by its colortype, it has a
        non-null colortable. Returns true if all of the above are met.
    */
    bool readyToDraw() const {
        return this->getPixels() != NULL &&
               (this->colorType() != kIndex_8_SkColorType || this->getColorTable());
    }

    /** Return the bitmap's colortable, if it uses one (i.e. colorType is
        Index_8).
        Otherwise returns NULL. Does not affect the colortable's
        reference count.
    */
    SkColorTable* getColorTable() const;

    /** Returns a non-zero, unique value corresponding to the pixels in our
        pixelref. Each time the pixels are changed (and notifyPixelsChanged
        is called), a different generation ID will be returned. Finally, if
        there is no pixelRef then zero is returned.
    */
    uint32_t getGenerationID() const;

    /** Call this if you have changed the contents of the pixels. This will in-
        turn cause a different generation ID value to be returned from
        getGenerationID().
    */
    void notifyPixelsChanged() const;

    /**
     *  Fill the entire bitmap with the specified color.
     *  If the bitmap's colortype does not support alpha (e.g. 565) then the alpha
     *  of the color is ignored (treated as opaque). If the colortype only supports
     *  alpha (e.g. A1 or A8) then the color's r,g,b components are ignored.
     */
    void eraseColor(SkColor c) const;

    /**
     *  Fill the entire bitmap with the specified color.
     *  If the bitmap's colortype does not support alpha (e.g. 565) then the alpha
     *  of the color is ignored (treated as opaque). If the colortype only supports
     *  alpha (e.g. A1 or A8) then the color's r,g,b components are ignored.
     */
    void eraseARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b) const {
        this->eraseColor(SkColorSetARGB(a, r, g, b));
    }

    SK_ATTR_DEPRECATED("use eraseARGB or eraseColor")
    void eraseRGB(U8CPU r, U8CPU g, U8CPU b) const {
        this->eraseARGB(0xFF, r, g, b);
    }

    /**
     *  Fill the specified area of this bitmap with the specified color.
     *  If the bitmap's colortype does not support alpha (e.g. 565) then the alpha
     *  of the color is ignored (treated as opaque). If the colortype only supports
     *  alpha (e.g. A1 or A8) then the color's r,g,b components are ignored.
     */
    void erase(SkColor c, const SkIRect& area) const;

    // DEPRECATED
    void eraseArea(const SkIRect& area, SkColor c) const {
        this->erase(c, area);
    }

    /**
     *  Converts the pixel at the specified coordinate to an unpremultiplied
     *  SkColor. Note: this ignores any SkColorSpace information, and may return
     *  lower precision data than is actually in the pixel. Alpha only
     *  colortypes (e.g. kAlpha_8_SkColorType) return black with the appropriate
     *  alpha set.  The value is undefined for kUnknown_SkColorType or if x or y
     *  are out of bounds, or if the bitmap does not have any pixels.
     */
    SkColor getColor(int x, int y) const {
        SkPixmap pixmap;
        SkAssertResult(this->peekPixels(&pixmap));
        return pixmap.getColor(x, y);
    }

    /** Returns the address of the specified pixel. This performs a runtime
        check to know the size of the pixels, and will return the same answer
        as the corresponding size-specific method (e.g. getAddr16). Since the
        check happens at runtime, it is much slower than using a size-specific
        version. Unlike the size-specific methods, this routine also checks if
        getPixels() returns null, and returns that. The size-specific routines
        perform a debugging assert that getPixels() is not null, but they do
        not do any runtime checks.
    */
    void* getAddr(int x, int y) const;

    /** Returns the address of the pixel specified by x,y for 32bit pixels.
     *  In debug build, this asserts that the pixels are allocated and that the
     *  colortype is 32-bit, however none of these checks are performed
     *  in the release build.
     */
    inline uint32_t* getAddr32(int x, int y) const;

    /** Returns the address of the pixel specified by x,y for 16bit pixels.
     *  In debug build, this asserts that the pixels are allocated
     *  and that the colortype is 16-bit, however none of these checks are performed
     *  in the release build.
     */
    inline uint16_t* getAddr16(int x, int y) const;

    /** Returns the address of the pixel specified by x,y for 8bit pixels.
     *  In debug build, this asserts that the pixels are allocated
     *  and that the colortype is 8-bit, however none of these checks are performed
     *  in the release build.
     */
    inline uint8_t* getAddr8(int x, int y) const;

    /** Returns the color corresponding to the pixel specified by x,y for
     *  colortable based bitmaps.
     *  In debug build, this asserts that the pixels are allocated,
     *  that the colortype is indexed, and that the colortable is allocated,
     *  however none of these checks are performed in the release build.
     */
    inline SkPMColor getIndex8Color(int x, int y) const;

    /** Set dst to be a setset of this bitmap. If possible, it will share the
        pixel memory, and just point into a subset of it. However, if the colortype
        does not support this, a local copy will be made and associated with
        the dst bitmap. If the subset rectangle, intersected with the bitmap's
        dimensions is empty, or if there is an unsupported colortype, false will be
        returned and dst will be untouched.
        @param dst  The bitmap that will be set to a subset of this bitmap
        @param subset The rectangle of pixels in this bitmap that dst will
                      reference.
        @return true if the subset copy was successfully made.
    */
    bool extractSubset(SkBitmap* dst, const SkIRect& subset) const;

    /**
     *  Copy the bitmap's pixels into the specified buffer (pixels + rowBytes),
     *  converting them into the requested format (SkImageInfo). The src pixels are read
     *  starting at the specified (srcX,srcY) offset, relative to the top-left corner.
     *
     *  The specified ImageInfo and (srcX,srcY) offset specifies a source rectangle
     *
     *      srcR.setXYWH(srcX, srcY, dstInfo.width(), dstInfo.height());
     *
     *  srcR is intersected with the bounds of the bitmap. If this intersection is not empty,
     *  then we have two sets of pixels (of equal size). Replace the dst pixels with the
     *  corresponding src pixels, performing any colortype/alphatype transformations needed
     *  (in the case where the src and dst have different colortypes or alphatypes).
     *
     *  This call can fail, returning false, for several reasons:
     *  - If srcR does not intersect the bitmap bounds.
     *  - If the requested colortype/alphatype cannot be converted from the src's types.
     *  - If the src pixels are not available.
     */
    bool readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                    int srcX, int srcY) const;
    bool readPixels(const SkPixmap& dst, int srcX, int srcY) const;
    bool readPixels(const SkPixmap& dst) const {
        return this->readPixels(dst, 0, 0);
    }

    /**
     *  Copy the src pixmap's pixels into this bitmap, offset by dstX, dstY.
     *
     *  This is logically the same as creating a bitmap around src, and calling readPixels on it
     *  with this bitmap as the dst.
     */
    bool writePixels(const SkPixmap& src, int dstX, int dstY) {
        return this->writePixels(src, dstX, dstY, SkTransferFunctionBehavior::kRespect);
    }
    bool writePixels(const SkPixmap& src) {
        return this->writePixels(src, 0, 0);
    }

#ifdef SK_BUILD_FOR_ANDROID
    bool hasHardwareMipMap() const {
        return (fFlags & kHasHardwareMipMap_Flag) != 0;
    }

    void setHasHardwareMipMap(bool hasHardwareMipMap) {
        if (hasHardwareMipMap) {
            fFlags |= kHasHardwareMipMap_Flag;
        } else {
            fFlags &= ~kHasHardwareMipMap_Flag;
        }
    }
#endif

    bool extractAlpha(SkBitmap* dst) const {
        return this->extractAlpha(dst, NULL, NULL, NULL);
    }

    bool extractAlpha(SkBitmap* dst, const SkPaint* paint,
                      SkIPoint* offset) const {
        return this->extractAlpha(dst, paint, NULL, offset);
    }

    /** Set dst to contain alpha layer of this bitmap. If destination bitmap
        fails to be initialized, e.g. because allocator can't allocate pixels
        for it, dst will not be modified and false will be returned.

        @param dst The bitmap to be filled with alpha layer
        @param paint The paint to draw with
        @param allocator Allocator used to allocate the pixelref for the dst
                         bitmap. If this is null, the standard HeapAllocator
                         will be used.
        @param offset If not null, it is set to top-left coordinate to position
                      the returned bitmap so that it visually lines up with the
                      original
    */
    bool extractAlpha(SkBitmap* dst, const SkPaint* paint, Allocator* allocator,
                      SkIPoint* offset) const;

    /**
     *  If the pixels are available from this bitmap return true, and fill out the
     *  specified pixmap (if not null). If there are no pixels, return false and
     *  ignore the pixmap parameter.
     *
     *  Note: if this returns true, the results (in the pixmap) are only valid until the bitmap
     *  is changed in any way, in which case the results are invalid.
     */
    bool peekPixels(SkPixmap*) const;

    SkDEBUGCODE(void validate() const;)

    class Allocator : public SkRefCnt {
    public:
        /** Allocate the pixel memory for the bitmap, given its dimensions and
            colortype. Return true on success, where success means either setPixels
            or setPixelRef was called. If the colortype requires a colortable,
            it also must be installed via setColorTable. If false is returned,
            the bitmap and colortable should be left unchanged.
        */
        virtual bool allocPixelRef(SkBitmap*, SkColorTable*) = 0;
    private:
        typedef SkRefCnt INHERITED;
    };

    /** Subclass of Allocator that returns a pixelref that allocates its pixel
        memory from the heap. This is the default Allocator invoked by
        allocPixels().
    */
    class HeapAllocator : public Allocator {
    public:
        bool allocPixelRef(SkBitmap*, SkColorTable*) override;
    };

    SK_TO_STRING_NONVIRT()

private:
    enum Flags {
        kImageIsVolatile_Flag   = 0x02,
#ifdef SK_BUILD_FOR_ANDROID
        /* A hint for the renderer responsible for drawing this bitmap
         * indicating that it should attempt to use mipmaps when this bitmap
         * is drawn scaled down.
         */
        kHasHardwareMipMap_Flag = 0x08,
#endif
    };

    sk_sp<SkPixelRef>   fPixelRef;
    void*               fPixels;
    SkIPoint            fPixelRefOrigin;
    SkImageInfo         fInfo;
    uint32_t            fRowBytes;
    uint8_t             fFlags;

    friend class SkImage_Raster;
    bool writePixels(const SkPixmap& src, int x, int y, SkTransferFunctionBehavior behavior);

    /*  Unreference any pixelrefs or colortables
    */
    void freePixels();
    void updatePixelsFromRef();

    static void WriteRawPixels(SkWriteBuffer*, const SkBitmap&);
    static bool ReadRawPixels(SkReadBuffer*, SkBitmap*);

    friend class SkReadBuffer;        // unflatten, rawpixels
    friend class SkBinaryWriteBuffer; // rawpixels
};

///////////////////////////////////////////////////////////////////////////////

inline uint32_t* SkBitmap::getAddr32(int x, int y) const {
    SkASSERT(fPixels);
    SkASSERT(4 == this->bytesPerPixel());
    SkASSERT((unsigned)x < (unsigned)this->width() && (unsigned)y < (unsigned)this->height());
    return (uint32_t*)((char*)fPixels + y * fRowBytes + (x << 2));
}

inline uint16_t* SkBitmap::getAddr16(int x, int y) const {
    SkASSERT(fPixels);
    SkASSERT(2 == this->bytesPerPixel());
    SkASSERT((unsigned)x < (unsigned)this->width() && (unsigned)y < (unsigned)this->height());
    return (uint16_t*)((char*)fPixels + y * fRowBytes + (x << 1));
}

inline uint8_t* SkBitmap::getAddr8(int x, int y) const {
    SkASSERT(fPixels);
    SkASSERT(1 == this->bytesPerPixel());
    SkASSERT((unsigned)x < (unsigned)this->width() && (unsigned)y < (unsigned)this->height());
    return (uint8_t*)fPixels + y * fRowBytes + x;
}

inline SkPMColor SkBitmap::getIndex8Color(int x, int y) const {
    SkASSERT(fPixels);
    SkASSERT(kIndex_8_SkColorType == this->colorType());
    SkASSERT((unsigned)x < (unsigned)this->width() && (unsigned)y < (unsigned)this->height());
    SkASSERT(this->getColorTable());
    return (*this->getColorTable())[*((const uint8_t*)fPixels + y * fRowBytes + x)];
}

#endif
