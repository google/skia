// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "experimental/editor/editor.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkExecutor.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkPath.h"
#include "modules/skshaper/include/SkShaper.h"
#include "src/utils/SkUTF.h"

#include "experimental/editor/run_handler.h"

using namespace editor;

static constexpr SkRect kUnsetRect{-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX};

static SkRect selection_box(const SkFontMetrics& metrics,
                            float advance,
                            SkPoint pos) {
    if (fabsf(advance) < 1.0f) {
        advance = copysignf(1.0f, advance);
    }
    return SkRect{pos.x(),
                  pos.y() + metrics.fAscent,
                  pos.x() + advance,
                  pos.y() + metrics.fDescent}.makeSorted();
}

void callback_fn(void* context,
                 const char* utf8Text,
                 size_t utf8TextBytes,
                 size_t glyphCount,
                 const SkGlyphID* glyphs,
                 const SkPoint* positions,
                 const uint32_t* clusters,
                 const SkFont& font)
{
    SkASSERT(context);
    SkASSERT(glyphCount > 0);
    SkRect* cursors = (SkRect*)context;

    SkFontMetrics metrics;
    font.getMetrics(&metrics);
    std::unique_ptr<float[]> advances(new float[glyphCount]);
    font.getWidths(glyphs, glyphCount, advances.get());

    // Loop over each cluster in this run.
    size_t clusterStart = 0;
    for (size_t glyphIndex = 0; glyphIndex < glyphCount; ++glyphIndex) {
        if (glyphIndex + 1 < glyphCount  // more glyphs
            && clusters[glyphIndex] == clusters[glyphIndex + 1]) {
            continue; // multi-glyph cluster
        }
        unsigned textBegin = clusters[glyphIndex];
        unsigned textEnd = utf8TextBytes;
        for (size_t i = 0; i < glyphCount; ++i) {
            if (clusters[i] >= textEnd) {
                textEnd = clusters[i] + 1;
            }
        }
        for (size_t i = 0; i < glyphCount; ++i) {
            if (clusters[i] > textBegin && clusters[i] < textEnd) {
                textEnd = clusters[i];
                if (textEnd == textBegin + 1) { break; }
            }
        }
        SkASSERT(glyphIndex + 1 > clusterStart);
        unsigned clusterGlyphCount = glyphIndex + 1 - clusterStart;
        const SkPoint* clusterGlyphPositions = &positions[clusterStart];
        const float* clusterAdvances = &advances[clusterStart];
        clusterStart = glyphIndex + 1;  // for next loop

        SkRect clusterBox = selection_box(metrics, clusterAdvances[0], clusterGlyphPositions[0]);
        for (unsigned i = 1; i < clusterGlyphCount; ++i) { // multiple glyphs
            clusterBox.join(selection_box(metrics, clusterAdvances[i], clusterGlyphPositions[i]));
        }
        if (textBegin + 1 == textEnd) {  // single byte, fast path.
            cursors[textBegin] = clusterBox;
            continue;
        }
        int textCount = textEnd - textBegin;
        int codePointCount = SkUTF::CountUTF8(utf8Text + textBegin, textCount);
        if (codePointCount == 1) {  // single codepoint, fast path.
            cursors[textBegin] = clusterBox;
            continue;
        }

        float width = clusterBox.width() / codePointCount;
        SkASSERT(width > 0);
        const char* ptr = utf8Text + textBegin;
        const char* end = utf8Text + textEnd;
        float x = clusterBox.left();
        while (ptr < end) {  // for each codepoint in cluster
            const char* nextPtr = ptr;
            SkUTF::NextUTF8(&nextPtr, end);
            int firstIndex = ptr - utf8Text;
            float nextX = x + width;
            cursors[firstIndex] = SkRect{x, clusterBox.top(), nextX, clusterBox.bottom()};
            x = nextX;
            ptr = nextPtr;
        }
    }
}

void Editor::Shape(TextLine* line, SkShaper* shaper, float width, const SkFont& font,
                   SkRect space) {
    SkASSERT(line);
    SkASSERT(shaper);
    line->fCursorPos.resize(line->fText.size() + 1);
    for (SkRect& c : line->fCursorPos) {
        c = kUnsetRect;
    }
    RunHandler runHandler(line->fText.begin(), line->fText.size());
    runHandler.setRunCallback(callback_fn, line->fCursorPos.data());
    shaper->shape(line->fText.begin(), line->fText.size(), font, true, width, &runHandler);
    SkRect& last = line->fCursorPos[line->fText.size()];
    last = space;
    if (line->fText.size() > 0) {
        last.fLeft = line->fCursorPos[line->fText.size() - 1].fRight;
        last.fRight = last.fLeft + space.width();
    }
    float h = std::max(runHandler.endPoint().y(), font.getSpacing());
    line->fHeight = (int)ceilf(h);
    line->fBlob = runHandler.makeBlob();
}

// Kind of like Python's readlines(), but without any allocation.
// Calls f() on each line.
// F is [](const char*, size_t) -> void
template <typename F>
static void readlines(const void* data, size_t size, F f) {
    const char* start = (const char*)data;
    const char* end = start + size;
    const char* ptr = start;
    while (ptr < end) {
        while (*ptr++ != '\n' && ptr < end) {}
        size_t len = ptr - start;
        f(start, len);
        start = ptr;
    }
}

void Editor::setText(const char* data, size_t length) {
    std::vector<Editor::TextLine> lines;
    if (data && length) {
        readlines(data, length, [&lines](const char* p, size_t s) {
            if (s > 0 && p[s - 1] == '\n') { --s; }  // rstrip()
            lines.push_back(Editor::TextLine(p, s));
        });
    }
    fLines = std::move(lines);
    this->markAllDirty();
}

void Editor::setFont(SkFont font) {
    if (font != fFont) {
        fFont = font;
        auto shaper = SkShaper::Make();
        const char kSpace[] = " ";
        TextLine textLine(kSpace, strlen(kSpace));
        Editor::Shape(&textLine, shaper.get(), FLT_MAX, fFont, SkRect{0, 0, 0, 0});
        fSpaceBounds = textLine.fCursorPos[0];
        this->markAllDirty();
    }
}

Editor::TextPosition Editor::getPosition(SkIPoint xy) {
    this->reshapeAll();
    for (size_t j = 0; j < fLines.size(); ++j) {
        const TextLine& line = fLines[j];
        if (!line.fBlob) { continue; }
        SkIRect b = line.fBlob->bounds().roundOut().makeOffset(line.fOrigin.x(), line.fOrigin.y());
        if (b.contains(xy.x(), xy.y())) {
            xy -= line.fOrigin;
            const std::vector<SkRect>& pos = line.fCursorPos;
            for (size_t i = 0; i < pos.size(); ++i) {
                if (pos[i] == kUnsetRect) {
                    continue;
                }
                if (pos[i].contains((float)xy.x(), (float)xy.y())) {
                    return Editor::TextPosition{i, j};
                }
            }
        }
    }
    return Editor::TextPosition();
}

static inline bool is_utf8_continuation(char v) {
    return ((unsigned char)v & 0b11000000) ==
                               0b10000000;
}

static const char* next_utf8(const char* p, const char* end) {
    if (p < end) {
        do {
            ++p;
        } while (p < end && is_utf8_continuation(*p));
    }
    return p;
}

static const char* align_utf8(const char* p, const char* begin) {
    while (p > begin && is_utf8_continuation(*p)) {
        --p;
    }
    return p;
}

static const char* prev_utf8(const char* p, const char* begin) {
    return p > begin ? align_utf8(p - 1, begin) : begin;
}

Editor::TextPosition Editor::insert(TextPosition pos, const char* utf8Text, size_t byteLen) {
    //FIXME    if (!valid_utf8(utf8Text, byteLen)) { return; }
    pos = this->move(Editor::Movement::kNowhere, pos);
    if (pos.fParagraphIndex < fLines.size()) {
        fLines[pos.fParagraphIndex].fText.insert(pos.fTextByteIndex, utf8Text, byteLen);
        fLines[pos.fParagraphIndex].fBlob = nullptr;
        fNeedsReshape = true;
        return Editor::TextPosition{pos.fTextByteIndex + byteLen, pos.fParagraphIndex};
    } else {
        // append new line
    }
    return pos;
}

Editor::TextPosition Editor::remove(TextPosition pos1, TextPosition pos2) {
    pos1 = this->move(Editor::Movement::kNowhere, pos1);
    pos2 = this->move(Editor::Movement::kNowhere, pos2);
    auto cmp = [](const Editor::TextPosition& u, const Editor::TextPosition& v) { return u < v; };
    Editor::TextPosition start = std::min(pos1, pos2, cmp);
    Editor::TextPosition end = std::max(pos1, pos2, cmp);
    if (start == end || start.fParagraphIndex == fLines.size()) {
        return start;
    }
    if (start.fParagraphIndex == end.fParagraphIndex) {
        SkASSERT(end.fTextByteIndex > start.fTextByteIndex);
        fLines[start.fParagraphIndex].fText.remove(
                start.fTextByteIndex, end.fTextByteIndex - start.fTextByteIndex);
        fLines[start.fParagraphIndex].fBlob = nullptr;
        fNeedsReshape = true;
    } else {
        // delete across lines
    }
    return start;
}


static inline const char* begin(const StringSlice& s) { return s.begin(); }

static inline const char* end(const StringSlice& s) { return s.end(); }

static size_t align_column(const StringSlice& str, size_t p) {
    if (p >= str.size()) {
        return str.size();
    }
    return align_utf8(begin(str) + p, begin(str)) - begin(str);
}

Editor::TextPosition Editor::move(Editor::Movement move, Editor::TextPosition pos) {
    pos.fParagraphIndex = std::min(pos.fParagraphIndex, fLines.size());
    pos.fTextByteIndex = align_column(fLines[pos.fParagraphIndex].fText, pos.fTextByteIndex);
    switch (move) {
        case Editor::Movement::kNowhere:
            break;
        case Editor::Movement::kLeft:
            if (0 == pos.fTextByteIndex) {
                if (pos.fParagraphIndex > 0) {
                    --pos.fParagraphIndex;
                    pos.fTextByteIndex = fLines[pos.fParagraphIndex].fText.size();
                }
            } else {
                const auto& str = fLines[pos.fParagraphIndex].fText;
                pos.fTextByteIndex =
                    prev_utf8(begin(str) + pos.fTextByteIndex, begin(str)) - begin(str);
            }
            break;
        case Editor::Movement::kRight:
            if (fLines[pos.fParagraphIndex].fText.size() == pos.fTextByteIndex) {
                if (pos.fParagraphIndex + 1 < fLines.size()) {
                    ++pos.fParagraphIndex;
                    pos.fTextByteIndex = 0;
                }
            } else {
                const auto& str = fLines[pos.fParagraphIndex].fText;
                pos.fTextByteIndex =
                    next_utf8(begin(str) + pos.fTextByteIndex, end(str)) - begin(str);
            }
            break;
        case Editor::Movement::kHome:
            pos.fTextByteIndex = 0;
            break;
        case Editor::Movement::kEnd:
            pos.fTextByteIndex = fLines[pos.fParagraphIndex].fText.size();
            break;
        case Editor::Movement::kUp:
            if (pos.fParagraphIndex > 0) {
                --pos.fParagraphIndex;
                pos.fTextByteIndex =
                    align_column(fLines[pos.fParagraphIndex].fText, pos.fTextByteIndex);
            }
            break;
        case Editor::Movement::kDown:
            if (pos.fParagraphIndex + 1 < fLines.size()) {
                ++pos.fParagraphIndex;
                pos.fTextByteIndex =
                    align_column(fLines[pos.fParagraphIndex].fText, pos.fTextByteIndex);
            }
            break;
    }
    return pos;
}

static inline SkRect offset(SkRect r, SkIPoint p) {
    r.offset((float)p.x(), (float)p.y());
    return r;
}

void Editor::paint(SkCanvas* c, PaintOpts options) {
    this->reshapeAll();

    c->drawPaint(SkPaint(options.fBackgroundColor));

    SkPaint selection = SkPaint(options.fSelectionColor);
    auto cmp = [](const Editor::TextPosition& u, const Editor::TextPosition& v) { return u < v; };
    for (TextPosition pos = std::min(options.fSelectionBegin, options.fSelectionEnd, cmp),
                      end = std::max(options.fSelectionBegin, options.fSelectionEnd, cmp);
         pos < end;
         pos = this->move(Editor::Movement::kRight, pos))
    {
        SkASSERT(pos.fParagraphIndex < fLines.size());
        const TextLine& l = fLines[pos.fParagraphIndex];
        c->drawRect(offset(l.fCursorPos[pos.fTextByteIndex], l.fOrigin), selection);
    }

    if (fLines.size() > 0) {
        const TextLine& cLine = fLines[options.fCursor.fParagraphIndex];
        SkRect pos = fSpaceBounds;
        if (options.fCursor.fTextByteIndex < cLine.fCursorPos.size()) {
            pos = cLine.fCursorPos[options.fCursor.fTextByteIndex];
        }
        pos.fRight = pos.fLeft + 1;
        pos.fLeft -= 1;
        c->drawRect(offset(pos, cLine.fOrigin), SkPaint(options.fCursorColor));
    }

    SkPaint foreground = SkPaint(options.fForegroundColor);
    for (const TextLine& line : fLines) {
        if (line.fBlob) {
            c->drawTextBlob(line.fBlob.get(), line.fOrigin.x(), line.fOrigin.y(), foreground);
        }
    }
}

void Editor::markAllDirty() {
    for (TextLine& line : fLines) {
        line.fBlob = nullptr;
    }
    fNeedsReshape = true;
};

void Editor::reshapeAll() {
    if (fNeedsReshape) {
        float shape_width = (float)(fWidth - 2 * fMargin);
        #ifdef SK_EDITOR_GO_FAST
        SkSemaphore semaphore;
        std::unique_ptr<SkExecutor> executor = SkExecutor::MakeFIFOThreadPool(100);
        int jobCount = 0;
        for (TextLine& line : fLines) {
            if (!line.textBlob()) {
                executor->add([&]() {
                    Editor::Shape(&line, SkShaper::Make().get(), shape_width, fFont, fSpaceBounds);
                    semaphore.signal();
                }
                ++jobCount;
            });
        }
        while (jobCount-- > 0) { semaphore.wait(); }
        #else
        auto shaper = SkShaper::Make();
        for (TextLine& line : fLines) {
            if (!line.fBlob) {
                Editor::Shape(&line, shaper.get(), shape_width, fFont, fSpaceBounds);
            }
        }
        #endif
        int y = fMargin;
        for (TextLine& line : fLines) {
            line.fOrigin = {fMargin, y};
            y += line.fHeight;
        }
        fHeight = y + fMargin;
        fNeedsReshape = false;
    }
}

void Editor::setWidth(int w) {
    if (fWidth != w) {
        fWidth = w;
        this->markAllDirty();
    }
}
