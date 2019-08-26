// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "modules/skplaintexteditor/include/editor.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

static constexpr SkISize kSize = {512, 256};

void draw_editor(const char* resource, SkCanvas* canvas) {
    auto data = GetResourceAsData(resource);
    if (data) {
        SkPlainTextEditor::Editor editor;
        editor.setWidth(kSize.width() - 8);
        editor.setFont(SkFont(ToolUtils::create_portable_typeface(), 14));
        editor.insert({0, 0}, (const char*)data->data(), data->size());
        SkPlainTextEditor::Editor::PaintOpts opts;
        opts.fSelectionBegin = {0, 0};
        opts.fSelectionEnd = {0, 1};
        opts.fCursor = {0, 1};
        SkAutoCanvasRestore autoCanvasRestore(canvas, true);
        canvas->translate(4, 4);
        editor.paint(canvas, opts);
    }
}

#define M(X)                                                                            \
    DEF_SIMPLE_GM(plaintexteditor_ ## X, canvas, kSize.width(), kSize.height()) {       \
        draw_editor("text/" #X ".txt", canvas);                                         \
    }
M(arabic)
M(armenian)
M(balinese)
M(bengali)
M(buginese)
M(cherokee)
M(cyrillic)
M(devanagari)
M(emoji)
M(english)
M(ethiopic)
M(greek)
M(han_simplified)
M(han_traditional)
M(hangul)
M(hebrew)
M(javanese)
M(kana)
M(khmer)
M(lao)
M(mandaic)
M(myanmar)
M(newtailue)
M(nko)
M(sinhala)
M(sundanese)
M(syriac)
M(taitham)
M(tamil)
M(thaana)
M(thai)
M(tibetan)
M(tifnagh)
M(vai)
#undef M
