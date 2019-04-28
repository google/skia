// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "editor.h"

#include "include/core/SkExecutor.h"
#include "include/core/SkCanvas.h"
#include "modules/skshaper/include/SkShaper.h"

using namespace editor;

static sk_sp<const SkTextBlob> shape(const std::string& text, const SkFont& font,
                                     const SkShaper* shaper, float width, float* height) {
    SkASSERT(height);
    SkTextBlobBuilderRunHandler textBlobBuilder(text.c_str(), {0, 0});
    shaper->shape(text.c_str(), text.size(), font, true, width, &textBlobBuilder);
    *height = std::max(textBlobBuilder.endPoint().y(), font.getSpacing());
    return textBlobBuilder.makeBlob();
}

void Editor::paint(SkCanvas* c) {
    SkPaint background;
    background.setBlendMode(SkBlendMode::kSrc);
    background.setColor4f(fBackgroundColor, nullptr);
    c->drawPaint(background);
    float y = fMargin;
    SkPaint p;
    p.setColor4f(fForegroundColor, nullptr);
    SkPaint diff;
    diff.setColor(SK_ColorWHITE);
    diff.setBlendMode(SkBlendMode::kDifference);
    float left = (float)fMargin;
    float right = (float)(fWidth - fMargin);
    for (const TextLine& line : fLines) {
        if (line.fBlob) {
            c->drawTextBlob(line.fBlob.get(), fMargin, y, p);
        }
        if (line.fSelected) {
            c->drawRect(SkRect{left, y, right, y + line.fHeight}, diff);
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
    fHeight = (float)ceilf(h);
}
