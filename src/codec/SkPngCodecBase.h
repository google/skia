/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPngCodecBase_DEFINED
#define SkPngCodecBase_DEFINED

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

#include "include/codec/SkCodec.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTemplates.h"

class SkColorPalette;
class SkSampler;
class SkStream;
class SkSwizzler;
enum class SkEncodedImageFormat;
struct SkImageInfo;
template <typename T> class SkSpan;

// This class implements functionality shared between `SkPngCodec` and
// `SkPngRustCodec` (the latter is from `experimental/rust_png`).
class SkPngCodecBase : public SkCodec {
public:
    ~SkPngCodecBase() override;

    static bool isCompatibleColorProfileAndType(const SkEncodedInfo::ICCProfile* profile,
                                                SkEncodedInfo::Color color);
protected:
    SkPngCodecBase(SkEncodedInfo&&, std::unique_ptr<SkStream>);

    // Initialize most fields needed by `applyXformRow`.
    Result initializeXforms(const SkImageInfo& dstInfo, const Options& options);

    // Initialize other fields needed by `applyXformRow`.
    //
    // Needs to be called *after* (i.e. outside of) `onStartIncrementalDecode`.
    void initializeXformParams();

    // Transforms a decoded row into the `dstInfo` format that was earlier
    // passes to `initializeXforms`.
    void applyXformRow(SkSpan<uint8_t> dstRow, SkSpan<const uint8_t> srcRow);
    void applyXformRow(void* dstRow, const uint8_t* srcRow);

    // Gets the size of a decoded row in bytes - size of a row described by
    // `getEncodedInfo` and minimal size of `src` taken by `applyXformRow`.
    size_t getEncodedInfoRowSize();

    const SkSwizzler* swizzler() const { return fSwizzler.get(); }

    struct PaletteColorEntry {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    };
    virtual std::optional<SkSpan<const PaletteColorEntry>> onTryGetPlteChunk() = 0;
    virtual std::optional<SkSpan<const uint8_t>> onTryGetTrnsChunk() = 0;

private:
    // SkCodec overrides:
    SkEncodedImageFormat onGetEncodedFormat() const final;
    SkSampler* getSampler(bool createIfNecessary) final;

    void allocateStorage(const SkImageInfo& dstInfo);
    void initializeSwizzler(const SkImageInfo& dstInfo,
                            const Options& options,
                            bool skipFormatConversion);
    bool createColorTable(const SkImageInfo& dstInfo);

    enum XformMode {
        // Requires only a swizzle pass.
        kSwizzleOnly_XformMode,

        // Requires only a color xform pass.
        kColorOnly_XformMode,

        // Requires a swizzle and a color xform.
        kSwizzleColor_XformMode,
    };
    XformMode fXformMode;

    std::unique_ptr<SkSwizzler> fSwizzler;
    skia_private::AutoTMalloc<uint8_t> fStorage;
    int fXformWidth = -1;
    sk_sp<SkColorPalette> fColorTable;  // May be unpremul.

#if defined(SK_DEBUG)
    size_t fDstMinRowBytes = 0;  // Size of destination row in bytes.
#endif
};

#endif  // SkPngCodecBase_DEFINED
