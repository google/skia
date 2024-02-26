/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "modules/skottie/include/Skottie.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"

class DecodeBench : public Benchmark {
protected:
    DecodeBench(const char* name, const char* source)
        : fName(SkStringPrintf("decode_%s", name))
        , fSource(source)
    {}

    bool isSuitableFor(Backend backend) final {
            return backend == Backend::kNonRendering;
    }

    const char* onGetName() final { return fName.c_str(); }

    void onDelayedSetup() override {
        fData = GetResourceAsData(fSource);
        SkASSERT(fData);
    }

protected:
    sk_sp<SkData>  fData;

private:
    const SkString fName;
    const char*    fSource;
};

class BitmapDecodeBench final : public DecodeBench {
public:
    BitmapDecodeBench(const char* name, const char* source)
        : INHERITED(name, source)
    {}

    void onDraw(int loops, SkCanvas*) override {
        while (loops-- > 0) {
            SkBitmap bm;
            SkAssertResult(ToolUtils::DecodeDataToBitmap(fData, &bm));
        }
    }

private:
    using INHERITED = DecodeBench;
};


class SkottieDecodeBench final : public DecodeBench {
public:
    SkottieDecodeBench(const char* name, const char* source)
        : INHERITED(name, source)
    {}

    void onDraw(int loops, SkCanvas*) override {
        while (loops-- > 0) {
            const auto anim = skottie::Animation::Builder()
                .setFontManager(ToolUtils::TestFontMgr())
                .make(reinterpret_cast<const char*>(fData->data()),
                                                    fData->size());
        }
    }

private:
    using INHERITED = DecodeBench;
};

class SkottiePictureDecodeBench final : public DecodeBench {
public:
    SkottiePictureDecodeBench(const char* name, const char* source)
        : INHERITED(name, source)
    {}

    void onDraw(int loops, SkCanvas*) override {
        while (loops-- > 0) {
            const auto anim = skottie::Animation::Builder()
                .setFontManager(ToolUtils::TestFontMgr())
                .make(reinterpret_cast<const char*>(fData->data()),
                                                    fData->size());
            SkPictureRecorder recorder;
            anim->seek(0);
            anim->render(recorder.beginRecording(anim->size().width(), anim->size().height()));

            const auto pic = recorder.finishRecordingAsPicture();
        }
    }

private:
    using INHERITED = DecodeBench;
};

DEF_BENCH(return new SkottieDecodeBench("skottie_large",  // 426593
                                        "skottie/skottie-text-scale-to-fit-minmax.json"));
DEF_BENCH(return new SkottieDecodeBench("skottie_medium", //  10947
                                        "skottie/skottie-sphere-effect.json"));
DEF_BENCH(return new SkottieDecodeBench("skottie_small",  //   1112
                                        "skottie/skottie_sample_multiframe.json"));
// Created from PhoneHub assets SVG source, with https://lottiefiles.com/svg-to-lottie
DEF_BENCH(return new SkottieDecodeBench("skottie_phonehub_connecting.json",    // 216x216
                                        "skottie/skottie-phonehub-connecting.json"));
DEF_BENCH(return new SkottieDecodeBench("skottie_phonehub_generic_error.json", // 216x217
                                        "skottie/skottie-phonehub-generic-error.json"));
DEF_BENCH(return new SkottieDecodeBench("skottie_phonehub_onboard.json",       // 217x217
                                        "skottie/skottie-phonehub-onboard.json"));
// Created from PhoneHub assets SVG source, with https://jakearchibald.github.io/svgomg/ and then
// https://lottiefiles.com/svg-to-lottie
DEF_BENCH(return new SkottieDecodeBench("skottie_phonehub_svgo_connecting.json",
                                        "skottie/skottie-phonehub-svgo-connecting.json"));
DEF_BENCH(return new SkottieDecodeBench("skottie_phonehub_svgo_generic_error.json",
                                        "skottie/skottie-phonehub-svgo-generic-error.json"));
DEF_BENCH(return new SkottieDecodeBench("skottie_phonehub_svgo_onboard.json",
                                        "skottie/skottie-phonehub-svgo-onboard.json"));
// Created from PhoneHub assets SVG source:
// 1. Manually edited to have no masks (but look the same as the original)
// 2. https://jakearchibald.github.io/svgomg/
// 3. https://lottiefiles.com/svg-to-lottie
// Note: The Generic Error asset is excluded here because it has no masks in the first place.
DEF_BENCH(return new SkottieDecodeBench("skottie_phonehub_svgo_no_masks_connecting.json",
                                        "skottie/skottie-phonehub-svgo-no-masks-connecting.json"));
DEF_BENCH(return new SkottieDecodeBench("skottie_phonehub_svgo_no_masks_onboard.json",
                                        "skottie/skottie-phonehub-svgo-no-masks-onboard.json"));
// Created from PhoneHub assets SVG source:
// 1. Manually edited to use only the most basic functionality of SVG (but look the same as the
//    original)
// 2. https://jakearchibald.github.io/svgomg/
// 3. https://lottiefiles.com/svg-to-lottie
DEF_BENCH(return new SkottieDecodeBench("skottie_phonehub_svgo_no_frills_connecting.json",
                                        "skottie/skottie-phonehub-svgo-no-frills-connecting.json"));
DEF_BENCH(return new SkottieDecodeBench(
        "skottie_phonehub_svgo_no_frills_generic_error.json",
        "skottie/skottie-phonehub-svgo-no-frills-generic-error.json"));
DEF_BENCH(return new SkottieDecodeBench("skottie_phonehub_svgo_no_frills_onboard.json",
                                        "skottie/skottie-phonehub-svgo-no-frills-onboard.json"));
// All of the above PhoneHub benchmarks, with https://skia-review.googlesource.com/c/skia/+/141265
DEF_BENCH(return new SkottieDecodeBench("skottie_phonehub_connecting_min.json",
                                        "skottie/skottie-phonehub-connecting_min.json"));
DEF_BENCH(return new SkottieDecodeBench("skottie_phonehub_generic_error_min.json",
                                        "skottie/skottie-phonehub-generic-error_min.json"));
DEF_BENCH(return new SkottieDecodeBench("skottie_phonehub_onboard_min.json",
                                        "skottie/skottie-phonehub-onboard_min.json"));
DEF_BENCH(return new SkottieDecodeBench("skottie_phonehub_svgo_connecting_min.json",
                                        "skottie/skottie-phonehub-svgo-connecting_min.json"));
DEF_BENCH(return new SkottieDecodeBench("skottie_phonehub_svgo_generic_error_min.json",
                                        "skottie/skottie-phonehub-svgo-generic-error_min.json"));
DEF_BENCH(return new SkottieDecodeBench("skottie_phonehub_svgo_onboard_min.json",
                                        "skottie/skottie-phonehub-svgo-onboard_min.json"));
DEF_BENCH(return new SkottieDecodeBench(
        "skottie_phonehub_svgo_no_masks_connecting_min.json",
        "skottie/skottie-phonehub-svgo-no-masks-connecting_min.json"));
DEF_BENCH(return new SkottieDecodeBench("skottie_phonehub_svgo_no_masks_onboard_min.json",
                                        "skottie/skottie-phonehub-svgo-no-masks-onboard_min.json"));
DEF_BENCH(return new SkottieDecodeBench(
        "skottie_phonehub_svgo_no_frills_connecting_min.json",
        "skottie/skottie-phonehub-svgo-no-frills-connecting_min.json"));
DEF_BENCH(return new SkottieDecodeBench(
        "skottie_phonehub_svgo_no_frills_generic_error_min.json",
        "skottie/skottie-phonehub-svgo-no-frills-generic-error_min.json"));
DEF_BENCH(
        return new SkottieDecodeBench("skottie_phonehub_svgo_no_frills_onboard_min.json",
                                      "skottie/skottie-phonehub-svgo-no-frills-onboard_min.json"));

DEF_BENCH(return new SkottiePictureDecodeBench("skottiepic_large",
                                               "skottie/skottie-text-scale-to-fit-minmax.json"));
DEF_BENCH(return new SkottiePictureDecodeBench("skottiepic_medium",
                                               "skottie/skottie-sphere-effect.json"));
DEF_BENCH(return new SkottiePictureDecodeBench("skottiepic_small",
                                               "skottie/skottie_sample_multiframe.json"));
DEF_BENCH(return new SkottiePictureDecodeBench("skottiepic_phonehub_connecting.json",
                                               "skottie/skottie-phonehub-connecting.json"));
DEF_BENCH(return new SkottiePictureDecodeBench("skottiepic_phonehub_generic_error.json",
                                               "skottie/skottie-phonehub-generic-error.json"));
DEF_BENCH(return new SkottiePictureDecodeBench("skottiepic_phonehub_onboard.json",
                                               "skottie/skottie-phonehub-onboard.json"));
DEF_BENCH(return new SkottiePictureDecodeBench("skottiepic_phonehub_svgo_connecting.json",
                                               "skottie/skottie-phonehub-svgo-connecting.json"));
DEF_BENCH(return new SkottiePictureDecodeBench("skottiepic_phonehub_svgo_generic_error.json",
                                               "skottie/skottie-phonehub-svgo-generic-error.json"));
DEF_BENCH(return new SkottiePictureDecodeBench("skottiepic_phonehub_svgo_onboard.json",
                                               "skottie/skottie-phonehub-svgo-onboard.json"));
DEF_BENCH(return new SkottiePictureDecodeBench(
        "skottiepic_phonehub_svgo_no_masks_connecting.json",
        "skottie/skottie-phonehub-svgo-no-masks-connecting.json"));
DEF_BENCH(return new SkottiePictureDecodeBench(
        "skottiepic_phonehub_svgo_no_masks_onboard.json",
        "skottie/skottie-phonehub-svgo-no-masks-onboard.json"));
DEF_BENCH(return new SkottiePictureDecodeBench(
        "skottiepic_phonehub_svgo_no_frills_connecting.json",
        "skottie/skottie-phonehub-svgo-no-frills-connecting.json"));
DEF_BENCH(return new SkottiePictureDecodeBench(
        "skottiepic_phonehub_svgo_no_frills_generic_error.json",
        "skottie/skottie-phonehub-svgo-no-frills-generic-error.json"));
DEF_BENCH(return new SkottiePictureDecodeBench(
        "skottiepic_phonehub_svgo_no_frills_onboard.json",
        "skottie/skottie-phonehub-svgo-no-frills-onboard.json"));
DEF_BENCH(return new SkottiePictureDecodeBench("skottiepic_phonehub_connecting_min.json",
                                               "skottie/skottie-phonehub-connecting_min.json"));
DEF_BENCH(return new SkottiePictureDecodeBench("skottiepic_phonehub_generic_error_min.json",
                                               "skottie/skottie-phonehub-generic-error_min.json"));
DEF_BENCH(return new SkottiePictureDecodeBench("skottiepic_phonehub_onboard_min.json",
                                               "skottie/skottie-phonehub-onboard_min.json"));
DEF_BENCH(
        return new SkottiePictureDecodeBench("skottiepic_phonehub_svgo_connecting_min.json",
                                             "skottie/skottie-phonehub-svgo-connecting_min.json"));
DEF_BENCH(return new SkottiePictureDecodeBench(
        "skottiepic_phonehub_svgo_generic_error_min.json",
        "skottie/skottie-phonehub-svgo-generic-error_min.json"));
DEF_BENCH(return new SkottiePictureDecodeBench("skottiepic_phonehub_svgo_onboard_min.json",
                                               "skottie/skottie-phonehub-svgo-onboard_min.json"));
DEF_BENCH(return new SkottiePictureDecodeBench(
        "skottiepic_phonehub_svgo_no_masks_connecting_min.json",
        "skottie/skottie-phonehub-svgo-no-masks-connecting_min.json"));
DEF_BENCH(return new SkottiePictureDecodeBench(
        "skottiepic_phonehub_svgo_no_masks_onboard_min.json",
        "skottie/skottie-phonehub-svgo-no-masks-onboard_min.json"));
DEF_BENCH(return new SkottiePictureDecodeBench(
        "skottiepic_phonehub_svgo_no_frills_connecting_min.json",
        "skottie/skottie-phonehub-svgo-no-frills-connecting_min.json"));
DEF_BENCH(return new SkottiePictureDecodeBench(
        "skottiepic_phonehub_svgo_no_frills_generic_error_min.json",
        "skottie/skottie-phonehub-svgo-no-frills-generic-error_min.json"));
DEF_BENCH(return new SkottiePictureDecodeBench(
        "skottiepic_phonehub_svgo_no_frills_onboard_min.json",
        "skottie/skottie-phonehub-svgo-no-frills-onboard_min.json"));

DEF_BENCH(return new BitmapDecodeBench("png_large"    /*1600x1600*/, "images/mandrill_1600.png"));
DEF_BENCH(return new BitmapDecodeBench("png_medium"   /* 512x 512*/, "images/mandrill_512.png"));
DEF_BENCH(return new BitmapDecodeBench("png_small"    /*  32x  32*/, "images/mandrill_32.png"));
DEF_BENCH(return new BitmapDecodeBench("png_phonehub_connecting"   , "images/Connecting.png"));
DEF_BENCH(return new BitmapDecodeBench("png_phonehub_generic_error", "images/Generic_Error.png"));
DEF_BENCH(return new BitmapDecodeBench("png_phonehub_onboard"      , "images/Onboard.png"));
