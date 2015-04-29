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

#ifdef SKIA_PNG_PREFIXED
    // this must proceed png.h
    #include "pngprefix.h"
#endif
#include "png.h"

class SkScanlineDecoder;
class SkStream;

class SkPngCodec : public SkCodec {
public:
    // Assumes IsPng was called and returned true.
    static SkCodec* NewFromStream(SkStream*);
    static bool IsPng(SkStream*);
protected:
    Result onGetPixels(const SkImageInfo&, void*, size_t, const Options&, SkPMColor*, int*)
            override;
    SkEncodedFormat onGetEncodedFormat() const override { return kPNG_SkEncodedFormat; }
    SkScanlineDecoder* onGetScanlineDecoder(const SkImageInfo& dstInfo, const Options& options,
                                            SkPMColor ctable[], int* ctableCount) override;
    bool onReallyHasAlpha() const override { return fReallyHasAlpha; }
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
                              size_t rowBytes, const Options&, SkPMColor*, int* ctableCount);
    // Calls rewindIfNeeded, and returns true if the decoder can continue.
    bool handleRewind();
    bool decodePalette(bool premultiply, int bitDepth, int* ctableCount);
    void finish();
    void destroyReadStruct();

    friend class SkPngScanlineDecoder;

    typedef SkCodec INHERITED;
};
