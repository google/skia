/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkDataTable.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkStream.h"
#include "include/encode/SkPngEncoder.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include <memory>

DEF_TEST(AlphaEncodedInfo, r) {
    auto codec = SkCodec::MakeFromStream(GetResourceAsStream("images/grayscale.jpg"));
    REPORTER_ASSERT(r, codec->getInfo().colorType() == kGray_8_SkColorType);

    SkBitmap bm;
    bm.allocPixels(codec->getInfo().makeColorType(kAlpha_8_SkColorType).makeColorSpace(nullptr));
    auto result = codec->getPixels(codec->getInfo(), bm.getPixels(), bm.rowBytes());
    REPORTER_ASSERT(r, result == SkCodec::kSuccess);

    SkDynamicMemoryWStream stream;
    REPORTER_ASSERT(r, SkPngEncoder::Encode(&stream, bm.pixmap(), {}));
    REPORTER_ASSERT(r, stream.bytesWritten() > 0);

    codec = SkCodec::MakeFromData(stream.detachAsData());
    REPORTER_ASSERT(r, codec);
    // TODO: Make SkEncodedInfo public and compare to its version of kAlpha_8.
    REPORTER_ASSERT(r, codec->getInfo().colorType() == kAlpha_8_SkColorType);

    SkBitmap bm2;
    bm2.allocPixels(codec->getInfo().makeColorSpace(nullptr));
    result = codec->getPixels(bm2.pixmap());
    REPORTER_ASSERT(r, result == SkCodec::kSuccess);

    REPORTER_ASSERT(r, ToolUtils::equal_pixels(bm.pixmap(), bm2.pixmap()));
}
