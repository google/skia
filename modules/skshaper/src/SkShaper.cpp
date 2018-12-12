/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkShaper.h"

SkShaper::LineHandler::Buffer SkTextBlobBuilderLineHandler::newLineBuffer(const SkFont& font,
                                                                          int count) {
    const auto& runBuffer = fBuilder.allocRunPos(font, count);
    return { runBuffer.glyphs, reinterpret_cast<SkPoint*>(runBuffer.pos), nullptr };
}

sk_sp<SkTextBlob> SkTextBlobBuilderLineHandler::makeBlob() {
    return fBuilder.make();
}
