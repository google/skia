/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieTextEditor_DEFINED
#define SkottieTextEditor_DEFINED

#include "include/core/SkPath.h"
#include "modules/skottie/include/SkottieProperty.h"
#include "tools/skui/InputState.h"
#include "tools/skui/ModifierKey.h"

#include <chrono>

namespace skottie_utils {

// A sample WYSIWYG text editor built using the GlyphDecorator API.
class TextEditor final : public skottie::GlyphDecorator {
public:
    TextEditor(std::unique_ptr<skottie::TextPropertyHandle>&&,
               std::vector<std::unique_ptr<skottie::TextPropertyHandle>>&&);
    ~TextEditor() override;

    void toggleEnabled();

    void onDecorate(SkCanvas*, const TextInfo&) override;

    bool onMouseInput(SkScalar x, SkScalar y, skui::InputState state, skui::ModifierKey);

    bool onCharInput(SkUnichar c);

private:
    struct GlyphData {
        SkRect fDevBounds; // Glyph bounds mapped to device space.
        size_t fCluster;   // UTF8 cluster index.
    };

    std::tuple<size_t, size_t> currentSelection() const;
    size_t closestGlyph(const SkPoint& pt) const;
    void drawCursor(SkCanvas*, const TextInfo&) const;
    void insertChar(SkUnichar c);
    void deleteChars(size_t offset, size_t count);
    bool deleteSelection();
    void updateDeps(const SkString&);

    const std::unique_ptr<skottie::TextPropertyHandle>              fTextProp;
    const std::vector<std::unique_ptr<skottie::TextPropertyHandle>> fDependentProps;
    const SkPath                                                    fCursorPath;
    const SkRect                                                    fCursorBounds;

    std::vector<GlyphData>     fGlyphData;
    std::tuple<size_t, size_t> fSelection   = {0,0};  // Indices in the glyphs domain.
    size_t                     fCursorIndex = 0;      // Index in the UTF8 domain.
    bool                       fEnabled     = false;
    bool                       fMouseDown   = false;

    std::chrono::time_point<std::chrono::steady_clock> fTimeBase;
};

}  // namespace skottie_utils

#endif // SkottieTextEditor_DEFINED
