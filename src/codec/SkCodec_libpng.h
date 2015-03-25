/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec.h"
#include "SkColorTable.h"
#include "SkEncodedFormat.h"
#include "SkImageInfo.h"
#include "SkRefCnt.h"
#include "SkSwizzler.h"

extern "C" {
    #include "png.h"
}

class SkScanlineDecoder;
class SkStream;

class SkPngCodec : public SkCodec {
public:
    // Assumes IsPng was called and returned true.
    static SkCodec* NewFromStream(SkStream*);
    static bool IsPng(SkStream*);
protected:
    Result onGetPixels(const SkImageInfo&, void*, size_t, const Options&, SkPMColor*, int*)
            SK_OVERRIDE;
    SkEncodedFormat onGetEncodedFormat() const SK_OVERRIDE { return kPNG_SkEncodedFormat; }
    SkScanlineDecoder* onGetScanlineDecoder(const SkImageInfo& dstInfo) SK_OVERRIDE;
    bool onReallyHasAlpha() const SK_OVERRIDE { return fReallyHasAlpha; }
private:
    png_structp                 fPng_ptr;
    png_infop                   fInfo_ptr;

    // These are stored here so they can be used both by normal decoding and scanline decoding.
    SkAutoTUnref<SkColorTable>  fColorTable;    // May be unpremul.
    SkAutoTDelete<SkSwizzler>   fSwizzler;

    SkSwizzler::SrcConfig       fSrcConfig;
    int                         fNumberPasses;
    bool                        fReallyHasAlpha;

    SkPngCodec(const SkImageInfo&, SkStream*, png_structp, png_infop);
    ~SkPngCodec();

    // Helper to set up swizzler and color table. Also calls png_read_update_info.
    Result initializeSwizzler(const SkImageInfo& requestedInfo, void* dst,
                              size_t rowBytes, const Options&);
    bool decodePalette(bool premultiply);
    void finish();

    friend class SkPngScanlineDecoder;

    typedef SkCodec INHERITED;
};
