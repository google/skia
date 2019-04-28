// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "editor.h"

#include "include/core/SkExecutor.h"
#include "include/core/SkCanvas.h"
#include "modules/skshaper/include/SkShaper.h"

using namespace editor;

static sk_sp<const SkTextBlob> shape(const SkString& text, const SkFont& font,
                                     const SkShaper* shaper, float width, int* height) {
    SkASSERT(height);
    SkTextBlobBuilderRunHandler textBlobBuilder(text.c_str(), {0, 0});
    shaper->shape(text.c_str(), text.size(), font, true, width, &textBlobBuilder);
    float h = std::max(textBlobBuilder.endPoint().y(), font.getSpacing());
    *height = (int)ceilf(h);
    return textBlobBuilder.makeBlob();
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
    SkPaint diff;
    diff.setColor(SK_ColorWHITE);
    diff.setBlendMode(SkBlendMode::kDifference);
    float left = (float)fMargin;
    float right = (float)(fWidth - fMargin);
    for (const TextLine& line : fLines) {
        if (line.fBlob) {
            c->drawTextBlob(line.fBlob.get(), left, (float)y, p);
        }
        if (line.fSelected) {
            c->drawRect(SkRect{left, (float)y, right, (float)(y + line.fHeight)}, diff);
        }
        y += line.fHeight;
    }
}

void Editor::setWidth(int w) {
    fWidth = w;
    float width = (float)(fWidth - 2 * fMargin);
    #ifdef SK_EDITOR_GO_FAST
    SkSemaphore semaphore;
    std::unique_ptr<SkExecutor> executor = SkExecutor::MakeFIFOThreadPool(100);
    for (TextLine& line : fLines) {
        executor->add([&]() {
            line.fBlob = shape(line.fText, fFont, shaper.get(), width, &line.fHeight);
            semaphore.signal();
        });
    }
    for (const TextLine& l : fLines) { semaphore.wait(); }
    #else
    auto shaper = SkShaper::Make();
    for (TextLine& line : fLines) {
        line.fBlob = shape(line.fText, fFont, shaper.get(), width, &line.fHeight);
    }
    #endif
    float h = 2.0f * fMargin;
    for (TextLine& line : fLines) {
        h += line.fHeight;
    }
    fHeight = (int)ceilf(h);
}
