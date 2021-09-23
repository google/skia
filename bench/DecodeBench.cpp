/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkPictureRecorder.h"
#include "modules/skottie/include/Skottie.h"
#include "tools/Resources.h"

class DecodeBench : public Benchmark {
protected:
    DecodeBench(const char* name, const char* source)
        : fName(SkStringPrintf("decode_%s", name))
        , fSource(source)
    {}

    bool isSuitableFor(Backend backend) final {
            return backend == kNonRendering_Backend;
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
            SkAssertResult(DecodeDataToBitmap(fData, &bm));
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
            const auto anim = skottie::Animation::Make(reinterpret_cast<const char*>(fData->data()),
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
            const auto anim = skottie::Animation::Make(reinterpret_cast<const char*>(fData->data()),
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

DEF_BENCH(return new SkottiePictureDecodeBench("skottiepic_large",
                                               "skottie/skottie-text-scale-to-fit-minmax.json"));
DEF_BENCH(return new SkottiePictureDecodeBench("skottiepic_medium",
                                               "skottie/skottie-sphere-effect.json"));
DEF_BENCH(return new SkottiePictureDecodeBench("skottiepic_small",
                                               "skottie/skottie_sample_multiframe.json"));

DEF_BENCH(return new BitmapDecodeBench("png_large" , "images/mandrill_1600.png"));// 1600x1600
DEF_BENCH(return new BitmapDecodeBench("png_medium", "images/mandrill_512.png")); //  512x 512
DEF_BENCH(return new BitmapDecodeBench("png_small" , "images/mandrill_32.png"));  //   32x  32
