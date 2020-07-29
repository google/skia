/*
 * Copyright 2020 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/svg/model/SkSVGDOM.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkPicture.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "tools/ToolUtils.h"
#include "tools/flags/CommandLineFlags.h"

#include <iostream>

SkBitmap FuzzSKP(sk_sp<SkData> bytes) {
    SkReadBuffer buf(bytes->data(), bytes->size());
    SkDebugf("Decoding\n");
    sk_sp<SkPicture> pic(SkPicturePriv::MakeFromBuffer(buf));
    if (!pic) {
        SkDebugf("[terminated] Couldn't decode as a picture.\n");
        return {};
    }
    SkDebugf("Rendering\n");

    SkBitmap bitmap;
    SkIRect size = pic->cullRect().roundOut();
    bitmap.allocN32Pixels(size.width(), size.height());
    SkCanvas canvas(bitmap);
    canvas.drawPicture(pic);
    SkDebugf("[terminated] Success! Decoded and rendered an SkPicture!\n");

    pic->approximateBytesUsed();
    pic->approximateOpCount();
    return bitmap;
}

#if defined(IS_FUZZING_WITH_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    auto bytes = SkData::MakeWithoutCopy(data, size);
    FuzzSKP(bytes);
    return 0;
}
#endif
