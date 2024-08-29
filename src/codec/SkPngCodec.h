/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPngCodec_DEFINED
#define SkPngCodec_DEFINED

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

#include "include/codec/SkCodec.h"
#include "include/core/SkRefCnt.h"
#include "src/codec/SkPngCodecBase.h"

class SkPngChunkReader;
class SkStream;
struct SkEncodedInfo;
struct SkImageInfo;
template <typename T> class SkSpan;

class SkPngCodec : public SkPngCodecBase {
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

    SkPngCodec(SkEncodedInfo&&,
               std::unique_ptr<SkStream>,
               SkPngChunkReader*,
               void* png_ptr,
               void* info_ptr);

    Result onGetPixels(const SkImageInfo&, void*, size_t, const Options&, int*)
            override;
    bool onRewind() override;

    voidp png_ptr() { return fPng_ptr; }
    voidp info_ptr() { return fInfo_ptr; }

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

private:
    // SkPngCodecBase overrides:
    std::optional<SkSpan<const PaletteColorEntry>> onTryGetPlteChunk() override;
    std::optional<SkSpan<const uint8_t>> onTryGetTrnsChunk() override;

    // Thin wrapper around `SkPngCodecBase::initializeXforms` that also sets up
    // some `libpng`-specific state.
    Result initializeXforms(const SkImageInfo& dstInfo, const Options&);

    void destroyReadStruct();

    virtual Result decodeAllRows(void* dst, size_t rowBytes, int* rowsDecoded) = 0;
    virtual void setRange(int firstRow, int lastRow, void* dst, size_t rowBytes) = 0;
    virtual Result decode(int* rowsDecoded) = 0;

    size_t                         fIdatLength;
    bool                           fDecodedIdat;
};
#endif  // SkPngCodec_DEFINED
