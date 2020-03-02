/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegCodec_DEFINED
#define SkJpegCodec_DEFINED

#include "include/codec/SkCodec.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkStream.h"
#include "include/private/SkTemplates.h"
#include "src/codec/SkSwizzler.h"

class JpegDecoderMgr;

/*
 *
 * This class implements the decoding for jpeg images
 *
 */
class SkJpegCodec : public SkCodec {
public:
    static bool IsJpeg(const void*, size_t);

    /*
     * Assumes IsJpeg was called and returned true
     * Takes ownership of the stream
     */
    static std::unique_ptr<SkCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*);

protected:

    /*
     * Recommend a set of destination dimensions given a requested scale
     */
    SkISize onGetScaledDimensions(float desiredScale) const override;

    /*
     * Initiates the jpeg decode
     */
    Result onGetPixels(const SkImageInfo& dstInfo, void* dst, size_t dstRowBytes, const Options&,
            int*) override;

    bool onQueryYUV8(SkYUVASizeInfo* sizeInfo, SkYUVColorSpace* colorSpace) const override;

    Result onGetYUV8Planes(const SkYUVASizeInfo& sizeInfo,
                           void* planes[SkYUVASizeInfo::kMaxCount]) override;

    SkEncodedImageFormat onGetEncodedFormat() const override {
        return SkEncodedImageFormat::kJPEG;
    }

    bool onRewind() override;

    bool onDimensionsSupported(const SkISize&) override;

    bool conversionSupported(const SkImageInfo&, bool, bool) override;

private:
    /*
     * Allows SkRawCodec to communicate the color profile from the exif data.
     */
    static std::unique_ptr<SkCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*,
            std::unique_ptr<SkEncodedInfo::ICCProfile> defaultColorProfile);

    /*
     * Read enough of the stream to initialize the SkJpegCodec.
     * Returns a bool representing success or failure.
     *
     * @param codecOut
     * If this returns true, and codecOut was not nullptr,
     * codecOut will be set to a new SkJpegCodec.
     *
     * @param decoderMgrOut
     * If this returns true, and codecOut was nullptr,
     * decoderMgrOut must be non-nullptr and decoderMgrOut will be set to a new
     * JpegDecoderMgr pointer.
     *
     * @param stream
     * Deleted on failure.
     * codecOut will take ownership of it in the case where we created a codec.
     * Ownership is unchanged when we set decoderMgrOut.
     *
     * @param defaultColorProfile
     * If the jpeg does not have an embedded color profile, the image data should
     * be tagged with this color profile.
     */
    static Result ReadHeader(SkStream* stream, SkCodec** codecOut,
            JpegDecoderMgr** decoderMgrOut,
            std::unique_ptr<SkEncodedInfo::ICCProfile> defaultColorProfile);

    /*
     * Creates an instance of the decoder
     * Called only by NewFromStream
     *
     * @param info contains properties of the encoded data
     * @param stream the encoded image data
     * @param decoderMgr holds decompress struct, src manager, and error manager
     *                   takes ownership
     */
    SkJpegCodec(SkEncodedInfo&& info, std::unique_ptr<SkStream> stream,
            JpegDecoderMgr* decoderMgr, SkEncodedOrigin origin);

    void initializeSwizzler(const SkImageInfo& dstInfo, const Options& options,
                            bool needsCMYKToRGB);
    bool SK_WARN_UNUSED_RESULT allocateStorage(const SkImageInfo& dstInfo);
    int readRows(const SkImageInfo& dstInfo, void* dst, size_t rowBytes, int count, const Options&);

    /*
     * Scanline decoding.
     */
    SkSampler* getSampler(bool createIfNecessary) override;
    Result onStartScanlineDecode(const SkImageInfo& dstInfo,
            const Options& options) override;
    int onGetScanlines(void* dst, int count, size_t rowBytes) override;
    bool onSkipScanlines(int count) override;

    std::unique_ptr<JpegDecoderMgr>    fDecoderMgr;

    // We will save the state of the decompress struct after reading the header.
    // This allows us to safely call onGetScaledDimensions() at any time.
    const int                          fReadyState;


    SkAutoTMalloc<uint8_t>             fStorage;
    uint8_t*                           fSwizzleSrcRow;
    uint32_t*                          fColorXformSrcRow;

    // libjpeg-turbo provides some subsetting.  In the case that libjpeg-turbo
    // cannot take the exact the subset that we need, we will use the swizzler
    // to further subset the output from libjpeg-turbo.
    SkIRect                            fSwizzlerSubset;

    std::unique_ptr<SkSwizzler>        fSwizzler;

    friend class SkRawCodec;

    typedef SkCodec INHERITED;
};

#endif
