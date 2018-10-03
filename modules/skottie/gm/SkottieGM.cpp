/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "Resources.h"
#include "SkAnimCodecPlayer.h"
#include "SkAnimTimer.h"
#include "SkColor.h"
#include "SkMakeUnique.h"
#include "Skottie.h"
#include "SkottieProperty.h"

#include <cmath>
#include <vector>

using namespace skottie;

namespace {

static constexpr char kWebFontResource[] = "fonts/Roboto-Regular.ttf";
static constexpr char kSkottieResource[] = "skottie/skottie_sample_webfont.json";

// Dummy web font loader which serves a single local font (checked in under resources/).
class FakeWebFontProvider final : public ResourceProvider {
public:
    FakeWebFontProvider() : fFontData(GetResourceAsData(kWebFontResource)) {}

    sk_sp<SkData> loadFont(const char[], const char[]) const override {
        return fFontData;
    }

private:
    sk_sp<SkData> fFontData;

    using INHERITED = ResourceProvider;
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
            fAnimation = Animation::Builder()
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

    sk_sp<Animation> fAnimation;

    using INHERITED = skiagm::GM;
};

DEF_GM(return new SkottieWebFontGM;)

class SkottieColorizeGM : public skiagm::GM {
protected:
    SkString onShortName() override {
        return SkString("skottie_colorize");
    }

    SkISize onISize() override {
        return SkISize::Make(kSize, kSize);
    }

    void onOnceBeforeDraw() override {
        if (auto stream = GetResourceAsStream("skottie/skottie_sample_search.json")) {
            fColorizer = sk_make_sp<Colorizer>();
            fAnimation = Animation::Builder()
                            .setPropertyObserver(fColorizer)
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

    bool onHandleKey(SkUnichar uni) override {
        static constexpr SkColor kColors[] = {
            SK_ColorBLACK,
            SK_ColorRED,
            SK_ColorGREEN,
            SK_ColorYELLOW,
            SK_ColorCYAN,
        };

        if (uni == 'c') {
            fColorIndex = (fColorIndex + 1) % SK_ARRAY_COUNT(kColors);
            fColorizer->colorize(kColors[fColorIndex]);
            return true;
        }

        return false;
    }

private:
    class Colorizer final : public PropertyObserver {
    public:
        void onColorProperty(const char node_name[],
                             const PropertyObserver::LazyHandle<ColorPropertyHandle>& lh) override {
            fColorHandles.push_back(lh());
        }

        void colorize(SkColor c) {
            for (const auto& handle : fColorHandles) {
                handle->setColor(c);
            }
        }

    private:
        std::vector<std::unique_ptr<skottie::ColorPropertyHandle>> fColorHandles;
    };

    static constexpr SkScalar kSize = 800;

    sk_sp<Animation> fAnimation;
    sk_sp<Colorizer> fColorizer;
    size_t           fColorIndex = 0;

    using INHERITED = skiagm::GM;
};

DEF_GM(return new SkottieColorizeGM;)

class SkottieMultiFrameGM : public skiagm::GM {
public:
protected:
    SkString onShortName() override {
        return SkString("skottie_multiframe");
    }

    SkISize onISize() override {
        return SkISize::Make(kSize, kSize);
    }

    void onOnceBeforeDraw() override {
        if (auto stream = GetResourceAsStream("skottie/skottie_sample_multiframe.json")) {
            fAnimation = Animation::Builder()
                            .setResourceProvider(sk_make_sp<MultiFrameResourceProvider>())
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
    class MultiFrameImageAsset final : public skottie::ImageAsset {
    public:
        MultiFrameImageAsset() {
            if (auto codec = SkCodec::MakeFromData(GetResourceAsData("images/flightAnim.gif"))) {
                fPlayer = skstd::make_unique<SkAnimCodecPlayer>(std::move(codec));
            }
        }

        bool isMultiFrame() override { return fPlayer ? fPlayer->duration() > 0 : false; }

        sk_sp<SkImage> getFrame(float t) override {
            if (!fPlayer) {
                return nullptr;
            }

            fPlayer->seek(static_cast<uint32_t>(t * 1000));
            return fPlayer->getFrame();
        }

    private:
        std::unique_ptr<SkAnimCodecPlayer> fPlayer;
    };

    class MultiFrameResourceProvider final : public skottie::ResourceProvider {
    public:
        sk_sp<ImageAsset> loadImageAsset(const char[], const char[]) const override {
            return sk_make_sp<MultiFrameImageAsset>();
        }
    };

    static constexpr SkScalar kSize = 800;

    sk_sp<Animation> fAnimation;

    using INHERITED = skiagm::GM;
};

DEF_GM(return new SkottieMultiFrameGM;)
