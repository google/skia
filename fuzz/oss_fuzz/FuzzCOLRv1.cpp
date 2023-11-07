/*
 * Copyright 2022 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkFontMgrPriv.h"

#include <algorithm>

void FuzzCOLRv1(const uint8_t* data, size_t size) {
    // We do not want the portable fontmgr here, as it does not allow creation of fonts from bytes.
    gSkFontMgr_DefaultFactory = nullptr;
    std::unique_ptr<SkStreamAsset> stream = SkMemoryStream::MakeDirect(data, size);
    sk_sp<SkTypeface> typeface = SkTypeface::MakeFromStream(std::move(stream));

    if (!typeface) {
        return;
    }

    auto s = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(128, 128));
    if (!s) {
        return;
    }

    // Place at a baseline in the lower part of the canvas square, but canvas size and baseline
    // placement are chosen arbitrarily and we just need to cover colrv1 rendering in this
    // fuzz test.
    SkFont colrv1Font = SkFont(typeface, 120);
    SkCanvas* canvas = s->getCanvas();
    SkPaint paint;
    int numGlyphs = typeface->countGlyphs();

    for (int i = 0; i < std::min(numGlyphs, 10); ++i) {
        SkPoint origin = SkPoint::Make(10, 108);
        SkPoint position = SkPoint::Make(0, 0);
        SkGlyphID glyphId = i;
        canvas->drawGlyphs(1, &glyphId, &position, origin, colrv1Font, paint);
    }
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    // COLRv1 corpus file sizes range from 8k for a small test glyph file to about 80k covering
    // most of the complex Noto Emoji glyphs, compare:
    // See https://github.com/googlefonts/color-fonts/blob/main/rebuild_fuzzer_corpus.py
    if (size > 80 * 1024) {
        return 0;
    }
    FuzzCOLRv1(data, size);
    return 0;
}
#endif
