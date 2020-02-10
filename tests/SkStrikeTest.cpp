/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFont.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeSpec.h"
#include "tools/ToolUtils.h"

#include "tests/Test.h"

DEF_TEST(SkStrikeMultiThread, Reporter) {
sk_sp<SkTypeface> typefaces[] = {
        ToolUtils::create_portable_typeface("serif", SkFontStyle::Italic()),
        ToolUtils::create_portable_typeface("sans-serif", SkFontStyle::Italic())};

    SkDrawableGlyphBuffer drawble;
    SkSourceGlyphBuffer rejects;

    auto perThread = [&](int threadIndex) {
        SkFont font;
        font.setEdging(SkFont::Edging::kAntiAlias);
        font.setSubpixel(true);
        auto typeface = typefaces[threadIndex % 2];
        font.setTypeface(typeface);

        SkPackedGlyphID glyphs['z'];
        for (int c = ' '; c < 'z'; c++) {
            glyphs[c] = SkPackedGlyphID{font.unicharToGlyph(c)};
        }
        constexpr size_t glyphCount = 'z' - ' ';
        SkSpan<const SkPackedGlyphID> glyphIDs{&glyphs[SkTo<int>(' ')], glyphCount};

        rejects.setSource(glyphRun.source());

        SkPaint defaultPaint;
        SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(
                font, defaultPaint, SkSurfaceProps(0, kUnknown_SkPixelGeometry),
                SkScalerContextFlags::kNone, SkMatrix::I());

        SkScalerContextEffects effects;
        std::unique_ptr<SkScalerContext> ctx{typeface->createScalerContext(effects, &strikeSpec.descriptor())};
        SkStrike strike{strikeSpec.descriptor(), std::move(ctx)};

        drawble.startDevice(rejects.source(), {0, 0}, SkMatrix::I(), strike.roundingSpec());
        strike.prepareForMaskDrawing(&drawble, &rejects);
        rejects.flipRejectsToSource();
    };


}
