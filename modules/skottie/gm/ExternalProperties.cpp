/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkColor.h"
#include "include/utils/SkAnimCodecPlayer.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/include/SkottieProperty.h"
#include "modules/skottie/utils/SkottieUtils.h"
#include "modules/skresources/include/SkResources.h"
#include "tools/Resources.h"

#include <cmath>
#include <vector>

namespace {

static constexpr char kWebFontResource[] = "fonts/Roboto-Regular.ttf";
static constexpr char kSkottieResource[] = "skottie/skottie_sample_webfont.json";

// Mock web font loader which serves a single local font (checked in under resources/).
class FakeWebFontProvider final : public skresources::ResourceProvider {
public:
    FakeWebFontProvider()
        : fTypeface(SkTypeface::MakeFromData(GetResourceAsData(kWebFontResource))) {}

    sk_sp<SkTypeface> loadTypeface(const char[], const char[]) const override {
        return fTypeface;
    }

private:
    sk_sp<SkTypeface> fTypeface;

    using INHERITED = skresources::ResourceProvider;
};

} // namespace

class SkottieExternalPropsGM : public skiagm::GM {
public:
protected:
    SkString onShortName() override {
        return SkString("skottie_external_props");
    }

    SkISize onISize() override {
        return SkISize::Make(kSize, kSize);
    }

    void onOnceBeforeDraw() override {
        if (auto stream = GetResourceAsStream(kSkottieResource)) {
            fPropManager = std::make_unique<skottie_utils::CustomPropertyManager>();
            fAnimation = skottie::Animation::Builder()
                            .setResourceProvider(sk_make_sp<FakeWebFontProvider>())
                            .setPropertyObserver(fPropManager->getPropertyObserver())
                            .make(stream.get());
        }
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        if (!fAnimation) {
            *errorMsg = "No animation";
            return DrawResult::kFail;
        }

        for (size_t i = 0; i < 4; ++i) {
            this->update_props(i);
            auto dest = SkRect::MakeWH(kSize/2, kSize/2).makeOffset(kSize * (i & 1) / 2,
                                                                    kSize * (i & 2) / 4);
            fAnimation->render(canvas, &dest);
        }
        return DrawResult::kOk;
    }

    bool onAnimate(double nanos) override {
        if (!fAnimation) {
            return false;
        }

        const auto duration = fAnimation->duration();
        fAnimation->seek(std::fmod(1e-9 * nanos, duration) / duration);
        return true;
    }

private:
    void update_props(size_t i) {

        SkASSERT(i < 4);
        if (!i) {
            return;
        }

        static constexpr struct {
            const char* txt_string;
            SkColor     txt_color,
                        solid_color;
            float       transform_scale;
        } gTests[] = {
            { "update #1", SK_ColorRED    , SK_ColorYELLOW, 100.f },
            { "update #2", SK_ColorGREEN  , SK_ColorBLUE  ,  50.f },
            { "update #3", SK_ColorMAGENTA, SK_ColorCYAN  , 150.f },
        };

        SkASSERT(i - 1 < SK_ARRAY_COUNT(gTests));
        const auto& tst = gTests[i - 1];

        for (const auto& prop : fPropManager->getColorProps()) {
            SkAssertResult(fPropManager->setColor(prop, tst.solid_color));
        }

        for (const auto& prop : fPropManager->getTransformProps()) {
            auto t = fPropManager->getTransform(prop);
            t.fScale = {tst.transform_scale, tst.transform_scale};
            SkAssertResult(fPropManager->setTransform(prop, t));
        }

        for (const auto& prop : fPropManager->getTextProps()) {
            auto txt = fPropManager->getText(prop);
            txt.fText.set(tst.txt_string);
            txt.fFillColor = tst.txt_color;
            SkAssertResult(fPropManager->setText(prop, txt));
        }
    }

    inline static constexpr SkScalar kSize = 800;

    sk_sp<skottie::Animation>                             fAnimation;
    std::unique_ptr<skottie_utils::CustomPropertyManager> fPropManager;

    using INHERITED = skiagm::GM;
};

DEF_GM(return new SkottieExternalPropsGM;)
