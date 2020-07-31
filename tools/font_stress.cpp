/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkTemplates.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkTaskGroup.h"
#include "tools/flags/CommandLineFlags.h"

#include <vector>

void stress(sk_sp<SkData> font, SkRandom* rng) {
    auto tf = SkTypeface::MakeFromData(std::move(font));
    if (!tf) {
        fprintf(stderr, "Failed to create typeface!\n");
        exit(-1);
    }

    const auto glyph_count = tf->countGlyphs();
    const size_t kChunkSize = 16;

    SkAutoTMalloc<SkGlyphID> glyphs(kChunkSize);
    SkAutoTMalloc<SkScalar>  widths(kChunkSize);
    // SkAutoTMalloc<SkRect>    bounds(glyph_count);

    for (size_t i = 0; i < kChunkSize; ++i) {
        glyphs[i] = rng->nextRangeU(0, glyph_count + 1);
    }

    static std::vector<float> sizes = { 1, 10, 20, 50 };
    for (const auto sz : sizes) {
        SkFont font(tf, sz);
        font.getWidthsBounds(glyphs.get(), kChunkSize, widths.get(), nullptr, nullptr);
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [FONT_FILE]...\n", argv[0]);
        return -1;
    }

    std::vector<sk_sp<SkData>> fonts;

    for (int i = 1; i < argc; ++i) {
        auto font = SkData::MakeFromFileName(argv[i]);
        if (!font) {
            fprintf(stderr, "Could not load %s. Skipping.\n", argv[i]);
            continue;
        }

        printf("Loaded %s\n", argv[i]);
        fonts.push_back(std::move(font));
    }

    if (fonts.empty()) {
        return 0;
    }

    printf("starting %zu threads...\n", fonts.size());
    SkTaskGroup::Enabler enabler(fonts.size());

    SkTaskGroup tg;
    tg.batch(fonts.size(), [&](int i) {
        SkRandom rng;
        while (true) {
            stress(fonts[i], &rng);
        }
    });
    tg.wait();

    return 0;
}
