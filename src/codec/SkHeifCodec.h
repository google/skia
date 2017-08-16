/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkHeifCodec_DEFINED
#define SkHeifCodec_DEFINED

#include "SkCodec.h"
#include "SkColorSpace.h"
#include "SkColorSpaceXform.h"
#include "SkImageInfo.h"
#include "SkSwizzler.h"
#include "SkStream.h"

struct HeifFrameInfo;
struct HeifDecoder;

/*
 *
 * This class implements the decoding for heif images.
 *
 */
class SkHeifCodec : public SkCodec {
public:
    static bool IsHeif(const void*, size_t);

    /*
     * Assumes IsHeif was called and returned true.
     * Creates a heif decoder and takes ownership of the stream.
     */
    static std::unique_ptr<SkCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*);

protected:

    Result onGetPixels(
            const SkImageInfo& dstInfo,
            void* dst, size_t dstRowBytes,
            const Options& options,
            int* rowsDecoded) override;

    SkEncodedImageFormat onGetEncodedFormat() const override {
        return SkEncodedImageFormat::kHEIF;
    }

private:

    /*
     * Read enough of the stream to initialize the SkHeifCodec.
     * Returns a bool representing success or failure.
     *
     * @param codecOut
     * If this returns true, and codecOut was not nullptr,
     * codecOut will be set to a new SkHeifCodec.
     *
     * @param stream
     * Deleted on failure.
     * codecOut will take ownership of it in the case where we created a codec.
     */
    static Result ReadHeader(SkStream* stream, SkCodec** codecOut);

    /*
     * Creates an instance of the decoder
     * Called only by NewFromStream
     */
    SkHeifCodec(int width, int height, const SkEncodedInfo&,
            HeifDecoder*, sk_sp<SkColorSpace>, Origin);

    /*
     * Checks if the conversion between the input image and the requested output
     * image has been implemented.
     *
     * Sets the output color format.
     */
    bool setOutputColorFormat(const SkImageInfo& dst);

    void initializeSwizzler(const SkImageInfo& dstInfo, const Options& options);
    void allocateStorage(const SkImageInfo& dstInfo);
    int readRows(const SkImageInfo& dstInfo, void* dst,
            size_t rowBytes, int count, const Options&);

    /*
     * Scanline decoding.
     */
    SkSampler* getSampler(bool createIfNecessary) override;
    Result onStartScanlineDecode(const SkImageInfo& dstInfo,
            const Options& options) override;
    int onGetScanlines(void* dst, int count, size_t rowBytes) override;
    bool onSkipScanlines(int count) override;

    std::unique_ptr<HeifDecoder>       fHeifDecoder;
    std::unique_ptr<HeifFrameInfo>     fFrameInfo;
    SkAutoTMalloc<uint8_t>             fStorage;
    uint8_t*                           fSwizzleSrcRow;
    uint32_t*                          fColorXformSrcRow;

    // In the case that heif decoder cannot take the exact the subset that
    // we need, we will use the swizzler to further subset the output.
    SkIRect                            fSwizzlerSubset;

    std::unique_ptr<SkSwizzler>        fSwizzler;

    typedef SkCodec INHERITED;
};

#endif // SkHeifCodec_DEFINED
