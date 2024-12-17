/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegCodec_DEFINED
#define SkJpegCodec_DEFINED

#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedImageFormat.h"
#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/base/SkTemplates.h"

#include <cstddef>
#include <cstdint>
#include <memory>

class JpegDecoderMgr;
class SkSampler;
class SkStream;
class SkSwizzler;
struct SkGainmapInfo;
struct SkImageInfo;

/*
 *
 * This class implements the decoding for jpeg images
 *
 */
class SkJpegCodec : public SkCodec {
public:
    ~SkJpegCodec() override;

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

    bool onQueryYUVAInfo(const SkYUVAPixmapInfo::SupportedDataTypes&,
                         SkYUVAPixmapInfo*) const override;

    Result onGetYUVAPlanes(const SkYUVAPixmaps& yuvaPixmaps) override;

    SkEncodedImageFormat onGetEncodedFormat() const override {
        return SkEncodedImageFormat::kJPEG;
    }

    bool onRewind() override;

    bool onDimensionsSupported(const SkISize&) override;

    bool conversionSupported(const SkImageInfo&, bool, bool) override;

    bool onGetGainmapCodec(SkGainmapInfo* info, std::unique_ptr<SkCodec>* gainmapCodec) override;
    bool onGetGainmapInfo(SkGainmapInfo* info,
                          std::unique_ptr<SkStream>* gainmapImageStream) override;

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
     * @param origin indicates the image orientation as specified in Exif metadata.
     * @param xmpMetadata holds the XMP metadata included in the image, if any.
     */
    SkJpegCodec(SkEncodedInfo&& info,
                std::unique_ptr<SkStream> stream,
                JpegDecoderMgr* decoderMgr,
                SkEncodedOrigin origin);

    void initializeSwizzler(const SkImageInfo& dstInfo, const Options& options,
                            bool needsCMYKToRGB);
    [[nodiscard]] bool allocateStorage(const SkImageInfo& dstInfo);
    Result readRows(const SkImageInfo& dstInfo, void* dst, size_t rowBytes, int count,
                  const Options&, int* rowsDecoded);

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


    skia_private::AutoTMalloc<uint8_t>             fStorage;
    uint8_t* fSwizzleSrcRow = nullptr;
    uint32_t* fColorXformSrcRow = nullptr;

    // libjpeg-turbo provides some subsetting.  In the case that libjpeg-turbo
    // cannot take the exact the subset that we need, we will use the swizzler
    // to further subset the output from libjpeg-turbo.
    SkIRect fSwizzlerSubset = SkIRect::MakeEmpty();

    std::unique_ptr<SkSwizzler>        fSwizzler;

    friend class SkRawCodec;

    using INHERITED = SkCodec;
};

#endif
