/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkIcoCodec_DEFINED
#define SkIcoCodec_DEFINED

#include "SkCodec.h"
#include "SkImageInfo.h"
#include "SkStream.h"
#include "SkTArray.h"
#include "SkTypes.h"

/*
 * This class implements the decoding for bmp images
 */
class SkIcoCodec : public SkCodec {
public:
    static bool IsIco(const void*, size_t);

    /*
     * Assumes IsIco was called and returned true
     * Creates an Ico decoder
     * Reads enough of the stream to determine the image format
     */
    static std::unique_ptr<SkCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*);

protected:

    /*
     * Chooses the best dimensions given the desired scale
     */
    SkISize onGetScaledDimensions(float desiredScale) const override;

    bool onDimensionsSupported(const SkISize&) override;

    /*
     * Initiates the Ico decode
     */
    Result onGetPixels(const SkImageInfo& dstInfo, void* dst, size_t dstRowBytes, const Options&,
            int*) override;

    SkEncodedImageFormat onGetEncodedFormat() const override {
        return SkEncodedImageFormat::kICO;
    }

    SkScanlineOrder onGetScanlineOrder() const override;

    bool conversionSupported(const SkImageInfo&, SkColorType, bool,
                             const SkColorSpace*) const override {
        // This will be checked by the embedded codec.
        return true;
    }

    // Handled by the embedded codec.
    bool usesColorXform() const override { return false; }
private:

    Result onStartScanlineDecode(const SkImageInfo& dstInfo,
            const SkCodec::Options& options) override;

    int onGetScanlines(void* dst, int count, size_t rowBytes) override;

    bool onSkipScanlines(int count) override;

    Result onStartIncrementalDecode(const SkImageInfo& dstInfo, void* pixels, size_t rowBytes,
            const SkCodec::Options&) override;

    Result onIncrementalDecode(int* rowsDecoded) override;

    SkSampler* getSampler(bool createIfNecessary) override;

    /*
     * Searches fEmbeddedCodecs for a codec that matches requestedSize.
     * The search starts at startIndex and ends when an appropriate codec
     * is found, or we have reached the end of the array.
     *
     * @return the index of the matching codec or -1 if there is no
     *         matching codec between startIndex and the end of
     *         the array.
     */
    int chooseCodec(const SkISize& requestedSize, int startIndex);

    /*
     * Constructor called by NewFromStream
     * @param embeddedCodecs codecs for the embedded images, takes ownership
     */
    SkIcoCodec(int width, int height, const SkEncodedInfo& info,
            SkTArray<std::unique_ptr<SkCodec>, true>* embeddedCodecs, sk_sp<SkColorSpace> colorSpace);

    std::unique_ptr<SkTArray<std::unique_ptr<SkCodec>, true>> fEmbeddedCodecs;

    // fCurrCodec is owned by this class, but should not be an
    // std::unique_ptr.  It will be deleted by the destructor of fEmbeddedCodecs.
    SkCodec* fCurrCodec;

    typedef SkCodec INHERITED;
};
#endif  // SkIcoCodec_DEFINED
