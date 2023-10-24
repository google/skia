/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkFont.h"
#include "include/core/SkPath.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "include/utils/SkTextUtils.h"
#include "src/base/SkRandom.h"
#include "tools/DecodeUtils.h"
#include "tools/viewer/ClickHandlerSlide.h"

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
const int N_Modes = std::size(gModes);

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
        return fRect.intersects({x - 1, y - 1, x + 1, y + 1});
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
                                                                     SkTileMode::kClamp));
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

class XferSlide : public ClickHandlerSlide {
public:
    XferSlide() {
        const SkColor colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorBLACK };
        for (int i = 0; i < N; ++i) {
            fDrs[i].reset(new CircDrawable(200, colors[i]));
            fDrs[i]->fLoc.set(100.f + i * 100, 100.f + i * 100);
            fDrs[i]->fMode = SkBlendMode::kSrcOver;
        }
        fSelected = nullptr;

        this->addButtons();
        fName = "XferDemo";
    }

    void draw(SkCanvas* canvas) override {
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

protected:
    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey) override {
        // Check mode buttons first
        for (int i = 0; i < N_Modes; ++i) {
            if (fModeButtons[i].hitTest(x, y)) {
                Click* click = new Click();
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
        return fSelected ? new Click() : nullptr;
    }

    bool onClick(Click* click) override {
        int32_t mode;
        if (click->fMeta.findS32("mode", &mode)) {
            if (fSelected && skui::InputState::kUp == click->fState) {
                fSelected->fMode = gModes[mode];
            }
        } else {
            fSelected->fLoc.fX += click->fCurr.fX - click->fPrev.fX;
            fSelected->fLoc.fY += click->fCurr.fY - click->fPrev.fY;
        }
        return true;
    }

private:
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
};

DEF_SLIDE( return new XferSlide; )

//////////////////////////////////////////////////////////////////////////////

#include "tools/Resources.h"

class CubicResamplerSlide : public ClickHandlerSlide {
public:
    CubicResamplerSlide() {
        fName = "CubicResampler";
    }

protected:
    void load(SkScalar, SkScalar) override {
        SkRect r = {10, 10, 200, 200};
        for (const char* name : {"images/mandrill_128.png",
                                 "images/rle.bmp",
                                 "images/example_4.png"}) {
            fRecs.push_back({ToolUtils::GetResourceAsImage(name), r});
            r.offset(0, r.height() + 10);
        }
        fDomain.setXYWH(r.fLeft + 3 * r.width() + 40, 50, 200, 200);
        fCubic = {.3f, .5f};
    }

    void draw(SkCanvas* canvas) override {
        for (const auto& rec : fRecs) {
            rec.draw(canvas, fCubic);
        }

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStroke(true);
        canvas->drawRect(fDomain, paint);

        paint.setColor(SK_ColorRED);
        paint.setStroke(false);
        SkPoint loc = SkMatrix::RectToRect({0,0,1,1}, fDomain).mapXY(fCubic.B, fCubic.C);
        canvas->drawCircle(loc.fX, loc.fY, 8, paint);

        SkString str;
        str.printf("B=%4.2f  C=%4.2f", fCubic.B, fCubic.C);
        SkFont font;
        font.setSize(25);
        font.setEdging(SkFont::Edging::kAntiAlias);
        paint.setColor(SK_ColorBLACK);
        canvas->drawSimpleText(str.c_str(), str.size(), SkTextEncoding::kUTF8,
                               fDomain.fLeft + 10, fDomain.fBottom + 40, font, paint);
    }

    static float pin_unitize(float min, float max, float value) {
        return (std::min(std::max(value, min), max) - min) / (max - min);
    }
    static SkPoint pin_unitize(const SkRect& r, SkPoint p) {
        return {
            pin_unitize(r.fLeft, r.fRight,  p.fX),
            pin_unitize(r.fTop,  r.fBottom, p.fY),
        };
    }

protected:
    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey) override {
        if (fDomain.contains(x, y)) {
            return new Click([this](Click* click) {
                auto [B, C] = pin_unitize(fDomain, click->fCurr);
                fCubic = {B, C};
                return true;
            });
        }
        return nullptr;
    }

    bool onClick(ClickHandlerSlide::Click *) override { return false; }

private:
    struct Rec {
        sk_sp<SkImage>  fImage;
        SkRect          fBounds;

        void draw(SkCanvas* canvas, SkCubicResampler cubic) const {
            SkRect r = fBounds;
            SkPaint paint;

            SkMatrix lm = SkMatrix::Translate(r.x(), r.y())
                          * SkMatrix::Scale(10, 10);
            paint.setShader(fImage->makeShader(SkSamplingOptions(), lm));
            canvas->drawRect(r, paint);

            r.offset(r.width() + 10, 0);
            lm.postTranslate(r.width() + 10, 0);

            paint.setShader(fImage->makeShader(SkSamplingOptions(SkFilterMode::kLinear), lm));
            canvas->drawRect(r, paint);

            r.offset(r.width() + 10, 0);
            lm.postTranslate(r.width() + 10, 0);

            paint.setShader(fImage->makeShader(SkTileMode::kClamp, SkTileMode::kClamp,
                                               SkSamplingOptions(cubic), &lm));
            canvas->drawRect(r, paint);
        }
    };

    std::vector<Rec>  fRecs;
    SkRect            fDomain;
    SkCubicResampler  fCubic;
};

DEF_SLIDE( return new CubicResamplerSlide; )
