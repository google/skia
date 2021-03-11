/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkHighContrastFilter.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkOverdrawColorFilter.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkColorFilterPriv.h"
#include "tools/Resources.h"

// Just need an interesting filter, nothing to special about colormatrix
static sk_sp<SkColorFilter> make_grayscale() {
    float matrix[20];
    memset(matrix, 0, 20 * sizeof(float));
    matrix[0] = matrix[5] = matrix[10] = 0.2126f;
    matrix[1] = matrix[6] = matrix[11] = 0.7152f;
    matrix[2] = matrix[7] = matrix[12] = 0.0722f;
    matrix[18] = 1.0f;
    return SkColorFilters::Matrix(matrix);
}

/**
 *  Different ways to draw the same thing (a red rect)
 *  All of their timings should be about the same
 *  (we allow for slight overhead to figure out that we can undo the presence of the filters)
 */
class FilteredRectBench : public Benchmark {
public:
    enum Type {
        kNoFilter_Type,
        kColorFilter_Type,
        kImageFilter_Type,
    };

    FilteredRectBench(Type t) : fType(t) {
        static const char* suffix[] = { "nofilter", "colorfilter", "imagefilter" };
        fName.printf("filteredrect_%s", suffix[t]);
        fPaint.setColor(SK_ColorRED);
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDelayedSetup() override {
        switch (fType) {
            case kNoFilter_Type:
                break;
            case kColorFilter_Type:
                fPaint.setColorFilter(make_grayscale());
                break;
            case kImageFilter_Type:
                fPaint.setImageFilter(SkImageFilters::ColorFilter(make_grayscale(), nullptr));
            break;
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        const SkRect r = { 0, 0, 256, 256 };
        for (int i = 0; i < loops; ++i) {
            canvas->drawRect(r, fPaint);
        }
    }

private:
    SkPaint  fPaint;
    SkString fName;
    Type     fType;

    using INHERITED = Benchmark;
};

DEF_BENCH( return new FilteredRectBench(FilteredRectBench::kNoFilter_Type); )
DEF_BENCH( return new FilteredRectBench(FilteredRectBench::kColorFilter_Type); )
DEF_BENCH( return new FilteredRectBench(FilteredRectBench::kImageFilter_Type); )

namespace  {

class ColorFilterBench final : public Benchmark {
public:
    using Factory = sk_sp<SkColorFilter>(*)();

    explicit ColorFilterBench(const char* suffix, Factory f)
        : fFactory(f)
        , fName(SkStringPrintf("colorfilter_%s", suffix)) {}

private:
    const char* onGetName() override {
        return fName.c_str();
    }

    SkIPoint onGetSize() override {
        return { 256, 256 };
    }

    void onDelayedSetup() override {
        // Pass the image though a premul canvas so that we "forget" it is opaque.
        auto surface = SkSurface::MakeRasterN32Premul(256, 256);
        surface->getCanvas()->drawImage(GetResourceAsImage("images/mandrill_256.png"), 0, 0);

        fImage = surface->makeImageSnapshot();
        fColorFilter = fFactory();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint p;
        p.setColorFilter(fColorFilter);

        for (int i = 0; i < loops; ++i) {
            canvas->drawImage(fImage, 0, 0, SkSamplingOptions(), &p);
        }
    }

    const Factory  fFactory;
    const SkString fName;

    sk_sp<SkImage>       fImage;
    sk_sp<SkColorFilter> fColorFilter;
};

const char RuntimeNone_GPU_SRC[] = R"(
    uniform shader input;
    half4 main() { return sample(input); }
)";

const char RuntimeColorMatrix_GPU_SRC[] = R"(
    // WTB matrix/vector inputs.
    uniform half m0 , m1 , m2 , m3 , m4 ,
                 m5 , m6 , m7 , m8 , m9 ,
                 m10, m11, m12, m13, m14,
                 m15, m16, m17, m18, m19;
    uniform shader input;
    half4 main() {
        half4 c = unpremul(sample(input));

        half4x4 m = half4x4(m0, m5, m10, m15,
                            m1, m6, m11, m16,
                            m2, m7, m12, m17,
                            m3, m8, m13, m18);
        c = m * c + half4  (m4, m9, m14, m19);

        c = saturate(c);
        c.rgb *= c.a;
        return c;
    }
)";

static constexpr float gColorMatrix[] = {
    0.3f, 0.3f, 0.0f, 0.0f, 0.3f,
    0.0f, 0.3f, 0.3f, 0.0f, 0.3f,
    0.0f, 0.0f, 0.3f, 0.3f, 0.3f,
    0.3f, 0.0f, 0.3f, 0.3f, 0.0f,
};

} // namespace

DEF_BENCH( return new ColorFilterBench("none",
    []() { return sk_sp<SkColorFilter>(nullptr); }); )
DEF_BENCH( return new ColorFilterBench("blend_src",
    []() { return SkColorFilters::Blend(0x80808080, SkBlendMode::kSrc); }); )
DEF_BENCH( return new ColorFilterBench("blend_srcover",
    []() { return SkColorFilters::Blend(0x80808080, SkBlendMode::kSrcOver); }); )
DEF_BENCH( return new ColorFilterBench("linear_to_srgb",
    []() { return SkColorFilters::LinearToSRGBGamma(); }); )
DEF_BENCH( return new ColorFilterBench("srgb_to_linear",
    []() { return SkColorFilters::SRGBToLinearGamma(); }); )
DEF_BENCH( return new ColorFilterBench("matrix_rgba",
    []() { return SkColorFilters::Matrix(gColorMatrix); }); )
DEF_BENCH( return new ColorFilterBench("matrix_hsla",
    []() { return SkColorFilters::HSLAMatrix(gColorMatrix); }); )
DEF_BENCH( return new ColorFilterBench("compose_src",
    []() { return SkColorFilters::Compose(SkColorFilters::Blend(0x80808080, SkBlendMode::kSrc),
                                          SkColorFilters::Blend(0x80808080, SkBlendMode::kSrc));
    }); )
DEF_BENCH( return new ColorFilterBench("lerp_src",
    []() { return SkColorFilters::Lerp(0.3f,
                                       SkColorFilters::Blend(0x80808080, SkBlendMode::kSrc),
                                       SkColorFilters::Blend(0x80808080, SkBlendMode::kSrc));
    }); )

DEF_BENCH( return new ColorFilterBench("highcontrast", []() {
    return SkHighContrastFilter::Make({
        false, SkHighContrastConfig::InvertStyle::kInvertLightness, 0.2f
    });
}); )
DEF_BENCH( return new ColorFilterBench("overdraw", []() {
    const SkColor colors[SkOverdrawColorFilter::kNumColors] = {
            0x80FF0000, 0x8000FF00, 0x800000FF, 0x80FFFF00, 0x8000FFFF, 0x80FF00FF,
    };
    return SkOverdrawColorFilter::MakeWithSkColors(colors);
}); )
DEF_BENCH( return new ColorFilterBench("gaussian", []() {
    return SkColorFilterPriv::MakeGaussian();
}); )

#if SK_SUPPORT_GPU
DEF_BENCH( return new ColorFilterBench("src_runtime", []() {
        static sk_sp<SkRuntimeEffect> gEffect =
                SkRuntimeEffect::Make(SkString(RuntimeNone_GPU_SRC)).effect;
        sk_sp<SkColorFilter> input = nullptr;
        return gEffect->makeColorFilter(SkData::MakeEmpty(), &input, 1);
    });)
DEF_BENCH( return new ColorFilterBench("matrix_runtime", []() {
        static sk_sp<SkRuntimeEffect> gEffect =
                SkRuntimeEffect::Make(SkString(RuntimeColorMatrix_GPU_SRC)).effect;
        sk_sp<SkColorFilter> input = nullptr;
        return gEffect->makeColorFilter(SkData::MakeWithCopy(gColorMatrix, sizeof(gColorMatrix)),
                                        &input, 1);
    });)
#endif

class FilterColorBench final : public Benchmark {
public:
    explicit FilterColorBench() {}

    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

private:
    const char* onGetName() override { return "matrix_filterColor4f"; }

    void onDelayedSetup() override {
        SkScalar colorMatrix[20] = {
            0.9f, 0.9f, 0.9f, 0.9f, 0.9f,
            0.9f, 0.9f, 0.9f, 0.9f, 0.9f,
            0.9f, 0.9f, 0.9f, 0.9f, 0.9f,
            0.9f, 0.9f, 0.9f, 0.9f, 0.9f
        };
        fColorFilter = SkColorFilters::Matrix(colorMatrix);
    }

    void onDraw(int loops, SkCanvas*) override {
        SkColor4f c = { 1.f, 1.f, 0.f, 1.0f };

        for (int i = 0; i < loops; ++i) {
            c = fColorFilter->filterColor4f(c, /*srcCS=*/nullptr, /*dstCS=*/nullptr);
        }
    }

    sk_sp<SkColorFilter> fColorFilter;
};

DEF_BENCH( return new FilterColorBench(); )
