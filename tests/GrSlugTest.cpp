/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTextBlob.h"
#include "include/private/chromium/GrSlug.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrSlug_empty, reporter, ctxInfo) {
    auto dContext = ctxInfo.directContext();
    SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
    auto surface(SkSurface::MakeRenderTarget(dContext, SkBudgeted::kNo, info));
    auto canvas = surface->getCanvas();

    static const char* kText = " ";
    auto typeface = ToolUtils::create_portable_typeface("serif", SkFontStyle());
    SkFont font(typeface);
    size_t txtLen = strlen(kText);
    int glyphCount = font.countText(kText, txtLen, SkTextEncoding::kUTF8);

    SkTDArray<uint16_t> glyphs;
    glyphs.append(glyphCount);
    font.textToGlyphs(kText, txtLen, SkTextEncoding::kUTF8, glyphs.begin(), glyphCount);

    SkTextBlobBuilder builder;

    font.setSubpixel(true);
    font.setEdging(SkFont::Edging::kAntiAlias);
    font.setTypeface(typeface);
    font.setSize(16);

    const SkTextBlobBuilder::RunBuffer& buf = builder.allocRun(font, glyphs.count(), 0, 0);
    memcpy(buf.glyphs, glyphs.begin(), glyphs.count() * sizeof(uint16_t));
    auto blob = builder.make();

    SkPaint p;
    p.setAntiAlias(true);
    sk_sp<GrSlug> slug = GrSlug::ConvertBlob(canvas, *blob, {10, 10}, p);
    REPORTER_ASSERT(reporter, slug == nullptr);
}

