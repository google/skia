/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCodec_wbmp_DEFINED
#define SkCodec_wbmp_DEFINED

#include "SkCodec.h"
#include "SkColorSpace.h"
#include "SkSwizzler.h"

class SkWbmpCodec final : public SkCodec {
public:
    static bool IsWbmp(const void*, size_t);

    /*
     * Assumes IsWbmp was called and returned true
     * Creates a wbmp codec
     * Takes ownership of the stream
     */
    static std::unique_ptr<SkCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*);

protected:
    SkEncodedImageFormat onGetEncodedFormat() const override;
    Result onGetPixels(const SkImageInfo&, void*, size_t,
                       const Options&, int*) override;
    bool onRewind() override;
    bool conversionSupported(const SkImageInfo& dst, SkColorType srcColor,
                             bool srcIsOpaque, const SkColorSpace* srcCS) const override;
    // No need to Xform; all pixels are either black or white.
    bool usesColorXform() const override { return false; }
private:
    /*
     * Returns a swizzler on success, nullptr on failure
     */
    SkSwizzler* initializeSwizzler(const SkImageInfo& info,
                                   const Options& opts);
    SkSampler* getSampler(bool createIfNecessary) override {
        SkASSERT(fSwizzler || !createIfNecessary);
        return fSwizzler.get();
    }

    /*
     * Read a src row from the encoded stream
     */
    bool readRow(uint8_t* row);

    SkWbmpCodec(int width, int height, const SkEncodedInfo&, std::unique_ptr<SkStream>);

    const size_t                fSrcRowBytes;

    // Used for scanline decodes:
    std::unique_ptr<SkSwizzler> fSwizzler;
    SkAutoTMalloc<uint8_t>      fSrcBuffer;

    int onGetScanlines(void* dst, int count, size_t dstRowBytes) override;
    bool onSkipScanlines(int count) override;
    Result onStartScanlineDecode(const SkImageInfo& dstInfo,
            const Options& options) override;

    typedef SkCodec INHERITED;
};

#endif  // SkCodec_wbmp_DEFINED
