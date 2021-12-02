/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTDArray.h"
#include "include/private/chromium/GrSlug.h"
#include "tools/ToolUtils.h"

#if SK_SUPPORT_GPU && defined(SK_EXPERIMENTAL_ADD_ATLAS_PADDING)
class SlugGM : public skiagm::GM {
public:
    SlugGM(const char* txt)
            : fText(txt) {
    }

protected:
    void onOnceBeforeDraw() override {
        fTypeface = ToolUtils::create_portable_typeface("serif", SkFontStyle());
        SkFont font(fTypeface);
        size_t txtLen = strlen(fText);
        int glyphCount = font.countText(fText, txtLen, SkTextEncoding::kUTF8);

        fGlyphs.append(glyphCount);
        font.textToGlyphs(fText, txtLen, SkTextEncoding::kUTF8, fGlyphs.begin(), glyphCount);
    }

    SkString onShortName() override {
        return SkString("slug");
    }

    SkISize onISize() override {
        return SkISize::Make(1000, 480);
    }

    void onDraw(SkCanvas* canvas) override {
        sk_sp<SkTextBlob> blob(this->makeBlob());
        SkPaint p;
        p.setAntiAlias(true);
        canvas->clipIRect(SkIRect::MakeSize(this->getISize()).makeInset(40, 50));
        canvas->scale(1.3f, 1.3f);
        sk_sp<GrSlug> slug = GrSlug::ConvertBlob(canvas, *blob, {10, 10}, p);
        if (slug == nullptr) {
            return;
        }
        canvas->translate(0.5, 0.5);
        canvas->translate(30, 30);
        canvas->drawTextBlob(blob, 10, 10, p);
        canvas->translate(370, 0);
        slug->draw(canvas);
        for (float scale = 1.5; scale < 4; scale += 0.5) {
            canvas->translate(-370, 20 * scale);
            canvas->save();
            canvas->scale(scale, scale);
            canvas->rotate(5);
            canvas->drawTextBlob(blob, 10, 10, p);
            canvas->restore();
            canvas->translate(370, 0);
            canvas->save();
            canvas->scale(scale, scale);
            canvas->rotate(5);

            slug->draw(canvas);
            canvas->restore();
        }
    }

private:
    sk_sp<SkTextBlob> makeBlob() {
        SkTextBlobBuilder builder;

        SkFont font;
        font.setSubpixel(true);
        font.setEdging(SkFont::Edging::kAntiAlias);
        font.setTypeface(fTypeface);
        font.setSize(16);

        const SkTextBlobBuilder::RunBuffer& buf = builder.allocRun(font, fGlyphs.count(), 0, 0);
        memcpy(buf.glyphs, fGlyphs.begin(), fGlyphs.count() * sizeof(uint16_t));
        return builder.make();
    }

    SkTDArray<uint16_t> fGlyphs;
    sk_sp<SkTypeface>   fTypeface;
    const char*         fText;
    using INHERITED = skiagm::GM;
};

DEF_GM(return new SlugGM("hamburgefons");)
#endif
