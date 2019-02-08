/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkShaper.h"

#include "SkTextBlobPriv.h"

SkShaper::RunHandler::Buffer SkTextBlobBuilderRunHandler::newRunBuffer(const RunInfo&,
                                                                       const SkFont& font,
                                                                       int glyphCount,
                                                                       int textCount) {
    const auto& runBuffer = SkTextBlobBuilderPriv::AllocRunTextPos(&fBuilder, font, glyphCount,
                                                                   textCount, SkString());
    return { runBuffer.glyphs,
             runBuffer.points(),
             runBuffer.utf8text,
             runBuffer.clusters };
}

sk_sp<SkTextBlob> SkTextBlobBuilderRunHandler::makeBlob() {
    return fBuilder.make();
}

SkShaper::RunHandler::Buffer SkPrinterRunHandler::newRunBuffer(const RunInfo&,
                                                               const SkFont& font,
                                                               int glyphCount,
                                                               int textCount) {
  const auto& runBuffer = SkTextBlobBuilderPriv::AllocRunTextPos(&fBuilder, font, glyphCount,
                                                                 textCount, SkString());
  return { runBuffer.glyphs,
           runBuffer.points(),
           runBuffer.utf8text,
           runBuffer.clusters };
}

void SkPrinterRunHandler::addWord(const char* start, const char* end, SkPoint point, SkVector advance, SkScalar baseline) {
  SkString word(start, end - start);
  SkDebugf("Word: '%s'\n", word.c_str());
}

void SkPrinterRunHandler::addLine(const char* start, const char* end, SkPoint point, SkVector advance, size_t lineIndex, bool lastLine) {

  SkString line(start, end - start);
  SkDebugf("Line #%d: '%s'\n", lineIndex, line.c_str());
}

sk_sp<SkTextBlob> SkPrinterRunHandler::makeBlob() {
  return fBuilder.make();
}