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
    static bool IsPng(SkStream*);

    // Assume IsPng was called and returned true.
    static SkCodec* NewFromStream(SkStream*);
    static SkScanlineDecoder* NewSDFromStream(SkStream*);

    virtual ~SkPngCodec();

protected:
    Result onGetPixels(const SkImageInfo&, void*, size_t, const Options&, SkPMColor*, int*)
            override;
    SkEncodedFormat onGetEncodedFormat() const override { return kPNG_SkEncodedFormat; }
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
    int                         fBitDepth;

    SkPngCodec(const SkImageInfo&, SkStream*, png_structp, png_infop, int);


    // Helper to set up swizzler and color table. Also calls png_read_update_info.
    Result initializeSwizzler(const SkImageInfo& requestedInfo, const Options&,
                              SkPMColor*, int* ctableCount);

    // Calls rewindIfNeeded and returns true if the decoder can continue.
    bool handleRewind();
    bool decodePalette(bool premultiply, int* ctableCount);
    void destroyReadStruct();

    friend class SkPngScanlineDecoder;
    friend class SkPngInterlacedScanlineDecoder;

    typedef SkCodec INHERITED;
};
