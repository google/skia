/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkAnimTimer.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "Skottie.h"

class SkottieGM : public skiagm::GM {
    enum {
        kWidth = 800,
        kHeight = 600,
    };

    enum {
        N = 100,
    };
    skottie::Animation* fAnims[N];
    SkRect              fRects[N];
    SkScalar            fDur;

public:
    SkottieGM() {
        sk_bzero(fAnims, sizeof(fAnims));
    }
    ~SkottieGM() override {
        for (auto anim : fAnims) {
            SkSafeUnref(anim);
        }
    }

protected:

    SkString onShortName() override { return SkString("skottie"); }

    SkISize onISize() override { return SkISize::Make(kWidth, kHeight); }

    void init() {
        SkRandom rand;
        auto data = SkData::MakeFromFileName("/Users/reed/Downloads/maps_pinlet.json");
        for (int i = 0; i < N; ++i) {
            fAnims[i] = skottie::Animation::Make((const char*)data->data(), data->size()).release();
            SkScalar x = rand.nextF() * kWidth;
            SkScalar y = rand.nextF() * kHeight;
            fRects[i].setXYWH(x, y, 400, 400);
        }
        fDur = fAnims[0]->duration();
    }

    void onDraw(SkCanvas* canvas) override {
        if (!fAnims[0]) {
            this->init();
        }
        canvas->drawColor(0xFFBBBBBB);
        for (int i = 0; i < N; ++i) {
            fAnims[0]->render(canvas, &fRects[i]);
        }
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        SkScalar time = (float)(fmod(timer.secs(), fDur) / fDur);
        for (auto anim : fAnims) {
            anim->seek(time);
        }
        return true;
    }

private:
    typedef GM INHERITED;
};

DEF_GM( return new SkottieGM; )
