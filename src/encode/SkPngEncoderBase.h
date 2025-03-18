/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPngEncoderBase_DEFINED
#define SkPngEncoderBase_DEFINED

#include <cstddef>
#include <cstdint>

#include <optional>

#include "include/encode/SkEncoder.h"
#include "include/private/SkEncodedInfo.h"
#include "src/encode/SkImageEncoderFns.h"

struct SkImageInfo;
class SkPixmap;
template <typename T> class SkSpan;

// This class implements functionality shared between `SkPngEncoderImpl` and
// `SkPngRustEncoderImpl` (the latter is from `experimental/rust_png`).
class SkPngEncoderBase : public SkEncoder {
public:
    struct TargetInfo {
        SkEncodedInfo fDstInfo;
        transform_scanline_proc fTransformProc;
        size_t fDstRowSize;
    };

    // Gets the `fDstInfo` that `srcInfo` should be converted into before
    // encoding and a `fTransformProc` that can transform source rows into
    // ready-to-encode rows (and the `fDstRowSize` of such rows).
    //
    // For example, `kRGBA_F32_SkColorType` source will be encoded as
    // `SkEncodedInfo::kRGBA_Color` with 16 `bitsPerComponent`.  Depending on
    // `src`'s alpha type, such transformation can be handled by either
    // `transform_scanline_F32` or `transform_scanline_F32_premul`.
    //
    // Returns `std::nullopt` if `srcInfo` is not supported by the PNG encoder.
    static std::optional<TargetInfo> getTargetInfo(const SkImageInfo& srcInfo);

protected:
    SkPngEncoderBase(TargetInfo targetInfo, const SkPixmap& src);

    // SkEncoder override:
    bool onEncodeRows(int numRows) final;

    // Called from `onEncodeRows` to encode the given `row` (in `dstInfo` format
    // that was passed to the `SkPngEncoderBase`'s constructor).
    virtual bool onEncodeRow(SkSpan<const uint8_t> row) = 0;

    // Called from `onEncodeRows` to finalize the encoded PNG (e.g. write the
    // `IEND` chunk).
    virtual bool onFinishEncoding() = 0;

private:
    TargetInfo fTargetInfo;
    bool fFinishedEncoding = false;
};

#endif  // SkPngEncoderBase_DEFINED
