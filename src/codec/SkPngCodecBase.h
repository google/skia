/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPngCodecBase_DEFINED
#define SkPngCodecBase_DEFINED

#include <memory>

#include "include/codec/SkCodec.h"
#include "include/private/SkEncodedInfo.h"

class SkStream;
enum class SkEncodedImageFormat;

// This class implements functionality shared between `SkPngCodec` and
// `SkPngRustCodec` (the latter is from `experimental/rust_png`).
class SkPngCodecBase : public SkCodec {
public:
    ~SkPngCodecBase() override;

    static bool isCompatibleColorProfileAndType(const SkEncodedInfo::ICCProfile* profile,
                                                SkEncodedInfo::Color color);

protected:
    SkPngCodecBase(SkEncodedInfo&&, XformFormat srcFormat, std::unique_ptr<SkStream>);

    // TODO(https://crbug.com/356879515): Move `SkPngCodec::applyXformRow` here.

private:
    // SkCodec overrides:
    SkEncodedImageFormat onGetEncodedFormat() const final;
};

#endif  // SkPngCodecBase_DEFINED
