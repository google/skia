/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCodec_wbmp_DEFINED
#define SkCodec_wbmp_DEFINED

#include "SkCodec.h"
#include "SkSwizzler.h"

class SkWbmpCodec final : public SkCodec {
public:
    static bool IsWbmp(const void*, size_t);

    /*
     * Assumes IsWbmp was called and returned true
     * Creates a wbmp codec
     * Takes ownership of the stream
     */
    static SkCodec* NewFromStream(SkStream*);

protected:
    SkEncodedFormat onGetEncodedFormat() const override;
    Result onGetPixels(const SkImageInfo&, void*, size_t,
                       const Options&, SkPMColor[], int*, int*) override;
    bool onRewind() override;
private:
    /*
     * Returns a swizzler on success, nullptr on failure
     */
    SkSwizzler* initializeSwizzler(const SkImageInfo& info, const SkPMColor* ctable,
                                   const Options& opts);
    SkSampler* getSampler(bool createIfNecessary) override {
        SkASSERT(fSwizzler || !createIfNecessary);
        return fSwizzler;
    }

    /*
     * Read a src row from the encoded stream
     */
    bool readRow(uint8_t* row);

    SkWbmpCodec(const SkImageInfo&, SkStream*);

    const size_t                 fSrcRowBytes;

    // Used for scanline decodes:
    SkAutoTDelete<SkSwizzler>    fSwizzler;
    SkAutoTUnref<SkColorTable>   fColorTable;
    SkAutoTMalloc<uint8_t>       fSrcBuffer;

    // FIXME: Override onSkipScanlines to avoid swizzling.
    int onGetScanlines(void* dst, int count, size_t dstRowBytes) override;
    Result onStartScanlineDecode(const SkImageInfo& dstInfo, const Options& options,
            SkPMColor inputColorTable[], int* inputColorCount) override;

    typedef SkCodec INHERITED;
};

#endif  // SkCodec_wbmp_DEFINED
