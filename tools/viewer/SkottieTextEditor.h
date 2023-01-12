/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieTextEditor_DEFINED
#define SkottieTextEditor_DEFINED

#include "modules/skottie/include/SkottieProperty.h"
#include "tools/skui/InputState.h"
#include "tools/skui/ModifierKey.h"

// A sample WYSIWYG text editor built using the GlyphDecorator API.
class SkottieTextEditor final : public skottie::GlyphDecorator {
public:
    explicit SkottieTextEditor(std::unique_ptr<skottie::TextPropertyHandle>&&);
    ~SkottieTextEditor() override;

    void toggleEnabled();

    void onDecorate(SkCanvas*, const GlyphInfo[], size_t) override;

    bool onMouseInput(SkScalar x, SkScalar y, skui::InputState state, skui::ModifierKey);

private:
    struct GlyphData {
        SkRect fDevBounds; // Glyph bounds mapped to device space.
    };

    size_t closestGlyph(const SkPoint& pt) const;

    const std::unique_ptr<skottie::TextPropertyHandle> fTextProp;

    std::vector<GlyphData>     fGlyphData;
    std::tuple<size_t, size_t> fSelection = {0, std::numeric_limits<size_t>::max()};
    bool                       fEnabled   = false;
    bool                       fMouseDown = false;
};

#endif // SkottieTextEditor_DEFINED
