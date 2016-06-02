/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec.h"
#include "SkColorTable.h"
#include "SkPngChunkReader.h"
#include "SkEncodedFormat.h"
#include "SkImageInfo.h"
#include "SkRefCnt.h"
#include "SkSwizzler.h"

#include "png.h"

class SkStream;

class SkPngCodec : public SkCodec {
public:
    static bool IsPng(const char*, size_t);

    // Assume IsPng was called and returned true.
    static SkCodec* NewFromStream(SkStream*, SkPngChunkReader* = NULL);

    virtual ~SkPngCodec();

protected:
    SkPngCodec(int width, int height, const SkEncodedInfo&, SkStream*, SkPngChunkReader*,
            png_structp, png_infop, int bitDepth, sk_sp<SkColorSpace>);

    Result onGetPixels(const SkImageInfo&, void*, size_t, const Options&, SkPMColor*, int*, int*)
            override;
    SkEncodedFormat onGetEncodedFormat() const override { return kPNG_SkEncodedFormat; }
    bool onRewind() override;
    uint32_t onGetFillValue(SkColorType) const override;

    // Helper to set up swizzler and color table. Also calls png_read_update_info.
    bool initializeSwizzler(const SkImageInfo& requestedInfo, const Options&,
                            SkPMColor*, int* ctableCount);
    SkSampler* getSampler(bool createIfNecessary) override {
        SkASSERT(fSwizzler);
        return fSwizzler;
    }

    png_structp png_ptr() { return fPng_ptr; }
    png_infop info_ptr() { return fInfo_ptr; }
    SkSwizzler* swizzler() { return fSwizzler; }

    /**
     *  Pass available input to libpng to process it.
     *
     *  libpng will call any relevant callbacks installed. This will continue decoding
     *  until it reaches the end of the file, or until a callback tells libpng to stop.
     */
    void processData();

    Result onStartIncrementalDecode(const SkImageInfo& dstInfo, void* pixels, size_t rowBytes,
            const SkCodec::Options&,
            SkPMColor* ctable, int* ctableCount) override;
    Result onIncrementalDecode(int*) override;

private:
    SkAutoTUnref<SkPngChunkReader>  fPngChunkReader;
    png_structp                     fPng_ptr;
    png_infop                       fInfo_ptr;

    SkAutoTUnref<SkColorTable>      fColorTable;    // May be unpremul.
    SkAutoTDelete<SkSwizzler>       fSwizzler;

    const int                       fBitDepth;

    bool createColorTable(SkColorType dstColorType, bool premultiply, int* ctableCount);
    void destroyReadStruct();

    virtual Result decodeAllRows(void* dst, size_t rowBytes, int* rowsDecoded) = 0;
    virtual void setRange(int firstRow, int lastRow, void* dst, size_t rowBytes) = 0;
    virtual Result decode(int* rowsDecoded) = 0;

    typedef SkCodec INHERITED;
};
