/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageGenerator_DEFINED
#define SkImageGenerator_DEFINED

#include "SkBitmap.h"
#include "SkColor.h"
#include "SkImage.h"
#include "SkImageInfo.h"
#include "SkYUVSizeInfo.h"

class GrContext;
class GrContextThreadSafeProxy;
class GrTextureProxy;
class GrSamplerParams;
class SkBitmap;
class SkData;
class SkMatrix;
class SkPaint;
class SkPicture;

class SK_API SkImageGenerator : public SkNoncopyable {
public:
    /**
     *  The PixelRef which takes ownership of this SkImageGenerator
     *  will call the image generator's destructor.
     */
    virtual ~SkImageGenerator() { }

    uint32_t uniqueID() const { return fUniqueID; }

    /**
     *  Return a ref to the encoded (i.e. compressed) representation,
     *  of this data. If the GrContext is non-null, then the caller is only interested in
     *  gpu-specific formats, so the impl may return null even if they have encoded data,
     *  assuming they know it is not suitable for the gpu.
     *
     *  If non-NULL is returned, the caller is responsible for calling
     *  unref() on the data when it is finished.
     */
    SkData* refEncodedData(GrContext* ctx = nullptr) {
        return this->onRefEncodedData(ctx);
    }

    /**
     *  Return the ImageInfo associated with this generator.
     */
    const SkImageInfo& getInfo() const { return fInfo; }

    /**
     *  Decode into the given pixels, a block of memory of size at
     *  least (info.fHeight - 1) * rowBytes + (info.fWidth *
     *  bytesPerPixel)
     *
     *  Repeated calls to this function should give the same results,
     *  allowing the PixelRef to be immutable.
     *
     *  @param info A description of the format (config, size)
     *         expected by the caller.  This can simply be identical
     *         to the info returned by getInfo().
     *
     *         This contract also allows the caller to specify
     *         different output-configs, which the implementation can
     *         decide to support or not.
     *
     *         A size that does not match getInfo() implies a request
     *         to scale. If the generator cannot perform this scale,
     *         it will return kInvalidScale.
     *
     *  If info is kIndex8_SkColorType, then the caller must provide storage for up to 256
     *  SkPMColor values in ctable. On success the generator must copy N colors into that storage,
     *  (where N is the logical number of table entries) and set ctableCount to N.
     *
     *  If info is not kIndex8_SkColorType, then the last two parameters may be NULL. If ctableCount
     *  is not null, it will be set to 0.
     *
     *  @return true on success.
     */
    bool getPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                   SkPMColor ctable[], int* ctableCount);

    /**
     *  Simplified version of getPixels() that asserts that info is NOT kIndex8_SkColorType and
     *  uses the default Options.
     */
    bool getPixels(const SkImageInfo& info, void* pixels, size_t rowBytes);

    /**
     *  If decoding to YUV is supported, this returns true.  Otherwise, this
     *  returns false and does not modify any of the parameters.
     *
     *  @param sizeInfo   Output parameter indicating the sizes and required
     *                    allocation widths of the Y, U, and V planes.
     *  @param colorSpace Output parameter.
     */
    bool queryYUV8(SkYUVSizeInfo* sizeInfo, SkYUVColorSpace* colorSpace) const;

    /**
     *  Returns true on success and false on failure.
     *  This always attempts to perform a full decode.  If the client only
     *  wants size, it should call queryYUV8().
     *
     *  @param sizeInfo   Needs to exactly match the values returned by the
     *                    query, except the WidthBytes may be larger than the
     *                    recommendation (but not smaller).
     *  @param planes     Memory for each of the Y, U, and V planes.
     */
    bool getYUV8Planes(const SkYUVSizeInfo& sizeInfo, void* planes[3]);

#if SK_SUPPORT_GPU
    /**
     *  If the generator can natively/efficiently return its pixels as a GPU image (backed by a
     *  texture) this will return that image. If not, this will return NULL.
     *
     *  This routine also supports retrieving only a subset of the pixels. That subset is specified
     *  by the following rectangle:
     *
     *      subset = SkIRect::MakeXYWH(origin.x(), origin.y(), info.width(), info.height())
     *
     *  If subset is not contained inside the generator's bounds, this returns false.
     *
     *      whole = SkIRect::MakeWH(getInfo().width(), getInfo().height())
     *      if (!whole.contains(subset)) {
     *          return false;
     *      }
     *
     *  Regarding the GrContext parameter:
     *
     *  It must be non-NULL. The generator should only succeed if:
     *  - its internal context is the same
     *  - it can somehow convert its texture into one that is valid for the provided context.
     */
    sk_sp<GrTextureProxy> generateTexture(GrContext*, const SkImageInfo& info,
                                          const SkIPoint& origin);
#endif

    /**
     *  If the default image decoder system can interpret the specified (encoded) data, then
     *  this returns a new ImageGenerator for it. Otherwise this returns NULL. Either way
     *  the caller is still responsible for managing their ownership of the data.
     */
    static std::unique_ptr<SkImageGenerator> MakeFromEncoded(sk_sp<SkData>);

    /** Return a new image generator backed by the specified picture.  If the size is empty or
     *  the picture is NULL, this returns NULL.
     *  The optional matrix and paint arguments are passed to drawPicture() at rasterization
     *  time.
     */
    static std::unique_ptr<SkImageGenerator> MakeFromPicture(const SkISize&, sk_sp<SkPicture>,
                                                             const SkMatrix*, const SkPaint*,
                                                             SkImage::BitDepth,
                                                             sk_sp<SkColorSpace>);

protected:
    enum {
        kNeedNewImageUniqueID = 0
    };

    SkImageGenerator(const SkImageInfo& info, uint32_t uniqueId = kNeedNewImageUniqueID);

    virtual SkData* onRefEncodedData(GrContext* ctx);

    virtual bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                             SkPMColor ctable[], int* ctableCount);

    virtual bool onQueryYUV8(SkYUVSizeInfo*, SkYUVColorSpace*) const {
        return false;
    }
    virtual bool onGetYUV8Planes(const SkYUVSizeInfo&, void*[3] /*planes*/) {
        return false;
    }

    struct Options {
        Options()
            : fColorTable(nullptr)
            , fColorTableCount(nullptr)
            , fBehavior(SkTransferFunctionBehavior::kRespect)
        {}

        SkPMColor*                 fColorTable;
        int*                       fColorTableCount;
        SkTransferFunctionBehavior fBehavior;
    };
    bool getPixels(const SkImageInfo& info, void* pixels, size_t rowBytes, const Options* opts);
    virtual bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                             const Options& opts) {
        return this->onGetPixels(info, pixels, rowBytes, opts.fColorTable, opts.fColorTableCount);
    }

#if SK_SUPPORT_GPU
    virtual sk_sp<GrTextureProxy> onGenerateTexture(GrContext*, const SkImageInfo&,
                                                    const SkIPoint&);
#endif

private:
    const SkImageInfo fInfo;
    const uint32_t fUniqueID;

    friend class SkImageCacherator;

    // This is our default impl, which may be different on different platforms.
    // It is called from NewFromEncoded() after it has checked for any runtime factory.
    // The SkData will never be NULL, as that will have been checked by NewFromEncoded.
    static std::unique_ptr<SkImageGenerator> MakeFromEncodedImpl(sk_sp<SkData>);
};

#endif  // SkImageGenerator_DEFINED
