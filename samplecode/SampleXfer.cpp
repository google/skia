/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "AnimTimer.h"
#include "Sample.h"
#include "SkCanvas.h"
#include "SkDrawable.h"
#include "SkGradientShader.h"
#include "SkPath.h"
#include "SkRSXform.h"
#include "SkRandom.h"
#include "SkString.h"
#include "SkSurface.h"
#include "SkTextUtils.h"

const SkBlendMode gModes[] = {
    SkBlendMode::kSrcOver,
    SkBlendMode::kSrc,
    SkBlendMode::kSrcIn,
    SkBlendMode::kSrcOut,
    SkBlendMode::kSrcATop,
    SkBlendMode::kDstOver,
    SkBlendMode::kDstIn,
    SkBlendMode::kDstOut,
    SkBlendMode::kDstATop,
};
const int N_Modes = SK_ARRAY_COUNT(gModes);

static SkRandom gRand;

struct ModeButton {
    SkString fLabel;
    SkColor  fColor;
    SkRect   fRect;

public:
    void init(const char label[], const SkRect& rect) {
        fLabel = label;
        fRect = rect;
        fColor = (gRand.nextU() & 0x7F7F7F7F) | SkColorSetARGB(0xFF, 0, 0, 0x80);
    }

    void draw(SkCanvas* canvas) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(fColor);
        canvas->drawRoundRect(fRect, 8, 8, paint);

        paint.setColor(0xFFFFFFFF);
        SkFont font;
        font.setSize(16);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        SkTextUtils::DrawString(canvas, fLabel.c_str(), fRect.centerX(), fRect.fTop + 0.68f * fRect.height(),
                                font, paint, SkTextUtils::kCenter_Align);
    }

    bool hitTest(SkScalar x, SkScalar y) {
        return fRect.intersects(x - 1, y - 1, x + 1, y + 1);
    }
};

class ModeDrawable : public SkDrawable {
public:
    ModeDrawable() : fMode(SkBlendMode::kSrcOver), fLoc(SkPoint::Make(0, 0)) {}

    SkBlendMode fMode;
    SkPoint     fLoc;

    bool hitTest(SkScalar x, SkScalar y) {
        SkRect target = SkRect::MakeXYWH(x - fLoc.x() - 1, y - fLoc.y() - 1, 3, 3);
        return this->getBounds().intersects(target);
    }
};

class CircDrawable : public ModeDrawable {
    SkPaint fPaint;
    SkRect  fBounds;

public:
    CircDrawable(SkScalar size, SkColor c) {
        const SkColor colors[] = { 0, c };
        fPaint.setShader(SkGradientShader::MakeRadial(SkPoint::Make(size/2, size/2), size/2,
                                                                     colors, nullptr, 2,
                                                                     SkShader::kClamp_TileMode));
        fBounds = SkRect::MakeWH(size, size);
    }

protected:
    SkRect onGetBounds() override {
        return fBounds;
    }

    void onDraw(SkCanvas* canvas) override {
        fPaint.setBlendMode(fMode);
        canvas->save();
        canvas->translate(fLoc.x(), fLoc.y());
        canvas->drawOval(fBounds, fPaint);
        canvas->restore();
    }
};

class XferDemo : public Sample {
    enum {
        N = 4
    };

    SkRect        fModeRect[N_Modes];
    ModeButton    fModeButtons[N_Modes];
    sk_sp<CircDrawable> fDrs[N];
    CircDrawable* fSelected;

    void addButtons() {
        SkScalar x = 10;
        SkScalar y = 10;
        for (int i = 0; i < N_Modes; ++i) {
            fModeButtons[i].init(SkBlendMode_Name(gModes[i]), SkRect::MakeXYWH(x, y, 70, 25));
            fModeRect[i] = SkRect::MakeXYWH(x, y + 28, 70, 2);
            x += 80;
        }
    }

public:
    XferDemo() {
        const SkColor colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorBLACK };
        for (int i = 0; i < N; ++i) {
            fDrs[i].reset(new CircDrawable(200, colors[i]));
            fDrs[i]->fLoc.set(100.f + i * 100, 100.f + i * 100);
            fDrs[i]->fMode = SkBlendMode::kSrcOver;
        }
        fSelected = nullptr;

        this->addButtons();
    }

protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "XferDemo");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        for (int i = 0; i < N_Modes; ++i) {
            fModeButtons[i].draw(canvas);
        }

        SkPaint paint;
        if (fSelected) {
            for (int i = 0; i < N_Modes; ++i) {
                if (fSelected->fMode == gModes[i]) {
                    canvas->drawRect(fModeRect[i], paint);
                    break;
                }
            }
        }

        canvas->saveLayer(nullptr, nullptr);
        for (int i = 0; i < N; ++i) {
            fDrs[i]->draw(canvas);
        }
        canvas->restore();
    }

    Sample::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned) override {
        // Check mode buttons first
        for (int i = 0; i < N_Modes; ++i) {
            if (fModeButtons[i].hitTest(x, y)) {
                Click* click = new Click(this);
                click->fMeta.setS32("mode", i);
                return click;
            }
        }
        fSelected = nullptr;
        for (int i = N - 1; i >= 0; --i) {
            if (fDrs[i]->hitTest(x, y)) {
                fSelected = fDrs[i].get();
                break;
            }
        }
        return fSelected ? new Click(this) : nullptr;
    }

    bool onClick(Click* click) override {
        int32_t mode;
        if (click->fMeta.findS32("mode", &mode)) {
            if (fSelected && Click::kUp_State == click->fState) {
                fSelected->fMode = gModes[mode];
            }
        } else {
            fSelected->fLoc.fX += click->fCurr.fX - click->fPrev.fX;
            fSelected->fLoc.fY += click->fCurr.fY - click->fPrev.fY;
        }
        return true;
    }

private:
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new XferDemo; )
