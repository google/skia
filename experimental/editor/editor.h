// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef editor_DEFINED
#define editor_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"

#include <vector>

class SkCanvas;

// TODO: modulize this; editor::Editor becomes SkEditor ?

namespace editor {

class Editor {
public:
    void setText(const char* text, size_t len);
    int getHeight() const { return fHeight; }
    int getMargin() const { return fMargin; }
    void paint(SkCanvas* canvas);
    void setWidth(int w); // may force re-shape
    const SkFont& font() const { return fFont; }

    // query buffer:
    struct Str {
        const char* fPtr = nullptr;
        size_t fLen = 0;
    };
    size_t lineCount() const { return fLines.size(); }
    Str line(size_t index) const { return fLines[index].text(); }
    int lineHeight(size_t index) const { return fLines[index].fHeight; }

    // experimental interface
    void select(unsigned lineIndex) { fLines[lineIndex].fSelected = !fLines[lineIndex].fSelected; }

private:
    struct TextLine {
        SkString fText;
        int fHeight = 0;
        sk_sp<const SkTextBlob> fBlob;
        bool fSelected = false;  // Will allow selection of subset of text later.
        // Also will track presence of cursor.

        TextLine(SkString s) : fText(std::move(s)) {}
        Str text() const { return Str{fText.c_str(), fText.size()}; }
    };
    std::vector<TextLine> fLines;
    int fMargin = 10;
    int fWidth = 0;
    int fHeight = 0;
    SkFont fFont{nullptr, 24};
    SkColor4f fBackgroundColor = {0.8f, 0.8f, 0.8f, 1};
    SkColor4f fForegroundColor = {0, 0, 0, 1};
};
}  // namespace editor
#endif  // editor_DEFINED
