/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/utils/TextEditor.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkM44.h"
#include "include/core/SkPath.h"
#include "include/core/SkString.h"
#include "include/private/base/SkAssert.h"
#include "src/base/SkUTF.h"

namespace skottie_utils {

namespace {

SkPath make_cursor_path() {
    // Normalized values, relative to text/font size.
    constexpr float kWidth  = 0.2f,
                    kHeight = 0.75f;

    SkPath p;

    p.lineTo(kWidth  , 0);
    p.moveTo(kWidth/2, 0);
    p.lineTo(kWidth/2, kHeight);
    p.moveTo(0       , kHeight);
    p.lineTo(kWidth  , kHeight);

    return p;
}

size_t next_utf8(const SkString& str, size_t index) {
    SkASSERT(index < str.size());

    const char* utf8_ptr = str.c_str() + index;

    if (SkUTF::NextUTF8(&utf8_ptr, str.c_str() + str.size()) < 0){
        // Invalid UTF sequence.
        return index;
    }

    return utf8_ptr - str.c_str();
}

size_t prev_utf8(const SkString& str, size_t index) {
    SkASSERT(index > 0);

    // Find the previous utf8 index by probing the preceding 4 offsets.  Utf8 leading bytes are
    // always distinct from continuation bytes, so only one of these probes will succeed.
    for (unsigned i = 1; i <= SkUTF::kMaxBytesInUTF8Sequence && i <= index; ++i) {
        const char* utf8_ptr = str.c_str() + index - i;
        if (SkUTF::NextUTF8(&utf8_ptr, str.c_str() + str.size()) >= 0) {
            return index - i;
        }
    }

    // Invalid UTF sequence.
    return index;
}

} // namespace

TextEditor::TextEditor(
        std::unique_ptr<skottie::TextPropertyHandle>&& prop,
        std::vector<std::unique_ptr<skottie::TextPropertyHandle>>&& deps)
    : fTextProp(std::move(prop))
    , fDependentProps(std::move(deps))
    , fCursorPath(make_cursor_path())
    , fCursorBounds(fCursorPath.computeTightBounds())
{}

TextEditor::~TextEditor() = default;

void TextEditor::toggleEnabled() {
    fEnabled = !fEnabled;

    auto txt = fTextProp->get();
    txt.fDecorator = fEnabled ? sk_ref_sp(this) : nullptr;
    fTextProp->set(txt);

    if (fEnabled) {
        // Always reset the cursor position to the end.
        fCursorIndex = txt.fText.size();
    }

    fTimeBase = std::chrono::steady_clock::now();
}

void TextEditor::setEnabled(bool enabled) {
    if (enabled != fEnabled) {
        this->toggleEnabled();
    }
}

std::tuple<size_t, size_t> TextEditor::currentSelection() const {
    // Selection can be inverted.
    return std::make_tuple(std::min(std::get<0>(fSelection), std::get<1>(fSelection)),
                           std::max(std::get<0>(fSelection), std::get<1>(fSelection)));
}

size_t TextEditor::closestGlyph(const SkPoint& pt) const {
    float  min_distance = std::numeric_limits<float>::max();
    size_t min_index    = 0;

    for (size_t i = 0; i < fGlyphData.size(); ++i) {
        const auto dist = (fGlyphData[i].fDevBounds.center() - pt).length();
        if (dist < min_distance) {
            min_distance = dist;
            min_index = i;
        }
    }

    return min_index;
}

void TextEditor::drawCursor(SkCanvas* canvas, const TextInfo& tinfo) const {
    constexpr double kCursorHz = 2;
    const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::steady_clock::now() - fTimeBase).count();
    const long cycle = static_cast<long>(static_cast<double>(now_ms) * 0.001 * kCursorHz);
    if (cycle & 1) {
        // blink
        return;
    }

    auto txt_prop  = fTextProp->get();

    const auto glyph_index = [&]() -> size_t {
        if (!fCursorIndex) {
            return 0;
        }

        const auto prev_index = prev_utf8(txt_prop.fText, fCursorIndex);
        for (size_t i = 0; i < tinfo.fGlyphs.size(); ++i) {
            if (tinfo.fGlyphs[i].fCluster >= prev_index) {
                return i;
            }
        }

        return tinfo.fGlyphs.size() - 1;
    }();

    // Cursor index mapping:
    //   0 -> before the first char
    //   1 -> after the first char
    //   2 -> after the second char
    //   ...
    // The cursor is bottom-aligned to the baseline (y = 0), and horizontally centered to the right
    // of the glyph advance.
    const auto cscale = txt_prop.fTextSize * tinfo.fScale,
                cxpos = (fCursorIndex ? tinfo.fGlyphs[glyph_index].fAdvance : 0)
                         - fCursorBounds.width() * cscale * 0.5f,
                cypos = - fCursorBounds.height() * cscale;
    const auto cpath  = fCursorPath.makeTransform(SkMatrix::Translate(cxpos, cypos) *
                                                  SkMatrix::Scale(cscale, cscale));

    SkPaint p;
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeCap(SkPaint::kRound_Cap);

    SkAutoCanvasRestore acr(canvas, true);
    canvas->concat(tinfo.fGlyphs[glyph_index].fMatrix);

    p.setColor(SK_ColorWHITE);
    p.setStrokeWidth(3);
    canvas->drawPath(cpath, p);
    p.setColor(SK_ColorBLACK);
    p.setStrokeWidth(2);
    canvas->drawPath(cpath, p);
}

void TextEditor::updateDeps(const SkString& txt) {
    for (const auto& dep : fDependentProps) {
        auto txt_prop = dep->get();
        txt_prop.fText = txt;
        dep->set(txt_prop);
    }
}

void TextEditor::insertChar(SkUnichar c) {
    auto txt = fTextProp->get();
    const auto initial_size = txt.fText.size();

    txt.fText.insertUnichar(fCursorIndex, c);
    fCursorIndex += txt.fText.size() - initial_size;

    fTextProp->set(txt);
    this->updateDeps(txt.fText);
}

void TextEditor::deleteChars(size_t offset, size_t count) {
    auto txt = fTextProp->get();

    txt.fText.remove(offset, count);
    fTextProp->set(txt);
    this->updateDeps(txt.fText);

    fCursorIndex = offset;
}

bool TextEditor::deleteSelection() {
    const auto [glyph_sel_start, glyph_sel_end] = this->currentSelection();
    if (glyph_sel_start == glyph_sel_end) {
        return false;
    }

    const auto utf8_sel_start = fGlyphData[glyph_sel_start].fCluster,
               utf8_sel_end   = fGlyphData[glyph_sel_end  ].fCluster;
    SkASSERT(utf8_sel_start < utf8_sel_end);

    this->deleteChars(utf8_sel_start, utf8_sel_end - utf8_sel_start);

    fSelection = {0,0};

    return true;
}

void TextEditor::onDecorate(SkCanvas* canvas, const TextInfo& tinfo) {
    const auto [sel_start, sel_end] = this->currentSelection();

    fGlyphData.clear();

    for (size_t i = 0; i < tinfo.fGlyphs.size(); ++i) {
        const auto& ginfo = tinfo.fGlyphs[i];

        SkAutoCanvasRestore acr(canvas, true);
        canvas->concat(ginfo.fMatrix);

        // Stash some glyph info, for later use.
        fGlyphData.push_back({
            canvas->getLocalToDevice().asM33().mapRect(ginfo.fBounds),
            ginfo.fCluster
        });

        if (i < sel_start || i >= sel_end) {
            continue;
        }

        static constexpr SkColor4f kSelectionColor{0, 0, 1, 0.4f};
        canvas->drawRect(ginfo.fBounds, SkPaint(kSelectionColor));
    }

    // Only draw the cursor when there's no active selection.
    if (sel_start == sel_end) {
        this->drawCursor(canvas, tinfo);
    }
}

bool TextEditor::onMouseInput(SkScalar x, SkScalar y, skui::InputState state,
                                     skui::ModifierKey) {
    if (!fEnabled || fGlyphData.empty()) {
        return false;
    }

    switch (state) {
    case skui::InputState::kDown: {
        fMouseDown = true;

        const auto closest = this->closestGlyph({x, y});
        fSelection = {closest, closest};
    }   break;
    case skui::InputState::kUp:
        fMouseDown = false;
        break;
    case skui::InputState::kMove:
        if (fMouseDown) {
            const auto closest = this->closestGlyph({x, y});
            std::get<1>(fSelection) = closest < std::get<0>(fSelection)
                                            ? closest
                                            : closest + 1;
        }
        break;
    default:
        break;
    }

    return true;
}

bool TextEditor::onCharInput(SkUnichar c) {
    if (!fEnabled || fGlyphData.empty()) {
        return false;
    }

    const auto& txt_str = fTextProp->get().fText;

    // Natural editor bindings are currently intercepted by Viewer, so we use these instead.
    switch (c) {
    case '|':     // commit changes and exit editing mode
        this->toggleEnabled();
        break;
    case ']': {   // move right
        if (fCursorIndex < txt_str.size()) {
            fCursorIndex = next_utf8(txt_str, fCursorIndex);
        }
    } break;
    case '[':     // move left
        if (fCursorIndex > 0) {
            fCursorIndex = prev_utf8(txt_str, fCursorIndex);
        }
        break;
    case '\\': {  // delete
        if (!this->deleteSelection() && fCursorIndex > 0) {
            // Delete preceding char.
            const auto del_index = prev_utf8(txt_str, fCursorIndex),
                       del_count = fCursorIndex - del_index;

            this->deleteChars(del_index, del_count);
        }
    }   break;
    default:
        // Delete any selection on insert.
        this->deleteSelection();
        this->insertChar(c);
        break;
    }

    // Reset the cursor blink timer on input.
    fTimeBase = std::chrono::steady_clock::now();

    return true;
}

}  // namespace skottie_utils
