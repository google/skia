/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegCodec_DEFINED
#define SkJpegCodec_DEFINED

#include "SkCodec.h"
#include "SkImageInfo.h"
#include "SkJpegDecoderMgr.h"
#include "SkJpegUtility_codec.h"
#include "SkStream.h"

extern "C" {
    #include "jpeglib.h"
}

class SkScanlineDecoder;

/*
 *
 * This class implements the decoding for jpeg images
 *
 */
class SkJpegCodec : public SkCodec {
public:

    /*
     * Checks the start of the stream to see if the image is a jpeg
     * Does not take ownership of the stream
     */
    static bool IsJpeg(SkStream*);

    /*
     * Assumes IsJpeg was called and returned true
     * Creates a jpeg decoder
     * Takes ownership of the stream
     */
    static SkCodec* NewFromStream(SkStream*);

    /*
     * Assumes IsJpeg was called and returned true
     * Creates a jpeg scanline decoder
     * Takes ownership of the stream
     */
    static SkScanlineDecoder* NewSDFromStream(SkStream*);

protected:

    /*
     * Recommend a set of destination dimensions given a requested scale
     */
    SkISize onGetScaledDimensions(float desiredScale) const override;

    /*
     * Initiates the jpeg decode
     */
    Result onGetPixels(const SkImageInfo& dstInfo, void* dst, size_t dstRowBytes, const Options&,
            SkPMColor*, int*) override;

    SkEncodedFormat onGetEncodedFormat() const override {
        return kJPEG_SkEncodedFormat;
    }

    bool onRewind() override;

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
    SkJpegCodec(const SkImageInfo& srcInfo, SkStream* stream, JpegDecoderMgr* decoderMgr);

    /*
     * Checks if the conversion between the input image and the requested output
     * image has been implemented
     * Sets the output color space
     */
    bool setOutputColorSpace(const SkImageInfo& dst);

    /*
     * Checks if we can natively scale to the requested dimensions and natively scales the 
     * dimensions if possible
     */
    bool nativelyScaleToDimensions(uint32_t width, uint32_t height); 

    SkAutoTDelete<JpegDecoderMgr> fDecoderMgr;
    // We will save the state of the decompress struct after reading the header.
    // This allows us to safely call onGetScaledDimensions() at any time.
    const int                     fReadyState;
    
    friend class SkJpegScanlineDecoder;

    typedef SkCodec INHERITED;
};

#endif
