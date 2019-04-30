// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "editor.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkExecutor.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkPath.h"
#include "modules/skshaper/include/SkShaper.h"
#include "src/utils/SkUTF.h"

#include "run_handler.h"

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

void Editor::Shape(TextLine* line, SkShaper* shaper, float width, const SkFont& font) {
    SkASSERT(line);
    SkASSERT(shaper);
    line->fCursorPos.resize(line->fText.size());
    for (SkRect& c : line->fCursorPos) {
        c = kUnsetRect;
    }
    RunHandler runHandler(line->fText.c_str(), line->fText.size());
    runHandler.setRunCallback(callback_fn, line->fCursorPos.data());
    shaper->shape(line->fText.c_str(), line->fText.size(), font, true, width, &runHandler);
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
            lines.push_back(Editor::TextLine(SkString(p, s)));
        });
    }
    fLines = lines;
}

void Editor::paint(SkCanvas* c) {
    SkPaint background;
    background.setBlendMode(SkBlendMode::kSrc);
    background.setColor4f(fBackgroundColor, nullptr);
    c->drawPaint(background);
    int y = fMargin;
    SkPaint p;
    p.setColor4f(fForegroundColor, nullptr);
    //SkPaint diff;
    //diff.setColor(SK_ColorWHITE);
    //diff.setBlendMode(SkBlendMode::kDifference);
    float left = (float)fMargin;
    //float right = (float)(fWidth - fMargin);
    SkPaint stroke;
    stroke.setStyle(SkPaint::kStroke_Style);
    stroke.setColor(SkColorSetARGB(0xFF, 0xFF, 0xFF, 0xFF));
    SkRect lastRect = kUnsetRect;
    for (const TextLine& line : fLines) {
        if (line.fSelected) {
            SkPath path;
            for (unsigned i = 0; i < line.fText.size(); ++i) {
                SkRect r = line.fCursorPos[i];
                if (r == kUnsetRect) {
                    continue;
                }
                r.offset(left, (float)y);
                if (r != lastRect) {
                    path.addRect(r);
                    //c->drawRect(r, stroke);
                    lastRect = r;
                }
            }
            c->drawPath(path, stroke);
        }
        if (line.fBlob) {
            c->drawTextBlob(line.fBlob.get(), left, (float)y, p);
        }
        y += line.fHeight;
    }
}

void Editor::setWidth(int w) {
    fWidth = w;
    float shape_width = (float)(fWidth - 2 * fMargin);
    #ifdef SK_EDITOR_GO_FAST
    SkSemaphore semaphore;
    std::unique_ptr<SkExecutor> executor = SkExecutor::MakeFIFOThreadPool(100);
    for (TextLine& line : fLines) {
        executor->add([&]() {
            auto shaper = SkShaper::Make();
            Editor::Shape(&line, shaper.get(), shape_width, fFont);
            semaphore.signal();
        });
    }
    for (const TextLine& l : fLines) { semaphore.wait(); }
    #else
    auto shaper = SkShaper::Make();
    for (TextLine& line : fLines) {
        Editor::Shape(&line, shaper.get(), shape_width, fFont);
    }
    #endif
    float h = 2.0f * fMargin;
    for (TextLine& line : fLines) {
        h += line.fHeight;
    }
    fHeight = (int)ceilf(h);
}
