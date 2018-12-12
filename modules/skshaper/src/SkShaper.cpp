/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkShaper.h"

#include "SkTextBlobPriv.h"

SkShaper::LineHandler::Buffer SkTextBlobBuilderLineHandler::newLineBuffer(const SkFont& font,
                                                                          int glyphCount,
                                                                          int textCount) {
    const auto& runBuffer = SkTextBlobBuilderPriv::AllocRunTextPos(&fBuilder, font, glyphCount,
                                                                   textCount, SkString());
    return { runBuffer.glyphs,
             reinterpret_cast<SkPoint*>(runBuffer.pos),
             runBuffer.utf8text,
             runBuffer.clusters };
}

sk_sp<SkTextBlob> SkTextBlobBuilderLineHandler::makeBlob() {
    return fBuilder.make();
}
