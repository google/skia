/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/SkottieTextEditor.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkM44.h"
#include "include/core/SkPath.h"

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

} // namespace

SkottieTextEditor::SkottieTextEditor(std::unique_ptr<skottie::TextPropertyHandle>&& prop)
    : fTextProp(std::move(prop))
    , fCursorPath(make_cursor_path())
    , fCursorBounds(fCursorPath.computeTightBounds())
    , fCursorIndex(fTextProp->get().fText.size())
{}

SkottieTextEditor::~SkottieTextEditor() = default;

void SkottieTextEditor::toggleEnabled() {
    fEnabled = !fEnabled;

    auto txt = fTextProp->get();
    txt.fDecorator = fEnabled ? sk_ref_sp(this) : nullptr;
    fTextProp->set(txt);

    fTimeBase = std::chrono::steady_clock::now();
}

std::tuple<size_t, size_t> SkottieTextEditor::currentSelection() const {
    // Selection can be inverted.
    return std::make_tuple(std::min(std::get<0>(fSelection), std::get<1>(fSelection)),
                           std::max(std::get<0>(fSelection), std::get<1>(fSelection)));
}

size_t SkottieTextEditor::closestGlyph(const SkPoint& pt) const {
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

void SkottieTextEditor::drawCursor(SkCanvas* canvas, const GlyphInfo glyphs[], size_t size) const {
    constexpr double kCursorHz = 2;
    const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::steady_clock::now() - fTimeBase).count();
    const long cycle = static_cast<long>(static_cast<double>(now_ms) * 0.001 * kCursorHz);
    if (cycle & 1) {
        // blink
        return;
    }

    auto txt_prop  = fTextProp->get();
    const auto txt = txt_prop.fText;

    const auto glyph_index = [&]() -> size_t {
        if (!fCursorIndex) {
            return 0;
        }

        for (size_t i = 0; i < size; ++i) {
            if (glyphs[i].fCluster >= fCursorIndex - 1) {
                return i;
            }
        }

        return size - 1;
    }();

    const auto& glyph_bounds = glyphs[glyph_index].fBounds;

    // Cursor index mapping:
    //   0 -> before the first char
    //   1 -> after the first char
    //   2 -> after the second char
    //   ...
    // The cursor is bottom-aligned, and centered to the right/left edge of the glyph bounding box.
    const auto cscale = txt_prop.fTextSize,
                cxpos = (fCursorIndex ? glyph_bounds.fRight : glyph_bounds.fLeft)
                         - fCursorBounds.width() * cscale * 0.5f,
                cypos = glyph_bounds.fBottom - fCursorBounds.height() * cscale;
    const auto cpath  = fCursorPath.makeTransform(SkMatrix::Translate(cxpos, cypos) *
                                                  SkMatrix::Scale(cscale, cscale));

    SkPaint p;
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(2);
    p.setStrokeCap(SkPaint::kRound_Cap);

    SkAutoCanvasRestore acr(canvas, true);
    canvas->concat(glyphs[glyph_index].fMatrix);
    canvas->drawPath(cpath, p);
}

void SkottieTextEditor::insertChar(SkUnichar c) {
    auto txt = fTextProp->get();

    txt.fText.insertUnichar(fCursorIndex++, c);
    fTextProp->set(txt);
}

void SkottieTextEditor::deleteChars(size_t offset, size_t count) {
    auto txt = fTextProp->get();

    txt.fText.remove(offset, count);
    fTextProp->set(txt);

    if (fCursorIndex >= offset) {
        fCursorIndex -= count;
    }
}

void SkottieTextEditor::onDecorate(SkCanvas* canvas, const GlyphInfo glyphs[], size_t size) {
    const auto [sel_start, sel_end] = this->currentSelection();

    fGlyphData.clear();

    for (size_t i = 0; i < size; ++i) {
        const auto& ginfo = glyphs[i];

        SkAutoCanvasRestore acr(canvas, true);
        canvas->concat(ginfo.fMatrix);

        // Stash some glyph info, for later use.
        fGlyphData.push_back({canvas->getLocalToDevice().asM33().mapRect(ginfo.fBounds)});

        if (i < sel_start || i >= sel_end) {
            continue;
        }

        static constexpr SkColor4f kSelectionColor{0, 0, 1, 0.4f};
        canvas->drawRect(ginfo.fBounds, SkPaint(kSelectionColor));
    }

    // Only draw the cursor when there's no active selection.
    if (sel_start == sel_end) {
        this->drawCursor(canvas, glyphs, size);
    }
}

bool SkottieTextEditor::onMouseInput(SkScalar x, SkScalar y, skui::InputState state,
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

bool SkottieTextEditor::onCharInput(SkUnichar c) {
    if (!fEnabled || fGlyphData.empty()) {
        return false;
    }

    // Natural editor bindings are currently intercepted by Viewer, so we use these instead.
    switch (c) {
    case '|':     // commit changes and exit editing mode
        this->toggleEnabled();
        break;
    case ']': {   // move right
        if (fCursorIndex < fTextProp->get().fText.size()) {
            fCursorIndex++;
        }
    } break;
    case '[':     // move left
        if (fCursorIndex > 0) {
            fCursorIndex--;
        }
        break;
    case '\\': {  // delete
        const auto [sel_start, sel_end] = this->currentSelection();
        if (sel_start != sel_end) {
            this->deleteChars(sel_start, sel_end - sel_start);
            fSelection = {0,0};
        } else {
            if (fCursorIndex) {
                this->deleteChars(fCursorIndex - 1, 1);
            }
        }
    }   break;
    default:
        this->insertChar(c);
        break;
    }

    // Reset the cursor blink timer on input.
    fTimeBase = std::chrono::steady_clock::now();

    return true;
}
