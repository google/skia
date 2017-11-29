/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "Test.h"
#include "SkCodec.h"
#include "SkImage.h"
#include "SkImageEncoder.h"
#include "SkStream.h"
#include "SkAutoPixmapStorage.h"
#include "sk_tool_utils.h"

DEF_TEST(DumbCodec, r) {
    auto src = GetResourceAsImage("mandrill_64.png");

    const SkColorType cts[] = {
        kRGBA_8888_SkColorType, kBGRA_8888_SkColorType, kRGBA_F16_SkColorType,
//        kAlpha_8_SkColorType,
        kRGB_565_SkColorType, kARGB_4444_SkColorType,
    };

    SkAutoPixmapStorage storage;
    for (auto ct : cts) {
        SkImageInfo info = SkImageInfo::Make(src->width(), src->height(), ct, kOpaque_SkAlphaType);
        storage.alloc(info);
        REPORTER_ASSERT(r, src->readPixels(storage, 0, 0));

        SkDynamicMemoryWStream stream;
        REPORTER_ASSERT(r, SkEncodeImage(&stream, storage, SkEncodedImageFormat::kDUMB, 100));
        auto codec = SkCodec::MakeFromData(stream.detachAsData());
        REPORTER_ASSERT(r, codec);

        SkAutoPixmapStorage dstStorage;
        dstStorage.alloc(codec->getInfo());
        REPORTER_ASSERT(r, codec->getPixels(dstStorage) == SkCodec::kSuccess);

        REPORTER_ASSERT(r, sk_tool_utils::equal_pixels(storage, dstStorage));
    }

}
