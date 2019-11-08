/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFont.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "tests/Test.h"

static SkFont serialize_deserialize(const SkFont& font, skiatest::Reporter* reporter) {
    SkBinaryWriteBuffer wb;
    SkFontPriv::Flatten(font, wb);

    size_t size = wb.bytesWritten();
    SkAutoMalloc storage(size);
    wb.writeToMemory(storage.get());

    SkReadBuffer rb(storage.get(), size);

    SkFont clone;
    REPORTER_ASSERT(reporter, SkFontPriv::Unflatten(&clone, rb));
    return clone;
}

enum {
    kForceAutoHinting      = 1 << 0,
    kEmbeddedBitmaps       = 1 << 1,
    kSubpixel              = 1 << 2,
    kLinearMetrics         = 1 << 3,
    kEmbolden              = 1 << 4,
    kBaselineSnap          = 1 << 5,

    kAllBits = 0x3F,
};

static void apply_flags(SkFont* font, unsigned flags) {
    font->setForceAutoHinting(SkToBool(flags & kForceAutoHinting));
    font->setEmbeddedBitmaps( SkToBool(flags & kEmbeddedBitmaps));
    font->setSubpixel(        SkToBool(flags & kSubpixel));
    font->setLinearMetrics(   SkToBool(flags & kLinearMetrics));
    font->setEmbolden(        SkToBool(flags & kEmbolden));
    font->setBaselineSnap(    SkToBool(flags & kBaselineSnap));
}

DEF_TEST(Font_flatten, reporter) {
    const float sizes[] = {0, 0.001f, 1, 10, 10.001f, 100, 100000, 100000.01f};
    const float scales[] = {-5, -1, 0, 1, 5};
    const float skews[] = {-5, -1, 0, 1, 5};
    const SkFont::Edging edges[] = {
        SkFont::Edging::kAlias, SkFont::Edging::kAntiAlias, SkFont::Edging::kSubpixelAntiAlias
    };
    const SkFontHinting hints[] = {
        SkFontHinting::kNone, SkFontHinting::kSlight, SkFontHinting::kNormal, SkFontHinting::kFull
    };

    SkFont font;
    for (float size : sizes) {
        font.setSize(size);
        for (float scale : scales) {
            font.setScaleX(scale);
            for (float skew : skews) {
                font.setSkewX(skew);
                for (auto edge : edges) {
                    font.setEdging(edge);
                    for (auto hint : hints) {
                        font.setHinting(hint);
                        for (unsigned flags = 0; flags <= kAllBits; ++flags) {
                            apply_flags(&font, flags);

                            SkFont clone = serialize_deserialize(font, reporter);
                            REPORTER_ASSERT(reporter, font == clone);
                        }
                    }
                }
            }
        }
    }
}
