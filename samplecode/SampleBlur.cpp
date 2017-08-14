/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkGradientShader.h"
#include "SkRandom.h"
#include "SkTypeface.h"
#include "SkTextBlob.h"
#include "SkUtils.h"
#include "SkView.h"

void get_row(SkUnichar base, SkUnichar glyphs[16]) {
    for (int i = 0x0; i <= 0xF; ++i) {
        glyphs[i] = base;
        glyphs[i] |= i;
    }
}

static void add_to_text_blob(SkTextBlobBuilder* builder,
                             const char* text, int len, const SkPaint& origPaint,
                             SkScalar x, SkScalar y) {
    SkPaint paint(origPaint);
    SkTDArray<uint16_t> glyphs;

    glyphs.append(paint.textToGlyphs(text, len, nullptr));
    paint.textToGlyphs(text, len, glyphs.begin());

    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
    const SkTextBlobBuilder::RunBuffer& run = builder->allocRun(paint, glyphs.count(), x, y,
                                                                nullptr);
    memcpy(run.glyphs, glyphs.begin(), glyphs.count() * sizeof(uint16_t));
}

#include "SkStream.h"

std::unique_ptr<SkFILEStream> GetResourceAsStream(const char* resource) {
    auto stream = SkFILEStream::Make(resource);
    if (!stream) {
        SkDebugf("Resource %s not found.\n", resource);
        return nullptr;
    }
    return std::move(stream);
}

sk_sp<SkTypeface> bar(const char* resource) {
    std::unique_ptr<SkFILEStream> stream(GetResourceAsStream(resource));
    if (!stream) {
        return nullptr;
    }
    return SkTypeface::MakeFromStream(stream.release());
}

class BlurView : public SampleView {
    SkBitmap    fBM;
public:
    BlurView() {}

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Blur2");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
    }

    void init() {
        //sk_sp<SkTypeface> typeface = SkTypeface::MakeFromName("Arial", //Noto Sans CJK SC",
        //                                                      SkFontStyle());

        fTypeface = bar("C:\\src\\skia.1\\android-fonts\\NotoSansCJK-Regular.ttc");

        SkPaint paint;
        paint.setTypeface(fTypeface);
        paint.setTextSize(kTextSize);
        paint.setTextEncoding(SkPaint::kUTF32_TextEncoding);

        paint.getFontMetrics(&fMetrics);

        SkUnichar glyphs[16];

        for (int32_t i = 0x4F00; i < 0x9FA0; i += 0x10) {

            get_row(i, glyphs);

            SkTextBlobBuilder builder;

            add_to_text_blob(&builder, (const char*) glyphs, 16*4, paint, 0, 0);

            fBlobs.emplace_back(builder.make());
        }
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        if (!fInitialized) {
            this->init();
            fInitialized = true;
        }

        //canvas->clear(0xFFF0E0F0);

        SkPaint paint;
        paint.setTypeface(fTypeface);
        paint.setTextSize(kTextSize);
        paint.setTextEncoding(SkPaint::kUTF32_TextEncoding);

        SkScalar y = 0.0f;
        for (SkScalar y = 0.0f; y < 2560.0f; ) {
            int index = fRand.nextRangeU(0, fBlobs.count()-1);

            y += -fMetrics.fAscent;
            canvas->drawTextBlob(fBlobs[index], 0, y, paint);

            y += fMetrics.fDescent + fMetrics.fLeading;
        }

        this->inval(nullptr);
    }

private:
    static const int kTextSize = 20;

    bool                        fInitialized = false;
    sk_sp<SkTypeface>           fTypeface;
    SkPaint::FontMetrics        fMetrics;
    SkTArray<sk_sp<SkTextBlob>> fBlobs;
    SkRandom                    fRand;

    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new BlurView; }
static SkViewRegister reg(MyFactory);
