// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef editor_DEFINED
#define editor_DEFINED

#include "experimental/editor/stringslice.h"
#include "experimental/editor/stringview.h"

#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"

#include <climits>
#include <cstdint>
#include <utility>
#include <vector>

class SkCanvas;
class SkShaper;

// TODO: modulize this; editor::Editor becomes SkEditor ?

namespace editor {

class Editor {
    struct TextLine;
public:
    // total height in canvas display units.
    int getHeight() const { return fHeight; }

    // set display width in canvas display units
    void setWidth(int w); // may force re-shape

    // get/set current font (used for shaping and displaying text)
    const SkFont& font() const { return fFont; }
    void setFont(SkFont font);

    struct Text {
        const std::vector<TextLine>& fLines;
        struct Iterator {
            std::vector<TextLine>::const_iterator fPtr;
            StringView operator*() { return fPtr->fText.view(); }
            void operator++() { ++fPtr; }
            bool operator!=(const Iterator& other) const { return fPtr != other.fPtr; }
        };
        Iterator begin() const { return Iterator{fLines.begin()}; }
        Iterator end() const { return Iterator{fLines.end()}; }
    };
    // Loop over all the lines of text.  The lines are not '\0'- or '\n'-terminated.
    // For example, to dump the entire file to standard output:
    //     for (editor::StringView str : editor.text()) {
    //         std::cout.write(str.data, str.size) << '\n';
    //     }
    Text text() const { return Text{fLines}; }

    // get size of line in canvas display units.
    int lineHeight(size_t index) const { return fLines[index].fHeight; }

    struct TextPosition {
        size_t fTextByteIndex = SIZE_MAX;   // index into UTF-8 representation of line.
        size_t fParagraphIndex = SIZE_MAX;  // logical line, based on hard newline characters.
    };
    enum class Movement {
        kNowhere,
        kLeft,
        kUp,
        kRight,
        kDown,
        kHome,
        kEnd,
        kWordLeft,
        kWordRight,
    };
    TextPosition move(Editor::Movement move, Editor::TextPosition pos) const;
    TextPosition getPosition(SkIPoint);
    SkRect getLocation(TextPosition);
    // insert into current text.
    TextPosition insert(TextPosition, const char* utf8Text, size_t byteLen);
    // remove text between two positions
    TextPosition remove(TextPosition, TextPosition);

    // If dst is nullptr, returns size of given selection.
    // Otherwise, fill dst with a copy of the selection, and return the amount copied.
    size_t copy(TextPosition pos1, TextPosition pos2, char* dst = nullptr) const;
    size_t lineCount() const { return fLines.size(); }
    StringView line(size_t i) const {
        return i < fLines.size() ? fLines[i].fText.view() : StringView{nullptr, 0};
    }

    struct PaintOpts {
        SkColor4f fBackgroundColor = {1, 1, 1, 1};
        SkColor4f fForegroundColor = {0, 0, 0, 1};
        // TODO: maybe have multiple selections and cursors, each with separate colors.
        SkColor4f fSelectionColor = {0.729f, 0.827f, 0.988f, 1};
        SkColor4f fCursorColor = {1, 0, 0, 1};
        TextPosition fSelectionBegin;
        TextPosition fSelectionEnd;
        TextPosition fCursor;
    };
    void paint(SkCanvas* canvas, PaintOpts);

private:
    // TODO: rename this to TextParagraph. fLines to fParas.
    struct TextLine {
        StringSlice fText;
        sk_sp<const SkTextBlob> fBlob;
        std::vector<SkRect> fCursorPos;
        std::vector<unsigned> fLineEndOffsets;
        std::vector<bool> fWordBoundaries;
        SkIPoint fOrigin = {0, 0};
        int fHeight = 0;
        bool fShaped = false;

        TextLine(StringSlice t) : fText(std::move(t)) {}
        TextLine() {}
    };
    std::vector<TextLine> fLines;
    int fWidth = 0;
    int fHeight = 0;
    SkFont fFont;
    SkRect fSpaceBounds = {0, 0, 0, 0};
    bool fNeedsReshape = false;
    const char* fLocale = "en";  // TODO: make this setable

    static void Shape(TextLine*, SkShaper*, float width, const SkFont&, SkRect, const char*);
    void markDirty(TextLine*);
    void markAllDirty() { for (auto& l : fLines) { this->markDirty(&l); } }
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
