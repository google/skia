/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec.h"
#include "SkColorSpaceXform.h"
#include "SkColorTable.h"
#include "SkPngChunkReader.h"
#include "SkEncodedFormat.h"
#include "SkImageInfo.h"
#include "SkRefCnt.h"
#include "SkSwizzler.h"

class SkStream;

class SkPngCodec : public SkCodec {
public:
    static bool IsPng(const char*, size_t);

    // Assume IsPng was called and returned true.
    static SkCodec* NewFromStream(SkStream*, SkPngChunkReader* = NULL);

    virtual ~SkPngCodec();

protected:
    // We hold the png_ptr and info_ptr as voidp to avoid having to include png.h
    // or forward declare their types here.  voidp auto-casts to the real pointer types.
    struct voidp {
        voidp(void* ptr) : fPtr(ptr) {}

        template <typename T>
        operator T*() const { return (T*)fPtr; }

        explicit operator bool() const { return fPtr != nullptr; }

        void* fPtr;
    };

    Result onGetPixels(const SkImageInfo&, void*, size_t, const Options&, SkPMColor*, int*, int*)
            override;
    SkEncodedFormat onGetEncodedFormat() const override { return kPNG_SkEncodedFormat; }
    bool onRewind() override;
    uint64_t onGetFillValue(const SkImageInfo&) const override;

    // Helper to set up swizzler, color xforms, and color table. Also calls png_read_update_info.
    bool initializeXforms(const SkImageInfo& dstInfo, const Options&, SkPMColor* colorPtr,
                          int* colorCount);
    void initializeSwizzler(const SkImageInfo& dstInfo, const Options&);
    SkSampler* getSampler(bool createIfNecessary) override;
    void allocateStorage(const SkImageInfo& dstInfo);
    void applyXformRow(void* dst, const void* src, SkColorType, SkAlphaType, int width);

    virtual int readRows(const SkImageInfo& dstInfo, void* dst, size_t rowBytes, int count,
                         int startRow) = 0;

    SkPngCodec(const SkEncodedInfo&, const SkImageInfo&, SkStream*, SkPngChunkReader*,
               void* png_ptr, void* info_ptr, int, int);

    SkAutoTUnref<SkPngChunkReader> fPngChunkReader;
    voidp                          fPng_ptr;
    voidp                          fInfo_ptr;

    // These are stored here so they can be used both by normal decoding and scanline decoding.
    SkAutoTUnref<SkColorTable>         fColorTable;    // May be unpremul.
    SkAutoTDelete<SkSwizzler>          fSwizzler;
    std::unique_ptr<SkColorSpaceXform> fColorXform;
    SkAutoTMalloc<uint8_t>             fStorage;
    uint8_t*                           fSwizzlerSrcRow;
    uint32_t*                          fColorXformSrcRow;
    size_t                             fSrcRowBytes;

    const int                          fNumberPasses;
    int                                fBitDepth;

private:

    enum XformMode {
        // Requires only a swizzle pass.
        kSwizzleOnly_XformMode,

        // Requires only a color xform pass.
        kColorOnly_XformMode,

        // Requires a swizzle and a color xform.
        kSwizzleColor_XformMode,
    };

    bool createColorTable(const SkImageInfo& dstInfo, int* ctableCount);
    void destroyReadStruct();

    XformMode fXformMode;

    typedef SkCodec INHERITED;
};
