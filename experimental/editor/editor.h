// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef editor_DEFINED
#define editor_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkTextBlob.h"

#include <string>
#include <vector>

class SkCanvas;

namespace editor {

struct TextLine {
    TextLine(std::string s) : fText(std::move(s)) {}
    std::string fText;
    float fHeight = 0;
    sk_sp<const SkTextBlob> fBlob;
    bool fSelected = false;  // Will allow selection of subset of text later.
    // Also will track presence of cursor.
};

class Editor {
public:
    void setText(std::vector<TextLine> t) { fLines = std::move(t); }
    int getHeight() const { return fHeight; }
    int getMargin() const { return fMargin; }
    void paint(SkCanvas* canvas);
    void setWidth(int w); // may force re-shape
    const SkFont& font() const { return fFont; }

    // experimental
    void select(unsigned lineIndex) { fLines[lineIndex].fSelected = !fLines[lineIndex].fSelected; }
    const std::vector<TextLine>& lines() const { return fLines; }

private:
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
