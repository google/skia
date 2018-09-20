/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "Resources.h"
#include "SkAnimTimer.h"
#include "Skottie.h"

#include <cmath>

namespace {

static constexpr char kWebFontResource[] = "fonts/Roboto-Regular.ttf";
static constexpr char kSkottieResource[] = "skottie/skottie_sample_webfont.json";

// Dummy web font loader which serves a single local font (checked in under resources/).
class FakeWebFontProvider final : public skottie::ResourceProvider {
public:
    FakeWebFontProvider() : fFontData(GetResourceAsData(kWebFontResource)) {}

    sk_sp<SkData> loadWebFont(const char[]) const {
        return fFontData;
    }

private:
    sk_sp<SkData> fFontData;

    using INHERITED = skottie::ResourceProvider;
};

} // namespace

class SkottieWebFontGM : public skiagm::GM {
public:
protected:
    SkString onShortName() override {
        return SkString("skottie_webfont");
    }

    SkISize onISize() override {
        return SkISize::Make(kSize, kSize);
    }

    void onOnceBeforeDraw() override {
        if (auto stream = GetResourceAsStream(kSkottieResource)) {
            fAnimation = skottie::Animation::Builder()
                            .setResourceProvider(sk_make_sp<FakeWebFontProvider>())
                            .make(stream.get());
        }
    }

    void onDraw(SkCanvas* canvas) override {
        if (!fAnimation) {
            return;
        }

        auto dest = SkRect::MakeWH(kSize, kSize);
        fAnimation->render(canvas, &dest);
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        if (!fAnimation) {
            return false;
        }

        const auto duration = fAnimation->duration();
        fAnimation->seek(std::fmod(timer.secs(), duration) / duration);
        return true;
    }

private:
    static constexpr SkScalar kSize = 800;

    sk_sp<skottie::Animation> fAnimation;

    using INHERITED = skiagm::GM;
};

DEF_GM(return new SkottieWebFontGM;)
