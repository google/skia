/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/utils/SkTextLayoutJSON.h"
#include "src/utils/SkJSONWriter.h"

#include <algorithm>
#include <limits>
#include <string>
#include "src/utils/SkUTF.h"

bool SkTextLayoutJSON::writeInput(SkJSONWriter* JSONWriter, ParagraphImpl* paragraph) {
    JSONWriter->beginObject();
    JSONWriter->appendString("text", paragraph->text().data());
    JSONWriter->appendU32("dir", paragraph->paragraphStyle().getTextDirection() == TextDirection::kLtr ? 0 : 1);
    writeStyles(JSONWriter, paragraph);
    writeSkShaper(JSONWriter, paragraph);
    writeSkUnicode(JSONWriter, paragraph);
    JSONWriter->endObject();
    return true;
}

bool SkTextLayoutJSON::writeStyles(SkJSONWriter* JSONWriter, ParagraphImpl* paragraph) {

    JSONWriter->beginObject("Blocks");
    for (auto& s : paragraph->styles()) {
        JSONWriter->beginObject("block");
        JSONWriter->beginObject("range");
        JSONWriter->appendU64("start", s.fRange.start);
        JSONWriter->appendU64("end", s.fRange.end);
        JSONWriter->endObject();
        JSONWriter->beginObject("style");
        JSONWriter->appendU64("foreground", s.fStyle.getForeground().getColor());
        JSONWriter->appendU64("background", s.fStyle.getForeground().getColor());
        JSONWriter->beginArray("fonts");
        for (auto& ff : s.fStyle.getFontFamilies()) {
            JSONWriter->appendString(ff.c_str());
        }
        JSONWriter->endArray();
        JSONWriter->appendFloat("font_size", s.fStyle.getFontSize());
        JSONWriter->appendU32("font_weight", s.fStyle.getFontStyle().weight());
        JSONWriter->appendU32("font_slant", s.fStyle.getFontStyle().slant());
        JSONWriter->endObject();
        JSONWriter->endObject();
    }
    JSONWriter->endObject();
    return true;
}

bool SkTextLayoutJSON::writeSkShaper(SkJSONWriter* JSONWriter, ParagraphImpl* paragraph) {
    JSONWriter->beginObject("skshaper", true);

    for (auto& run : paragraph->runs()) {
        JSONWriter->beginObject("run", true);
        JSONWriter->appendU32("dir", run.leftToRight() ? 0 : 1);
        JSONWriter->beginObject("advance");
        JSONWriter->appendFloat("x", run.advance().fX);
        JSONWriter->appendFloat("y", run.advance().fY);
        JSONWriter->endObject();
        JSONWriter->beginObject("range");
        // TODO: It should be utf8Range, not textRange
        JSONWriter->appendU64("start", run.textRange().start);
        JSONWriter->appendU64("end", run.textRange().end);
        JSONWriter->endObject();

        JSONWriter->beginArray("positions");
        for (auto& pos : run.positions()) {
            JSONWriter->beginObject();
            JSONWriter->appendFloat("x", pos.fX);
            JSONWriter->appendFloat("y", pos.fY);
            JSONWriter->endObject();
        }
        JSONWriter->endArray();

        JSONWriter->beginArray("glyphs");
        for (auto glyph : run.glyphs()) {
            JSONWriter->appendS64(glyph);
        }
        JSONWriter->endArray();

        JSONWriter->beginArray("clusters");
        for (auto cluster : run.clusterIndexes()) {
            JSONWriter->appendS64(cluster);
        }
        JSONWriter->endArray();

        JSONWriter->endObject();
    }

    JSONWriter->endObject();
    return true;
}

bool SkTextLayoutJSON::writeSkUnicode(SkJSONWriter* JSONWriter, ParagraphImpl* paragraph) {

    auto unicode = paragraph->getUnicode();
    if (!unicode) {
        return false;
    }
    auto text = paragraph->text();

    JSONWriter->beginObject("skunicode", true);
    std::vector<SkUnicode::BidiRegion> bidis;
    auto textDirection = paragraph->paragraphStyle().getTextDirection() == TextDirection::kLtr
                          ? SkUnicode::TextDirection::kLTR
                          : SkUnicode::TextDirection::kRTL;
    if (!unicode->getBidiRegions(text.data(), text.size(), textDirection, &bidis)) {
        return false;
    }
    JSONWriter->beginArray("bidis");
    for (auto& bidi : bidis) {
        JSONWriter->beginObject();
        JSONWriter->appendU64("start", bidi.start);
        JSONWriter->appendU64("end", bidi.start);
        JSONWriter->appendU32("start", bidi.level);
        JSONWriter->endObject();
    }
    JSONWriter->endArray();

    std::vector<SkUnicode::Position> whitespaces;
    if (!unicode->getWhitespaces(text.data(), text.size(), &whitespaces)) {
        return false;
    }
    JSONWriter->beginArray("spaces");
    for (auto space : whitespaces) {
        JSONWriter->appendU64(space);
    }
    JSONWriter->endArray();

    std::vector<SkUnicode::LineBreakBefore> lineBreaks;
    if (!unicode->getLineBreaks(text.data(), text.size(), &lineBreaks)) {
        return false;
    }
    JSONWriter->beginArray("linebreaks");
    for (auto& linebreak : lineBreaks) {
        JSONWriter->beginObject();
        JSONWriter->appendU64("pos", linebreak.pos);
        JSONWriter->appendBool("type", linebreak.breakType == SkUnicode::LineBreakType::kHardLineBreak);
        JSONWriter->endObject();
    }
    JSONWriter->endArray();

    std::vector<SkUnicode::Position> graphemes;
    if (!unicode->getGraphemes(text.data(), text.size(), &graphemes)) {
        return false;
    }
    JSONWriter->beginArray("graphemes");
    for (auto grapheme : graphemes) {
        JSONWriter->appendU64(grapheme);
    }
    JSONWriter->endArray();

    JSONWriter->endObject();
    return true;
}

bool SkTextLayoutJSON::writeOutput(SkJSONWriter* JSONWriter, ParagraphImpl* paragraph) {
    return true;
}

bool SkTextLayoutJSON::paintOutput(SkCanvas* canvas, SkJSON* JSONReader) {
    return true;
}
