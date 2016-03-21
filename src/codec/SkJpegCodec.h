/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegCodec_DEFINED
#define SkJpegCodec_DEFINED

#include "SkCodec.h"
#include "SkColorSpace.h"
#include "SkImageInfo.h"
#include "SkSwizzler.h"
#include "SkStream.h"
#include "SkTemplates.h"

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
     * Creates a jpeg decoder
     * Takes ownership of the stream
     */
    static SkCodec* NewFromStream(SkStream*);

protected:

    /*
     * Recommend a set of destination dimensions given a requested scale
     */
    SkISize onGetScaledDimensions(float desiredScale) const override;

    /*
     * Initiates the jpeg decode
     */
    Result onGetPixels(const SkImageInfo& dstInfo, void* dst, size_t dstRowBytes, const Options&,
            SkPMColor*, int*, int*) override;

    bool onQueryYUV8(SkYUVSizeInfo* sizeInfo, SkYUVColorSpace* colorSpace) const override;

    Result onGetYUV8Planes(const SkYUVSizeInfo& sizeInfo, void* planes[3]) override;

    SkEncodedFormat onGetEncodedFormat() const override {
        return kJPEG_SkEncodedFormat;
    }

    bool onRewind() override;

    bool onDimensionsSupported(const SkISize&) override;

private:

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
     */
    static bool ReadHeader(SkStream* stream, SkCodec** codecOut,
            JpegDecoderMgr** decoderMgrOut);

    /*
     * Creates an instance of the decoder
     * Called only by NewFromStream
     *
     * @param srcInfo contains the source width and height
     * @param stream the encoded image data
     * @param decoderMgr holds decompress struct, src manager, and error manager
     *                   takes ownership
     */
    SkJpegCodec(const SkImageInfo& srcInfo, SkStream* stream, JpegDecoderMgr* decoderMgr,
            sk_sp<SkColorSpace> colorSpace, Origin origin);

    /*
     * Checks if the conversion between the input image and the requested output
     * image has been implemented
     * Sets the output color space
     */
    bool setOutputColorSpace(const SkImageInfo& dst);

    // scanline decoding
    void initializeSwizzler(const SkImageInfo& dstInfo, const Options& options);
    SkSampler* getSampler(bool createIfNecessary) override;
    Result onStartScanlineDecode(const SkImageInfo& dstInfo, const Options& options,
            SkPMColor ctable[], int* ctableCount) override;
    int onGetScanlines(void* dst, int count, size_t rowBytes) override;
    bool onSkipScanlines(int count) override;

    SkAutoTDelete<JpegDecoderMgr> fDecoderMgr;
    // We will save the state of the decompress struct after reading the header.
    // This allows us to safely call onGetScaledDimensions() at any time.
    const int                     fReadyState;

    // scanline decoding
    SkAutoTMalloc<uint8_t>     fStorage;    // Only used if sampling is needed
    uint8_t*                   fSrcRow;     // Only used if sampling is needed
    // libjpeg-turbo provides some subsetting.  In the case that libjpeg-turbo
    // cannot take the exact the subset that we need, we will use the swizzler
    // to further subset the output from libjpeg-turbo.
    SkIRect                    fSwizzlerSubset;
    SkAutoTDelete<SkSwizzler>  fSwizzler;
    
    typedef SkCodec INHERITED;
};

#endif
