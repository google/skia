// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef editor_DEFINED
#define editor_DEFINED

#include "experimental/editor/stringslice.h"

#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"

#include <climits>
#include <cstdint>
#include <vector>

class SkCanvas;
class SkShaper;

// TODO: modulize this; editor::Editor becomes SkEditor ?

namespace editor {

class Editor {
public:
    void setText(const char* text, size_t len);
    int getHeight() const { return fHeight; }
    int getMargin() const { return fMargin; }

    void setWidth(int w); // may force re-shape
    const SkFont& font() const { return fFont; }
    void setFont(SkFont font);

    // query buffer:
    struct Str {
        const char* fPtr = nullptr;
        size_t fLen = 0;
    };
    size_t lineCount() const { return fLines.size(); }
    Str line(size_t index) const { return fLines[index].text(); }
    int lineHeight(size_t index) const { return fLines[index].fHeight; }

    struct TextPosition {
        size_t fTextByteIndex = SIZE_MAX;  // index into UTF-8 representation of line.
        size_t fParagraphIndex = SIZE_MAX;    // logical line, based on hard newline characters.
    };
    enum class Movement {
        kNowhere,
        kLeft,
        kUp,
        kRight,
        kDown,
        kHome,
        kEnd,
    };
    TextPosition move(Editor::Movement move, Editor::TextPosition pos);
    TextPosition getPosition(SkIPoint);
    TextPosition insert(TextPosition, const char* utf8Text, size_t byteLen);
    TextPosition remove(TextPosition, TextPosition);
    StringSlice copy(TextPosition, TextPosition);

    struct PaintOpts {
        SkColor4f fBackgroundColor = {1, 1, 1, 1};
        SkColor4f fForegroundColor = {0, 0, 0, 1};
        SkColor4f fSelectionColor = {0.729f, 0.827f, 0.988f, 1};
        SkColor4f fCursorColor = {1, 0, 0, 1};
        TextPosition fSelectionBegin;
        TextPosition fSelectionEnd;
        TextPosition fCursor;
    };
    void paint(SkCanvas* canvas, PaintOpts);

private:
    struct TextLine {
        StringSlice fText;
        std::vector<SkRect> fCursorPos;
        SkIPoint fOrigin = {0, 0};
        int fHeight = 0;
        sk_sp<const SkTextBlob> fBlob;
        bool fSelected = false;  // Will allow selection of subset of text later.
        // Also will track presence of cursor.

        TextLine(const char* str, size_t len) : fText(str, len) {}
        Str text() const { return Str{fText.begin(), fText.size()}; }
    };
    std::vector<TextLine> fLines;
    int fMargin = 10;
    int fWidth = 0;
    int fHeight = 0;
    SkFont fFont;
    SkRect fSpaceBounds = {0, 0, 0, 0};
    bool fNeedsReshape = false;

    static void Shape(TextLine*, SkShaper*, float width, const SkFont&, SkRect);
    void markAllDirty();
    void reshapeAll();
};
}  // namespace editor

static inline bool operator==(const editor::Editor::TextPosition& u,
                              const editor::Editor::TextPosition& v) {
    return u.fParagraphIndex == v.fParagraphIndex && u.fTextByteIndex == v.fTextByteIndex;
}
static inline bool operator!=(const editor::Editor::TextPosition& u,
                              const editor::Editor::TextPosition& v) { return !(u == v); }

static inline bool operator<(const editor::Editor::TextPosition& u,
                             const editor::Editor::TextPosition& v) {
    return u.fParagraphIndex < v.fParagraphIndex ||
           (u.fParagraphIndex == v.fParagraphIndex && u.fTextByteIndex < v.fTextByteIndex);
}


#endif  // editor_DEFINED
