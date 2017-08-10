/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkAnimTimer.h"
#include "SkDrawable.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkDrawable.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRSXform.h"
#include "SkSurface.h"
#include "SkGradientShader.h"

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

class HasEventWig : public SkView {
public:
    void postWidgetEvent() {
        SkEvent evt;
        this->onPrepareWidEvent(&evt);
        this->postToListeners(evt, 0);
    }

protected:
    virtual void onPrepareWidEvent(SkEvent*) {}
};

static SkRandom gRand;

class PushButtonWig : public HasEventWig {
    SkString fLabel;
    SkColor  fColor;
    uint32_t fFast32;

public:
    PushButtonWig(const char label[], uint32_t fast) : fLabel(label) {
        fColor = (gRand.nextU() & 0x7F7F7F7F) | SkColorSetARGB(0xFF, 0, 0, 0x80);
        fFast32 = fast;
    }

protected:
    void onPrepareWidEvent(SkEvent* evt) override {
        evt->setType("push-button");
        evt->setFast32(fFast32);
        evt->setString("label", fLabel.c_str());
    }

//    bool onEvent(const SkEvent&) override;
    void onDraw(SkCanvas* canvas) override {
        SkRect r;
        this->getLocalBounds(&r);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(fColor);
        canvas->drawRoundRect(r, 8, 8, paint);

        paint.setColor(0xFFFFFFFF);
        paint.setTextSize(16);
        paint.setTextAlign(SkPaint::kCenter_Align);
        paint.setLCDRenderText(true);
        canvas->drawString(fLabel, r.centerX(), r.fTop + 0.68f * r.height(), paint);
    }

    Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) override {
        return new Click(this);
    }

    bool onClick(Click* click) override {
        SkRect target = SkRect::MakeXYWH(click->fCurr.x() - 1, click->fCurr.y() - 1, 3, 3);
        SkRect r;
        this->getLocalBounds(&r);
        if (r.intersects(target)) {
            fColor = SkColorSetA(fColor, 0x99);
        } else {
            fColor = SkColorSetA(fColor, 0xFF);
        }
        this->inval(nullptr);

        if (click->fState == SkView::Click::kUp_State) {
            this->postWidgetEvent();
        }
        return true;
    }

private:
    typedef HasEventWig INHERITED;
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

class XferDemo : public SampleView {
    enum {
        N = 4
    };

    SkRect        fModeRect[N_Modes];
    sk_sp<CircDrawable> fDrs[N];
    CircDrawable* fSelected;

    void addButtons() {
        SkScalar x = 10;
        SkScalar y = 10;
        for (int i = 0; i < N_Modes; ++i) {
            sk_sp<SkView> v(new PushButtonWig(SkBlendMode_Name(gModes[i]), (int)gModes[i]));
            v->setSize(70, 25);
            v->setLoc(x, y);
            v->setVisibleP(true);
            v->setEnabledP(true);
            v->addListenerID(this->getSinkID());
            this->attachChildToFront(v.get());
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
    bool onEvent(const SkEvent& evt) override {
        if (evt.isType("push-button")) {
            if (fSelected) {
                fSelected->fMode = (SkBlendMode)evt.getFast32();
                this->inval(nullptr);
            }
            return true;
        }
        return this->INHERITED::onEvent(evt);
    }

    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "XferDemo");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
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

    SkView::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned) override {
        fSelected = nullptr;
        for (int i = N - 1; i >= 0; --i) {
            if (fDrs[i]->hitTest(x, y)) {
                fSelected = fDrs[i].get();
                break;
            }
        }
        this->inval(nullptr);
        return fSelected ? new Click(this) : nullptr;
    }

    bool onClick(Click* click) override {
        fSelected->fLoc.fX += click->fCurr.fX - click->fPrev.fX;
        fSelected->fLoc.fY += click->fCurr.fY - click->fPrev.fY;
        this->inval(nullptr);
        return true;
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new XferDemo; )
