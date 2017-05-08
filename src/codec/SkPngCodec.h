/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPngCodec_DEFINED
#define SkPngCodec_DEFINED

#include "SkCodec.h"
#include "SkColorSpaceXform.h"
#include "SkColorTable.h"
#include "SkPngChunkReader.h"
#include "SkEncodedImageFormat.h"
#include "SkImageInfo.h"
#include "SkRefCnt.h"
#include "SkSwizzler.h"

// FIXME (scroggo): GOOGLE3 is currently using an outdated version of libpng,
// so we need to work around the lack of the method png_process_data_pause.
// This code will be unnecessary once we update GOOGLE3. It would make more
// sense to condition this on the version of libpng being used, but we do not
// know that here because png.h is only included by the cpp file.
#define SK_GOOGLE3_PNG_HACK

class SkStream;

class SkPngCodec : public SkCodec {
public:
    static bool IsPng(const char*, size_t);

    // Assume IsPng was called and returned true.
    static SkCodec* NewFromStream(SkStream*, SkPngChunkReader* = NULL);

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

    SkPngCodec(const SkEncodedInfo&, const SkImageInfo&, SkStream*, SkPngChunkReader*,
            void* png_ptr, void* info_ptr, int bitDepth);

    Result onGetPixels(const SkImageInfo&, void*, size_t, const Options&, SkPMColor*, int*, int*)
            override;
    SkEncodedImageFormat onGetEncodedFormat() const override { return SkEncodedImageFormat::kPNG; }
    bool onRewind() override;
    uint64_t onGetFillValue(const SkImageInfo&) const override;

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
    void processData();

#ifdef SK_GOOGLE3_PNG_HACK
    // In libpng 1.2.56, png_process_data_pause does not exist, so when we wanted to
    // read the header, we may have read too far. In that case, we need to delete the
    // png_ptr and info_ptr and recreate them. This method does that (and attaches the
    // chunk reader.
    bool rereadHeaderIfNecessary();

    // This method sets up the new png_ptr/info_ptr (created in rereadHeaderIfNecessary)
    // the way we set up the old one the first time in AutoCleanPng.decodeBounds's callback.
    void rereadInfoCallback();
#endif

    Result onStartIncrementalDecode(const SkImageInfo& dstInfo, void* pixels, size_t rowBytes,
            const SkCodec::Options&,
            SkPMColor* ctable, int* ctableCount) override;
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

    bool createColorTable(const SkImageInfo& dstInfo, int* ctableCount);
    // Helper to set up swizzler, color xforms, and color table. Also calls png_read_update_info.
    bool initializeXforms(const SkImageInfo& dstInfo, const Options&, SkPMColor* colorPtr,
                          int* colorCount);
    void initializeSwizzler(const SkImageInfo& dstInfo, const Options&, bool skipFormatConversion);
    void allocateStorage(const SkImageInfo& dstInfo);
    void destroyReadStruct();

    virtual Result decodeAllRows(void* dst, size_t rowBytes, int* rowsDecoded) = 0;
    virtual void setRange(int firstRow, int lastRow, void* dst, size_t rowBytes) = 0;
    virtual Result decode(int* rowsDecoded) = 0;

    XformMode                      fXformMode;
    SkColorSpaceXform::ColorFormat fXformColorFormat;
    SkAlphaType                    fXformAlphaType;
    int                            fXformWidth;

#ifdef SK_GOOGLE3_PNG_HACK
    bool        fNeedsToRereadHeader;
#endif

    typedef SkCodec INHERITED;
};
#endif  // SkPngCodec_DEFINED
