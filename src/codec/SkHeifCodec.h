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
#include "SkEncodedOrigin.h"
#include "SkImageInfo.h"
#include "SkSwizzler.h"
#include "SkStream.h"

#if !defined(__has_include)
    #define __has_include(x) 0
#endif

#if __has_include("HeifDecoderAPI.h")
    #include "HeifDecoderAPI.h"
#else
    #include "SkStubHeifDecoderAPI.h"
#endif

class SkHeifCodec : public SkCodec {
public:
    static bool IsHeif(const void*, size_t);

    /*
     * Assumes IsHeif was called and returned true.
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

    bool conversionSupported(const SkImageInfo&, SkColorType, bool,
                             const SkColorSpace*) const override {
        // This class checks for conversion after creating colorXform.
        return true;
    }

private:
    /*
     * Creates an instance of the decoder
     * Called only by NewFromStream
     */
    SkHeifCodec(int width, int height, const SkEncodedInfo&,
            HeifDecoder*, sk_sp<SkColorSpace>, SkEncodedOrigin);

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
    HeifFrameInfo                      fFrameInfo;
    SkAutoTMalloc<uint8_t>             fStorage;
    uint8_t*                           fSwizzleSrcRow;
    uint32_t*                          fColorXformSrcRow;

    std::unique_ptr<SkSwizzler>        fSwizzler;

    typedef SkCodec INHERITED;
};

#endif // SkHeifCodec_DEFINED
