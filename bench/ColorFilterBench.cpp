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
#include "include/effects/SkImageFilters.h"
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

    typedef Benchmark INHERITED;
};

DEF_BENCH( return new FilteredRectBench(FilteredRectBench::kNoFilter_Type); )
DEF_BENCH( return new FilteredRectBench(FilteredRectBench::kColorFilter_Type); )
DEF_BENCH( return new FilteredRectBench(FilteredRectBench::kImageFilter_Type); )

namespace  {

class ColorMatrixBench final : public Benchmark {
public:
    using Factory = sk_sp<SkColorFilter>(*)();

    explicit ColorMatrixBench(const char* suffix, Factory f)
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
            canvas->drawImage(fImage, 0, 0, &p);
        }
    }

    const Factory  fFactory;
    const SkString fName;

    sk_sp<SkImage>       fImage;
    sk_sp<SkColorFilter> fColorFilter;
};

void RuntimeNone_CPU(float[4], const void*) {}

const char RuntimeNone_GPU_SRC[] = R"(
    void main(inout half4 c) {}
)";

void RuntimeColorMatrix_CPU(float colors[4], const void* ctx) {
    auto c = reinterpret_cast<SkRGBA4f<kPremul_SkAlphaType>*>(colors)->unpremul();

    const auto* m = static_cast<const float*>(ctx);
    const auto c0 = c[0]*m[ 0] + c[1]*m[ 1] + c[2]*m[ 2] + c[3]*m[ 3] + m[ 4],
               c1 = c[0]*m[ 5] + c[1]*m[ 6] + c[2]*m[ 7] + c[3]*m[ 8] + m[ 9],
               c2 = c[0]*m[10] + c[1]*m[11] + c[2]*m[12] + c[3]*m[13] + m[14],
               c3 = c[0]*m[15] + c[1]*m[16] + c[2]*m[17] + c[3]*m[18] + m[19];
    c[0] = c0;
    c[1] = c1;
    c[2] = c2;
    c[3] = c3;

    *reinterpret_cast<SkRGBA4f<kPremul_SkAlphaType>*>(colors) = c.premul();
}

const char RuntimeColorMatrix_GPU_SRC[] = R"(
    // WTB matrix/vector inputs.
    layout(ctype=float) in uniform half m0 , m1 , m2 , m3 , m4 ,
                                        m5 , m6 , m7 , m8 , m9 ,
                                        m10, m11, m12, m13, m14,
                                        m15, m16, m17, m18, m19;
    void main(inout half4 c) {
        half nonZeroAlpha = max(c.a, 0.0001);
        c = half4(c.rgb / nonZeroAlpha, nonZeroAlpha);

        half4x4 m = half4x4(m0, m5, m10, m15,
                            m1, m6, m11, m16,
                            m2, m7, m12, m17,
                            m3, m8, m13, m18);
        c = m * c + half4  (m4, m9, m14, m19);

        c = saturate(c);
        c.rgb *= c.a;
    }
)";

static constexpr float gColorMatrix[] = {
    0.3f, 0.3f, 0.0f, 0.0f, 0.3f,
    0.0f, 0.3f, 0.3f, 0.0f, 0.3f,
    0.0f, 0.0f, 0.3f, 0.3f, 0.3f,
    0.3f, 0.0f, 0.3f, 0.3f, 0.0f,
};

} // namespace

DEF_BENCH( return new ColorMatrixBench("none",
    []() { return sk_sp<SkColorFilter>(nullptr); }); )
DEF_BENCH( return new ColorMatrixBench("blend_src",
    []() { return SkColorFilters::Blend(0x80808080, SkBlendMode::kSrc); }); )
DEF_BENCH( return new ColorMatrixBench("blend_srcover",
    []() { return SkColorFilters::Blend(0x80808080, SkBlendMode::kSrcOver); }); )
DEF_BENCH( return new ColorMatrixBench("linear_to_srgb",
    []() { return SkColorFilters::LinearToSRGBGamma(); }); )
DEF_BENCH( return new ColorMatrixBench("srgb_to_linear",
    []() { return SkColorFilters::SRGBToLinearGamma(); }); )
DEF_BENCH( return new ColorMatrixBench("matrix_rgba",
    []() { return SkColorFilters::Matrix(gColorMatrix); }); )
DEF_BENCH( return new ColorMatrixBench("matrix_hsla",
    []() { return SkColorFilters::HSLAMatrix(gColorMatrix); }); )
DEF_BENCH( return new ColorMatrixBench("compose_src",
    []() { return SkColorFilters::Compose(SkColorFilters::Blend(0x80808080, SkBlendMode::kSrc),
                                          SkColorFilters::Blend(0x80808080, SkBlendMode::kSrc));
    }); )
DEF_BENCH( return new ColorMatrixBench("lerp_src",
    []() { return SkColorFilters::Lerp(0.3f,
                                       SkColorFilters::Blend(0x80808080, SkBlendMode::kSrc),
                                       SkColorFilters::Blend(0x80808080, SkBlendMode::kSrc));
    }); )

#ifdef SK_SUPPORT_GPU
DEF_BENCH( return new ColorMatrixBench("src_runtime", []() {
        static SkRuntimeColorFilterFactory gRuntimeFact(SkString(RuntimeNone_GPU_SRC),
                                                        RuntimeNone_CPU);
        return gRuntimeFact.make(SkData::MakeWithCopy(gColorMatrix, sizeof(gColorMatrix)));
    });)
DEF_BENCH( return new ColorMatrixBench("matrix_runtime", []() {
        static SkRuntimeColorFilterFactory gRuntimeFact(SkString(RuntimeColorMatrix_GPU_SRC),
                                                        RuntimeColorMatrix_CPU);
        return gRuntimeFact.make(SkData::MakeWithCopy(gColorMatrix, sizeof(gColorMatrix)));
    });)
#endif
