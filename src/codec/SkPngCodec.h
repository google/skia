/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPngCodec_DEFINED
#define SkPngCodec_DEFINED

#include "include/codec/SkCodec.h"
#include "include/core/SkEncodedImageFormat.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPngChunkReader.h"
#include "include/core/SkRefCnt.h"
#include "src/codec/SkColorTable.h"
#include "src/codec/SkSwizzler.h"

class SkStream;

class SkPngCodec : public SkCodec {
public:
    static bool IsPng(const void*, size_t);

    // Assume IsPng was called and returned true.
    static std::unique_ptr<SkCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*,
                                                   SkPngChunkReader* = nullptr);

    // FIXME (scroggo): Temporarily needed by AutoCleanPng.
    void setIdatLength(size_t len) { fIdatLength = len; }

    ~SkPngCodec() override;

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

    SkPngCodec(SkEncodedInfo&&, std::unique_ptr<SkStream>, SkPngChunkReader*,
               void* png_ptr, void* info_ptr, int bitDepth);

    Result onGetPixels(const SkImageInfo&, void*, size_t, const Options&, int*)
            override;
    SkEncodedImageFormat onGetEncodedFormat() const override { return SkEncodedImageFormat::kPNG; }
    bool onRewind() override;

    SkSampler* getSampler(bool createIfNecessary) override;
    void applyXformRow(void* dst, const void* src);

    voidp png_ptr() { return fPng_ptr; }
    voidp info_ptr() { return fInfo_ptr; }

    SkSwizzler* swizzler() { return fSwizzler.get(); }

    // Initialize variables used by applyXformRow.
    void initializeXformParams();

    /**
     *  Pass available input to libpng to process it.
     *
     *  libpng will call any relevant callbacks installed. This will continue decoding
     *  until it reaches the end of the file, or until a callback tells libpng to stop.
     */
    bool processData();

    Result onStartIncrementalDecode(const SkImageInfo& dstInfo, void* pixels, size_t rowBytes,
            const SkCodec::Options&) override;
    Result onIncrementalDecode(int*) override;

    sk_sp<SkPngChunkReader>     fPngChunkReader;
    voidp                       fPng_ptr;
    voidp                       fInfo_ptr;

    // These are stored here so they can be used both by normal decoding and scanline decoding.
    sk_sp<SkColorTable>         fColorTable;    // May be unpremul.
    std::unique_ptr<SkSwizzler> fSwizzler;
    SkAutoTMalloc<uint8_t>      fStorage;
    void*                       fColorXformSrcRow;
    const int                   fBitDepth;

private:

    enum XformMode {
        // Requires only a swizzle pass.
        kSwizzleOnly_XformMode,

        // Requires only a color xform pass.
        kColorOnly_XformMode,

        // Requires a swizzle and a color xform.
        kSwizzleColor_XformMode,
    };

    bool createColorTable(const SkImageInfo& dstInfo);
    // Helper to set up swizzler, color xforms, and color table. Also calls png_read_update_info.
    SkCodec::Result initializeXforms(const SkImageInfo& dstInfo, const Options&);
    void initializeSwizzler(const SkImageInfo& dstInfo, const Options&, bool skipFormatConversion);
    void allocateStorage(const SkImageInfo& dstInfo);
    void destroyReadStruct();

    virtual Result decodeAllRows(void* dst, size_t rowBytes, int* rowsDecoded) = 0;
    virtual void setRange(int firstRow, int lastRow, void* dst, size_t rowBytes) = 0;
    virtual Result decode(int* rowsDecoded) = 0;

    XformMode                      fXformMode;
    int                            fXformWidth;

    size_t                         fIdatLength;
    bool                           fDecodedIdat;

    typedef SkCodec INHERITED;
};
#endif  // SkPngCodec_DEFINED
