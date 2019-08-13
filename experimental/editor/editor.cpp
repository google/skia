// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "experimental/editor/editor.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkExecutor.h"
#include "include/core/SkPath.h"
#include "src/utils/SkUTF.h"

#include "experimental/editor/shape.h"

#include <algorithm>

using namespace editor;

static inline SkRect offset(SkRect r, SkIPoint p) {
    return r.makeOffset((float)p.x(), (float)p.y());
}

static constexpr SkRect kUnsetRect{-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX};

static bool valid_utf8(const char* ptr, size_t size) { return SkUTF::CountUTF8(ptr, size) >= 0; }

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
        SkASSERT(len > 0);
        f(start, len);
        start = ptr;
    }
}

static const StringSlice remove_newline(const char* str, size_t len) {
    return SkASSERT((str != nullptr) || (len == 0)),
           StringSlice(str, (len > 0 && str[len - 1] == '\n') ? len - 1 : len);
}

void Editor::markDirty(TextLine* line) {
    line->fBlob = nullptr;
    line->fShaped = false;
    line->fWordBoundaries = std::vector<bool>();
}

void Editor::setFont(SkFont font) {
    if (font != fFont) {
        fFont = std::move(font);
        fNeedsReshape = true;
        for (auto& l : fLines) { this->markDirty(&l); }
    }
}

void Editor::setWidth(int w) {
    if (fWidth != w) {
        fWidth = w;
        fNeedsReshape = true;
        for (auto& l : fLines) { this->markDirty(&l); }
    }
}
static SkPoint to_point(SkIPoint p) { return {(float)p.x(), (float)p.y()}; }

Editor::TextPosition Editor::getPosition(SkIPoint xy) {
    Editor::TextPosition approximatePosition;
    this->reshapeAll();
    for (size_t j = 0; j < fLines.size(); ++j) {
        const TextLine& line = fLines[j];
        SkIRect lineRect = {0,
                            line.fOrigin.y(),
                            fWidth,
                            j + 1 < fLines.size() ? fLines[j + 1].fOrigin.y() : INT_MAX};
        if (const SkTextBlob* b = line.fBlob.get()) {
            SkIRect r = b->bounds().roundOut();
            r.offset(line.fOrigin);
            lineRect.join(r);
        }
        if (!lineRect.contains(xy.x(), xy.y())) {
            continue;
        }
        SkPoint pt = to_point(xy - line.fOrigin);
        const std::vector<SkRect>& pos = line.fCursorPos;
        for (size_t i = 0; i < pos.size(); ++i) {
            if (pos[i] != kUnsetRect && pos[i].contains(pt.x(), pt.y())) {
                return Editor::TextPosition{i, j};
            }
        }
        approximatePosition = {xy.x() <= line.fOrigin.x() ? 0 : line.fText.size(), j};
    }
    return approximatePosition;
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

SkRect Editor::getLocation(Editor::TextPosition cursor) {
    this->reshapeAll();
    cursor = this->move(Editor::Movement::kNowhere, cursor);
    if (fLines.size() > 0) {
        const TextLine& cLine = fLines[cursor.fParagraphIndex];
        SkRect pos = {0, 0, 0, 0};
        if (cursor.fTextByteIndex < cLine.fCursorPos.size()) {
            pos = cLine.fCursorPos[cursor.fTextByteIndex];
        }
        pos.fRight = pos.fLeft + 1;
        pos.fLeft -= 1;
        return offset(pos, cLine.fOrigin);
    }
    return SkRect{0, 0, 0, 0};
}

static size_t count_char(const StringSlice& string, char value) {
    size_t count = 0;
    for (char c : string) { if (c == value) { ++count; } }
    return count;
}

Editor::TextPosition Editor::insert(TextPosition pos, const char* utf8Text, size_t byteLen) {
    if (!valid_utf8(utf8Text, byteLen) || 0 == byteLen) {
        return pos;
    }
    pos = this->move(Editor::Movement::kNowhere, pos);
    fNeedsReshape = true;
    if (pos.fParagraphIndex < fLines.size()) {
        fLines[pos.fParagraphIndex].fText.insert(pos.fTextByteIndex, utf8Text, byteLen);
        this->markDirty(&fLines[pos.fParagraphIndex]);
    } else {
        SkASSERT(pos.fParagraphIndex == fLines.size());
        SkASSERT(pos.fTextByteIndex == 0);
        fLines.push_back(Editor::TextLine(StringSlice(utf8Text, byteLen)));
    }
    pos = Editor::TextPosition{pos.fTextByteIndex + byteLen, pos.fParagraphIndex};
    size_t newlinecount = count_char(fLines[pos.fParagraphIndex].fText, '\n');
    if (newlinecount > 0) {
        StringSlice src = std::move(fLines[pos.fParagraphIndex].fText);
        std::vector<TextLine>::const_iterator next = fLines.begin() + pos.fParagraphIndex + 1;
        fLines.insert(next, newlinecount, TextLine());
        TextLine* line = &fLines[pos.fParagraphIndex];
        readlines(src.begin(), src.size(), [&line](const char* str, size_t l) {
            (line++)->fText = remove_newline(str, l);
        });
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
    fNeedsReshape = true;
    if (start.fParagraphIndex == end.fParagraphIndex) {
        SkASSERT(end.fTextByteIndex > start.fTextByteIndex);
        fLines[start.fParagraphIndex].fText.remove(
                start.fTextByteIndex, end.fTextByteIndex - start.fTextByteIndex);
        this->markDirty(&fLines[start.fParagraphIndex]);
    } else {
        SkASSERT(end.fParagraphIndex < fLines.size());
        auto& line = fLines[start.fParagraphIndex];
        line.fText.remove(start.fTextByteIndex,
                          line.fText.size() - start.fTextByteIndex);
        line.fText.insert(start.fTextByteIndex,
                          fLines[end.fParagraphIndex].fText.begin() + end.fTextByteIndex,
                          fLines[end.fParagraphIndex].fText.size() - end.fTextByteIndex);
        this->markDirty(&line);
        fLines.erase(fLines.begin() + start.fParagraphIndex + 1,
                     fLines.begin() + end.fParagraphIndex + 1);
    }
    return start;
}

static void append(char** dst, size_t* count, const char* src, size_t n) {
    if (*dst) {
        ::memcpy(*dst, src, n);
        *dst += n;
    }
    *count += n;
}

size_t Editor::copy(TextPosition pos1, TextPosition pos2, char* dst) const {
    size_t size = 0;
    pos1 = this->move(Editor::Movement::kNowhere, pos1);
    pos2 = this->move(Editor::Movement::kNowhere, pos2);
    auto cmp = [](const Editor::TextPosition& u, const Editor::TextPosition& v) { return u < v; };
    Editor::TextPosition start = std::min(pos1, pos2, cmp);
    Editor::TextPosition end = std::max(pos1, pos2, cmp);
    if (start == end || start.fParagraphIndex == fLines.size()) {
        return size;
    }
    if (start.fParagraphIndex == end.fParagraphIndex) {
        SkASSERT(end.fTextByteIndex > start.fTextByteIndex);
        auto& str = fLines[start.fParagraphIndex].fText;
        append(&dst, &size, str.begin() + start.fTextByteIndex,
               end.fTextByteIndex - start.fTextByteIndex);
        return size;
    }
    SkASSERT(end.fParagraphIndex < fLines.size());
    const std::vector<TextLine>::const_iterator firstP = fLines.begin() + start.fParagraphIndex;
    const std::vector<TextLine>::const_iterator lastP  = fLines.begin() + end.fParagraphIndex;
    const auto& first = firstP->fText;
    const auto& last  = lastP->fText;

    append(&dst, &size, first.begin() + start.fTextByteIndex, first.size() - start.fTextByteIndex);
    for (auto line = firstP + 1; line < lastP; ++line) {
        append(&dst, &size, "\n", 1);
        append(&dst, &size, line->fText.begin(), line->fText.size());
    }
    append(&dst, &size, "\n", 1);
    append(&dst, &size, last.begin(), end.fTextByteIndex);
    return size;
}

static inline const char* begin(const StringSlice& s) { return s.begin(); }

static inline const char* end(const StringSlice& s) { return s.end(); }

static size_t align_column(const StringSlice& str, size_t p) {
    if (p >= str.size()) {
        return str.size();
    }
    return align_utf8(begin(str) + p, begin(str)) - begin(str);
}

// returns smallest i such that list[i] > value.  value > list[i-1]
// Use a binary search since list is monotonic
template <typename T>
static size_t find_first_larger(const std::vector<T>& list, T value) {
    return (size_t)(std::upper_bound(list.begin(), list.end(), value) - list.begin());
}

static size_t find_closest_x(const std::vector<SkRect>& bounds, float x, size_t b, size_t e) {
    if (b >= e) {
        return b;
    }
    SkASSERT(e <= bounds.size());
    size_t best_index = b;
    float best_diff = ::fabsf(bounds[best_index].x() - x);
    for (size_t i = b + 1; i < e; ++i) {
        float d = ::fabsf(bounds[i].x() - x);
        if (d < best_diff) {
            best_diff = d;
            best_index = i;
        }
    }
    return best_index;
}

Editor::TextPosition Editor::move(Editor::Movement move, Editor::TextPosition pos) const {
    if (fLines.empty()) {
        return {0, 0};
    }
    // First thing: fix possible bad input values.
    if (pos.fParagraphIndex >= fLines.size()) {
        pos.fParagraphIndex = fLines.size() - 1;
        pos.fTextByteIndex = fLines[pos.fParagraphIndex].fText.size();
    } else {
        pos.fTextByteIndex = align_column(fLines[pos.fParagraphIndex].fText, pos.fTextByteIndex);
    }

    SkASSERT(pos.fParagraphIndex < fLines.size());
    SkASSERT(pos.fTextByteIndex <= fLines[pos.fParagraphIndex].fText.size());

    SkASSERT(pos.fTextByteIndex == fLines[pos.fParagraphIndex].fText.size() ||
             !is_utf8_continuation(fLines[pos.fParagraphIndex].fText.begin()[pos.fTextByteIndex]));

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
            {
                const std::vector<size_t>& list = fLines[pos.fParagraphIndex].fLineEndOffsets;
                size_t f = find_first_larger(list, pos.fTextByteIndex);
                pos.fTextByteIndex = f > 0 ? list[f - 1] : 0;
            }
            break;
        case Editor::Movement::kEnd:
            {
                const std::vector<size_t>& list = fLines[pos.fParagraphIndex].fLineEndOffsets;
                size_t f = find_first_larger(list, pos.fTextByteIndex);
                if (f < list.size()) {
                    pos.fTextByteIndex = list[f] > 0 ? list[f] - 1 : 0;
                } else {
                    pos.fTextByteIndex = fLines[pos.fParagraphIndex].fText.size();
                }
            }
            break;
        case Editor::Movement::kUp:
            {
                SkASSERT(pos.fTextByteIndex < fLines[pos.fParagraphIndex].fCursorPos.size());
                float x = fLines[pos.fParagraphIndex].fCursorPos[pos.fTextByteIndex].left();
                const std::vector<size_t>& list = fLines[pos.fParagraphIndex].fLineEndOffsets;
                size_t f = find_first_larger(list, pos.fTextByteIndex);
                // list[f] > value.  value > list[f-1]
                if (f > 0) {
                    // not the first line in paragraph.
                    pos.fTextByteIndex = find_closest_x(fLines[pos.fParagraphIndex].fCursorPos, x,
                                                        (f == 1) ? 0 : list[f - 2],
                                                        list[f - 1]);
                } else if (pos.fParagraphIndex > 0) {
                    --pos.fParagraphIndex;
                    const auto& newLine = fLines[pos.fParagraphIndex];
                    size_t r = newLine.fLineEndOffsets.size();
                    if (r > 0) {
                        pos.fTextByteIndex = find_closest_x(newLine.fCursorPos, x,
                                                            newLine.fLineEndOffsets[r - 1],
                                                            newLine.fCursorPos.size());
                    } else {
                        pos.fTextByteIndex = find_closest_x(newLine.fCursorPos, x, 0,
                                                            newLine.fCursorPos.size());
                    }
                }
                pos.fTextByteIndex =
                    align_column(fLines[pos.fParagraphIndex].fText, pos.fTextByteIndex);
            }
            break;
        case Editor::Movement::kDown:
            {
                const std::vector<size_t>& list = fLines[pos.fParagraphIndex].fLineEndOffsets;
                float x = fLines[pos.fParagraphIndex].fCursorPos[pos.fTextByteIndex].left();

                size_t f = find_first_larger(list, pos.fTextByteIndex);
                if (f < list.size()) {
                    const auto& bounds = fLines[pos.fParagraphIndex].fCursorPos;
                    pos.fTextByteIndex = find_closest_x(bounds, x, list[f],
                                                        f + 1 < list.size() ? list[f + 1]
                                                                            : bounds.size());
                } else if (pos.fParagraphIndex + 1 < fLines.size()) {
                    ++pos.fParagraphIndex;
                    const auto& bounds = fLines[pos.fParagraphIndex].fCursorPos;
                    const std::vector<size_t>& l2 = fLines[pos.fParagraphIndex].fLineEndOffsets;
                    pos.fTextByteIndex = find_closest_x(bounds, x, 0,
                                                        l2.size() > 0 ? l2[0] : bounds.size());
                } else {
                    pos.fTextByteIndex = fLines[pos.fParagraphIndex].fText.size();
                }
                pos.fTextByteIndex =
                    align_column(fLines[pos.fParagraphIndex].fText, pos.fTextByteIndex);
            }
            break;
        case Editor::Movement::kWordLeft:
            {
                if (pos.fTextByteIndex == 0) {
                    pos = this->move(Editor::Movement::kLeft, pos);
                    break;
                }
                const std::vector<bool>& words = fLines[pos.fParagraphIndex].fWordBoundaries;
                SkASSERT(words.size() == fLines[pos.fParagraphIndex].fText.size());
                do {
                    --pos.fTextByteIndex;
                } while (pos.fTextByteIndex > 0 && !words[pos.fTextByteIndex]);
            }
            break;
        case Editor::Movement::kWordRight:
            {
                const StringSlice& text = fLines[pos.fParagraphIndex].fText;
                if (pos.fTextByteIndex == text.size()) {
                    pos = this->move(Editor::Movement::kRight, pos);
                    break;
                }
                const std::vector<bool>& words = fLines[pos.fParagraphIndex].fWordBoundaries;
                SkASSERT(words.size() == text.size());
                do {
                    ++pos.fTextByteIndex;
                } while (pos.fTextByteIndex < text.size() && !words[pos.fTextByteIndex]);
            }
            break;

    }
    return pos;
}

void Editor::paint(SkCanvas* c, PaintOpts options) {
    this->reshapeAll();
    if (!c) {
        return;
    }

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
        c->drawRect(Editor::getLocation(options.fCursor), SkPaint(options.fCursorColor));
    }

    SkPaint foreground = SkPaint(options.fForegroundColor);
    for (const TextLine& line : fLines) {
        if (line.fBlob) {
            c->drawTextBlob(line.fBlob.get(), line.fOrigin.x(), line.fOrigin.y(), foreground);
        }
    }
}

void Editor::reshapeAll() {
    if (fNeedsReshape) {
        if (fLines.empty()) {
            fLines.push_back(TextLine());
        }
        float shape_width = (float)(fWidth);
        #ifdef SK_EDITOR_GO_FAST
        SkSemaphore semaphore;
        std::unique_ptr<SkExecutor> executor = SkExecutor::MakeFIFOThreadPool(100);
        int jobCount = 0;
        for (TextLine& line : fLines) {
            if (!line.fShaped) {
                executor->add([&]() {
                    ShapeResult result = Shape(line.fText.begin(), line.fText.size(),
                                               fFont, fLocale, shape_width);
                    line.fBlob           = std::move(result.blob);
                    line.fLineEndOffsets = std::move(result.lineBreakOffsets);
                    line.fCursorPos      = std::move(result.glyphBounds);
                    line.fWordBoundaries = std::move(result.wordBreaks);
                    line.fHeight         = result.verticalAdvance;
                    line.fShaped = true;
                    semaphore.signal();
                }
                ++jobCount;
            });
        }
        while (jobCount-- > 0) { semaphore.wait(); }
        #else
        int i = 0;
        for (TextLine& line : fLines) {
            if (!line.fShaped) {
                ShapeResult result = Shape(line.fText.begin(), line.fText.size(),
                                           fFont, fLocale, shape_width);
                line.fBlob           = std::move(result.blob);
                line.fLineEndOffsets = std::move(result.lineBreakOffsets);
                line.fCursorPos      = std::move(result.glyphBounds);
                line.fWordBoundaries = std::move(result.wordBreaks);
                line.fHeight         = result.verticalAdvance;
                line.fShaped = true;
            }
            ++i;
        }
        #endif
        int y = 0;
        for (TextLine& line : fLines) {
            line.fOrigin = {0, y};
            y += line.fHeight;
        }
        fHeight = y;
        fNeedsReshape = false;
    }
}

