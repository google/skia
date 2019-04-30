/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if SK_SUPPORT_ATLAS_TEXT

#include "gm/gm.h"
#include "include/atlastext/SkAtlasTextContext.h"
#include "include/atlastext/SkAtlasTextFont.h"
#include "include/atlastext/SkAtlasTextRenderer.h"
#include "include/atlastext/SkAtlasTextTarget.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/utils/SkRandom.h"
#include "src/utils/SkUTF.h"
#include "tools/ToolUtils.h"
#include "tools/gpu/atlastext/GLTestAtlasTextRenderer.h"
#include "tools/gpu/atlastext/TestAtlasTextRenderer.h"

#include <memory>
#include <utility>

class GrContext;
class GrRenderTargetContext;

// GM that draws text using the Atlas Text interface offscreen and then blits that to the canvas.

static SkScalar draw_string(SkAtlasTextTarget* target, const SkString& text, SkScalar x, SkScalar y,
                            uint32_t color, sk_sp<SkTypeface> typeface, float size) {
    if (!text.size()) {
        return x;
    }
    auto atlas_font = SkAtlasTextFont::Make(typeface, size);
    int cnt = SkUTF::CountUTF8(text.c_str(), text.size());
    std::unique_ptr<SkGlyphID[]> glyphs(new SkGlyphID[cnt]);

    // Using a paint to get the positions for each glyph.
    SkFont font;
    font.setSize(size);
    font.setTypeface(std::move(typeface));

    font.textToGlyphs(text.c_str(), text.size(), SkTextEncoding::kUTF8, glyphs.get(), cnt);

    std::unique_ptr<SkScalar[]> widths(new SkScalar[cnt]);
    font.getWidths(glyphs.get(), cnt, widths.get());

    std::unique_ptr<SkPoint[]> positions(new SkPoint[cnt]);
    positions[0] = {x, y};
    for (int i = 1; i < cnt; ++i) {
        positions[i] = {positions[i - 1].fX + widths[i - 1], y};
    }

    target->drawText(glyphs.get(), positions.get(), cnt, color, *atlas_font);

    // Return the width of the of draw.
    return positions[cnt - 1].fX + widths[cnt - 1] - positions[0].fX;
}

class AtlasTextGM : public skiagm::GpuGM {
public:
    AtlasTextGM() = default;

protected:
    SkString onShortName() override { return SkString("atlastext"); }

    SkISize onISize() override { return SkISize::Make(kSize, kSize); }

    void onOnceBeforeDraw() override {
        fRenderer = sk_gpu_test::MakeGLTestAtlasTextRenderer();
        if (!fRenderer) {
            return;
        }
        fContext = SkAtlasTextContext::Make(fRenderer);
        auto targetHandle = fRenderer->makeTargetHandle(kSize, kSize);
        if (!targetHandle) {
            return;
        }

        fTarget = SkAtlasTextTarget::Make(fContext, kSize, kSize, targetHandle);

        fTypefaces[0] = ToolUtils::create_portable_typeface("serif", SkFontStyle::Italic());
        fTypefaces[1] = ToolUtils::create_portable_typeface("sans-serif", SkFontStyle::Italic());
        fTypefaces[2] = ToolUtils::create_portable_typeface("serif", SkFontStyle::Normal());
        fTypefaces[3] = ToolUtils::create_portable_typeface("sans-serif", SkFontStyle::Normal());
        fTypefaces[4] = ToolUtils::create_portable_typeface("serif", SkFontStyle::Bold());
        fTypefaces[5] = ToolUtils::create_portable_typeface("sans-serif", SkFontStyle::Bold());
    }

    DrawResult onDraw(GrContext*,
                      GrRenderTargetContext*,
                      SkCanvas* canvas,
                      SkString* errorMsg) override {
        if (!fRenderer) {
            *errorMsg = "No renderer... probably not supported.";
            return DrawResult::kSkip;
        }
        if (!fTarget || !fTarget->handle()) {
            *errorMsg = "No target... we can't continue.";
            return DrawResult::kFail;
        }
        fRenderer->clearTarget(fTarget->handle(), 0xFF808080);
        auto bmp = this->drawText();
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc);
        canvas->drawBitmap(bmp, 0, 0);
        return DrawResult::kOk;
    }

private:
    SkBitmap drawText() {
        static const int kSizes[] = {8, 13, 18, 23, 30};

        static const SkString kTexts[] = {SkString("ABCDEFGHIJKLMNOPQRSTUVWXYZ"),
                                          SkString("abcdefghijklmnopqrstuvwxyz"),
                                          SkString("0123456789"),
                                          SkString("!@#$%^&*()<>[]{}")};
        SkScalar x = 0;
        SkScalar y = 10;

        SkRandom random;
        do {
            for (auto s : kSizes) {
                auto size = 2 * s;
                for (const auto& typeface : fTypefaces) {
                    for (const auto& text : kTexts) {
                        // Choose a random color but don't let alpha be too small to see.
                        uint32_t color = random.nextU() | 0x40000000;
                        fTarget->save();
                        // Randomly add a little bit of perspective
                        if (random.nextBool()) {
                            SkMatrix persp;
                            persp.reset();
                            persp.setPerspY(0.0005f);
                            persp.preTranslate(-x, -y + s);
                            persp.postTranslate(x, y - s);
                            fTarget->concat(persp);
                        }
                        // Randomly switch between positioning with a matrix vs x, y passed to draw.
                        SkScalar drawX = x, drawY = y;
                        if (random.nextBool()) {
                            fTarget->translate(x, y);
                            drawX = drawY = 0;
                        }
                        x += size +
                             draw_string(fTarget.get(), text, drawX, drawY, color, typeface, size);
                        x = SkScalarCeilToScalar(x);
                        fTarget->restore();
                        // Flush periodically to test continued drawing after a flush.
                        if ((random.nextU() % 8) == 0) {
                            fTarget->flush();
                        }
                        if (x + 100 > kSize) {
                            x = 0;
                            y += SkScalarCeilToScalar(size + 3);
                            if (y > kSize) {
                                fTarget->flush();
                                return fRenderer->readTargetHandle(fTarget->handle());
                            }
                        }
                    }
                }
            }
        } while (true);
    }

    static constexpr int kSize = 1280;

    sk_sp<SkTypeface> fTypefaces[6];
    sk_sp<sk_gpu_test::TestAtlasTextRenderer> fRenderer;
    std::unique_ptr<SkAtlasTextTarget> fTarget;
    sk_sp<SkAtlasTextContext> fContext;

    typedef GM INHERITED;
};

constexpr int AtlasTextGM::kSize;

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new AtlasTextGM;)

#endif
