/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBmpBaseCodec_DEFINED
#define SkBmpBaseCodec_DEFINED

#include "include/codec/SkCodec.h"
#include "include/private/base/SkTemplates.h"
#include "src/codec/SkBmpCodec.h"

#include <cstdint>
#include <memory>

class SkStream;
struct SkEncodedInfo;

/*
 * Common base class for SkBmpStandardCodec and SkBmpMaskCodec.
 */
class SkBmpBaseCodec : public SkBmpCodec {
public:
    ~SkBmpBaseCodec() override;

    /*
     * Whether fSrcBuffer was successfully created.
     *
     * If false, this Codec must not be used.
     */
    bool didCreateSrcBuffer() const { return fSrcBuffer != nullptr; }

protected:
    SkBmpBaseCodec(SkEncodedInfo&& info, std::unique_ptr<SkStream>,
                   uint16_t bitsPerPixel, SkCodec::SkScanlineOrder rowOrder);

    uint8_t* srcBuffer() { return reinterpret_cast<uint8_t*>(fSrcBuffer.get()); }

private:
    skia_private::UniqueVoidPtr fSrcBuffer;

    using INHERITED = SkBmpCodec;
};
#endif // SkBmpBaseCodec_DEFINED
