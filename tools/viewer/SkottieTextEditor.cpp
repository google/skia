/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/SkottieTextEditor.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkM44.h"

SkottieTextEditor::SkottieTextEditor(std::unique_ptr<skottie::TextPropertyHandle>&& prop)
    : fTextProp(std::move(prop))
{}

SkottieTextEditor::~SkottieTextEditor() = default;

void SkottieTextEditor::toggleEnabled() {
    fEnabled = !fEnabled;

    auto txt = fTextProp->get();
    txt.fDecorator = fEnabled ? sk_ref_sp(this) : nullptr;
    fTextProp->set(txt);
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

void SkottieTextEditor::onDecorate(SkCanvas* canvas, const GlyphInfo glyphs[], size_t size) {
    // Selection can be inverted.
    const auto sel_start = std::min(std::get<0>(fSelection), std::get<1>(fSelection)),
               sel_end   = std::max(std::get<0>(fSelection), std::get<1>(fSelection));

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
    } break;
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
