/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBmpBaseCodec_DEFINED
#define SkBmpBaseCodec_DEFINED

#include "SkBmpCodec.h"
#include "SkTemplates.h"

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
    SkBmpBaseCodec(int width, int height, const SkEncodedInfo& info, SkStream* stream,
                   uint16_t bitsPerPixel, SkCodec::SkScanlineOrder rowOrder);

    uint8_t* srcBuffer() { return reinterpret_cast<uint8_t*>(fSrcBuffer.get()); }

private:
    SkAutoFree fSrcBuffer;

    typedef SkBmpCodec INHERITED;
};
#endif // SkBmpBaseCodec_DEFINED
