/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCodec_wbmp_DEFINED
#define SkCodec_wbmp_DEFINED

#include "SkCodec.h"
#include "SkScanlineDecoder.h"
#include "SkSwizzler.h"

class SkWbmpCodec final : public SkCodec {
public:
    static bool IsWbmp(SkStream*);

    /*
     * Assumes IsWbmp was called and returned true
     * Creates a wbmp codec
     * Takes ownership of the stream
     */
    static SkCodec* NewFromStream(SkStream*);

    /*
     * Assumes IsWbmp was called and returned true
     * Creates a wbmp scanline decoder
     * Takes ownership of the stream
     */
    static SkScanlineDecoder* NewSDFromStream(SkStream*);
protected:
    SkEncodedFormat onGetEncodedFormat() const override;
    Result onGetPixels(const SkImageInfo&, void*, size_t,
                       const Options&, SkPMColor[], int*) override;
private:

    /*
     * Calls rewindIfNeeded and returns true if the decoder can continue
     */
    bool handleRewind();

    /*
     * Returns a swizzler on success, NULL on failure
     */
    SkSwizzler* initializeSwizzler(const SkImageInfo& info, const SkPMColor* ctable,
                                   const Options& opts);

    /*
     * Read a src row from the encoded stream
     */
    Result readRow(uint8_t* row);

    SkWbmpCodec(const SkImageInfo&, SkStream*);

    const size_t fSrcRowBytes;

    friend class SkWbmpScanlineDecoder;
    typedef SkCodec INHERITED;
};

#endif  // SkCodec_wbmp_DEFINED
