/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkHeifCodec_DEFINED
#define SkHeifCodec_DEFINED

#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkStream.h"
#include "src/codec/SkSwizzler.h"

#if __has_include("HeifDecoderAPI.h")
    #include "HeifDecoderAPI.h"
#else
    #include "src/codec/SkStubHeifDecoderAPI.h"
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

    bool conversionSupported(const SkImageInfo&, bool, bool) override;

    bool onRewind() override;

private:
    /*
     * Creates an instance of the decoder
     * Called only by NewFromStream
     */
    SkHeifCodec(SkEncodedInfo&&, HeifDecoder*, SkEncodedOrigin);

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
