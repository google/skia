/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "Resources.h"
#include "sk_tool_utils.h"
#include "SkAnimTimer.h"
#include "SkCanvas.h"
#include "SkPath.h"

// Reproduces https://code.google.com/p/chromium/issues/detail?id=279014

constexpr int kWidth = 440;
constexpr int kHeight = 440;
constexpr SkScalar kAngle = 0.305f;
constexpr int kMaxNumSteps = 140;

// Renders a string art shape.
// The particular shape rendered can be controlled by adjusting kAngle, from 0 to 1

class StringArtGM : public skiagm::GM {
public:
    StringArtGM() : fNumSteps(kMaxNumSteps) {}

protected:

    SkString onShortName() override {
        return SkString("stringart");
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, kHeight);
    }

    void onDraw(SkCanvas* canvas) override {
        SkScalar angle = kAngle*SK_ScalarPI + SkScalarHalf(SK_ScalarPI);
        SkScalar size = SkIntToScalar(SkMin32(kWidth, kHeight));
        SkPoint center = SkPoint::Make(SkScalarHalf(kWidth), SkScalarHalf(kHeight));
        SkScalar length = 5;
        SkScalar step = angle;

        SkPath path;
        path.moveTo(center);

        for (int i = 0; i < fNumSteps && length < (SkScalarHalf(size) - 10.f); ++i) {
            SkPoint rp = SkPoint::Make(length*SkScalarCos(step) + center.fX,
                                       length*SkScalarSin(step) + center.fY);
            path.lineTo(rp);
            length += angle / SkScalarHalf(SK_ScalarPI);
            step += angle;
        }

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(sk_tool_utils::color_to_565(0xFF007700));

        canvas->drawPath(path, paint);
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        constexpr SkScalar kDesiredDurationSecs = 3.0f;

        // Make the animation ping-pong back and forth but start in the fully drawn state
        SkScalar fraction = 1.0f - timer.scaled(2.0f/kDesiredDurationSecs, 2.0f);
        if (fraction <= 0.0f) {
            fraction = -fraction;
        }

        SkASSERT(fraction >= 0.0f && fraction <= 1.0f);

        fNumSteps = (int) (fraction * kMaxNumSteps);
        return true;
    }

private:
    int fNumSteps;

    typedef GM INHERITED;
};

DEF_GM( return new StringArtGM; )

/////////////////////////////////////////////////////////////////////////////////////////////////

#if 1
#include "Skottie.h"
#include "SkSGNode.h"

typedef SkTArray<sk_sp<sksg::Node>> NodeArray;

static sk_sp<skottie::Animation> custom_make(sk_sp<SkData> data, NodeArray* array) {
    skottie::Animation::Builder builder;

    builder.setNodeFinder("nm", [array](const char tagValue[], sksg::Node* node) {
        if (strcmp(tagValue, "\"Fill 1\"") == 0) {
            sk_sp<sksg::Node> n = sk_ref_sp(node);
            array->push_back(n);
        }
    });

    return builder.make((const char*)data->data(), data->size());
}

class SkottieGM : public skiagm::GM {
    enum {
        kWidth = 800,
        kHeight = 600,
    };

    sk_sp<skottie::Animation> fAnim;
    SkScalar                  fDur;

    NodeArray fNodes;

public:
    SkottieGM() {}
    ~SkottieGM() override {}

protected:

    SkString onShortName() override { return SkString("skottie"); }

    SkISize onISize() override { return SkISize::Make(kWidth, kHeight); }

    void init() {
        auto data = GetResourceAsData("skotty/sample_1_pretty.json");
        fAnim = custom_make(data, &fNodes);
        fDur = fAnim->duration();
    }

    void onDraw(SkCanvas* canvas) override {
        if (!fAnim) {
            this->init();
        }
        canvas->drawColor(0xFFBBBBBB);
        canvas->scale(4, 4);
        fAnim->render(canvas);
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        SkScalar time = (float)(fmod(timer.secs(), fDur) / fDur);
        fAnim->seek(time);

        SkColor4f c = {
            sinf(time*M_PI),
            cosf(time*M_PI),
            sinf(time*time*M_PI),
            1
        };
        for (auto& node : fNodes) {
            bool success = node->setColor(c);
            SkASSERT(success);
        }

        return true;
    }

private:
    typedef GM INHERITED;
};
DEF_GM( return new SkottieGM; )
#endif

