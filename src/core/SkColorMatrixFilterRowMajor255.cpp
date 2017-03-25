/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorMatrixFilterRowMajor255.h"
#include "SkColorPriv.h"
#include "SkNx.h"
#include "SkPM4fPriv.h"
#include "SkRasterPipeline.h"
#include "SkReadBuffer.h"
#include "SkRefCnt.h"
#include "SkString.h"
#include "SkUnPreMultiply.h"
#include "SkWriteBuffer.h"

/*
 *  RowMajor
 *  0  1  2  3  4
 *  5  6  7  8  9
 * 10 11 12 13 14
 * 15 16 17 18 19
 *
 *  ColMajor
 *  0  4  8 12 16
 *  1  5  9 13 17
 *  2  6 10 14 18
 *  3  7 11 15 19
 */

static void transpose_col_to_row_scale255(float dst[20], const float src[20]) {
    float* dst0 = dst + 0;
    float* dst1 = dst + 5;
    float* dst2 = dst + 10;
    float* dst3 = dst + 15;

    for (int i = 0; i < 4; ++i) {
        *dst0++ = *src++;
        *dst1++ = *src++;
        *dst2++ = *src++;
        *dst3++ = *src++;
    }
    *dst0 = *src++ * 255;
    *dst1 = *src++ * 255;
    *dst2 = *src++ * 255;
    *dst3 = *src++ * 255;
}

bool SkColorFilter::asColorMatrixRowMajor255(float rowMajor255[20]) const {
    float colMajor[20];
    if (this->onAsColorMatrix(colMajor)) {
        if (rowMajor255) {
            transpose_col_to_row_scale255(rowMajor255, colMajor);
        }
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// needed for legacy row-major-255 [src] matrices. [dst] is col-major 0..1
//
static void transpose_and_scale01(float dst[20], const float src[20]) {
    const float* srcR = src + 0;
    const float* srcG = src + 5;
    const float* srcB = src + 10;
    const float* srcA = src + 15;

    for (int i = 0; i < 16; i += 4) {
        dst[i + 0] = *srcR++;
        dst[i + 1] = *srcG++;
        dst[i + 2] = *srcB++;
        dst[i + 3] = *srcA++;
    }
    // Might as well scale these translates down to [0,1] here instead of every filter call.
    dst[16] = *srcR * (1/255.0f);
    dst[17] = *srcG * (1/255.0f);
    dst[18] = *srcB * (1/255.0f);
    dst[19] = *srcA * (1/255.0f);
}

void SkColorMatrixFilter::initState() {
    const float* array = fColMajor;

    // check if we have to munge Alpha
    bool changesAlpha = (array[3] || array[7] || array[11] || (array[15] - 1) || array[19]);
    bool usesAlpha = (array[12] || array[13] || array[14]);

    if (changesAlpha || usesAlpha) {
        fFlags = changesAlpha ? 0 : kAlphaUnchanged_Flag;
    } else {
        fFlags = kAlphaUnchanged_Flag;
    }
}

///////////////////////////////////////////////////////////////////////////////

SkColorMatrixFilter::SkColorMatrixFilter(const float array[20]) {
    memcpy(fColMajor, array, 20 * sizeof(float));
    this->initState();
}

uint32_t SkColorMatrixFilter::getFlags() const {
    return this->INHERITED::getFlags() | fFlags;
}

static Sk4f scale_rgb(float scale) {
    static_assert(SkPM4f::A == 3, "Alpha is lane 3");
    return Sk4f(scale, scale, scale, 1);
}

static Sk4f premul(const Sk4f& x) {
    return x * scale_rgb(x[SkPM4f::A]);
}

static Sk4f unpremul(const Sk4f& x) {
    return x * scale_rgb(1 / x[SkPM4f::A]);  // TODO: fast/approx invert?
}

static Sk4f clamp_0_1(const Sk4f& x) {
    return Sk4f::Max(Sk4f::Min(x, Sk4f(1)), Sk4f(0));
}

static SkPMColor round(const Sk4f& x) {
    SkPMColor c;
    SkNx_cast<uint8_t>(x * Sk4f(255) + Sk4f(0.5f)).store(&c);
    return c;
}

template <typename Adaptor, typename T>
void filter_span(const float array[], const T src[], int count, T dst[]) {
    const Sk4f c0 = Sk4f::Load(array + 0);
    const Sk4f c1 = Sk4f::Load(array + 4);
    const Sk4f c2 = Sk4f::Load(array + 8);
    const Sk4f c3 = Sk4f::Load(array + 12);
    const Sk4f c4 = Sk4f::Load(array + 16);

    // todo: we could cache this in the constructor...
    T matrix_translate_pmcolor = Adaptor::From4f(premul(clamp_0_1(c4)));

    for (int i = 0; i < count; i++) {
        Sk4f srcf = Adaptor::To4f(src[i]);
        float srcA = srcf[SkPM4f::A];

        if (0 == srcA) {
            dst[i] = matrix_translate_pmcolor;
            continue;
        }
        if (1 != srcA) {
            srcf = unpremul(srcf);
        }

        Sk4f r4 = srcf[Adaptor::R];
        Sk4f g4 = srcf[Adaptor::G];
        Sk4f b4 = srcf[Adaptor::B];
        Sk4f a4 = srcf[Adaptor::A];
        // apply matrix
        Sk4f dst4 = c0 * r4 + c1 * g4 + c2 * b4 + c3 * a4 + c4;

        dst[i] = Adaptor::From4f(premul(clamp_0_1(dst4)));
    }
}

struct SkPMColorAdaptor {
    enum {
        R = SK_R_INDEX,
        G = SK_G_INDEX,
        B = SK_B_INDEX,
        A = SK_A_INDEX,
    };
    static SkPMColor From4f(const Sk4f& c4) {
        return round(swizzle_rb_if_bgra(c4));
    }
    static Sk4f To4f(SkPMColor c) {
        return Sk4f_fromL32(c);
    }
};
void SkColorMatrixFilter::filterSpan(const SkPMColor src[], int count, SkPMColor dst[]) const {
    filter_span<SkPMColorAdaptor>(fColMajor, src, count, dst);
}

struct SkPM4fAdaptor {
    enum {
        R = SkPM4f::R,
        G = SkPM4f::G,
        B = SkPM4f::B,
        A = SkPM4f::A,
    };
    static SkPM4f From4f(const Sk4f& c4) {
        return SkPM4f::From4f(c4);
    }
    static Sk4f To4f(const SkPM4f& c) {
        return c.to4f();
    }
};
void SkColorMatrixFilter::filterSpan4f(const SkPM4f src[], int count, SkPM4f dst[]) const {
    filter_span<SkPM4fAdaptor>(fColMajor, src, count, dst);
}

///////////////////////////////////////////////////////////////////////////////

void SkColorMatrixFilter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeScalarArray(fColMajor, 20);
}

sk_sp<SkFlattenable> SkColorMatrixFilter::CreateProc(SkReadBuffer& buffer) {
    float colMajor[20];
    if (buffer.readScalarArray(colMajor, 20)) {
        return sk_make_sp<SkColorMatrixFilter>(colMajor);
    }
    return nullptr;
}

bool SkColorMatrixFilter::onAsColorMatrix(float colMajor[20]) const {
    if (colMajor) {
        memcpy(colMajor, fColMajor, 20 * sizeof(float));
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
//  This code was duplicated from src/effects/SkColorMatrixc.cpp in order to be used in core.
//////

// To detect if we need to apply clamping after applying a matrix, we check if
// any output component might go outside of [0, 255] for any combination of
// input components in [0..255].
// Each output component is an affine transformation of the input component, so
// the minimum and maximum values are for any combination of minimum or maximum
// values of input components (i.e. 0 or 255).
// E.g. if R' = x*R + y*G + z*B + w*A + t
// Then the maximum value will be for R=255 if x>0 or R=0 if x<0, and the
// minimum value will be for R=0 if x>0 or R=255 if x<0.
// Same goes for all components.
static bool component_needs_clamping(const float row[5]) {
    float maxValue = row[4] / 255;
    float minValue = row[4] / 255;
    for (int i = 0; i < 4; ++i) {
        if (row[i] > 0)
            maxValue += row[i];
        else
            minValue += row[i];
    }
    return (maxValue > 1) || (minValue < 0);
}

// TODO: rewrite this to be colMajor
static bool needs_clamping(const float matrix[20]) {
    return component_needs_clamping(matrix)
        || component_needs_clamping(matrix+5)
        || component_needs_clamping(matrix+10)
        || component_needs_clamping(matrix+15);
}

// TODO: rewrite this to be colMajor
static void set_concat(float result[20], const float outer[20], const float inner[20]) {
    int index = 0;
    for (int j = 0; j < 20; j += 5) {
        for (int i = 0; i < 4; i++) {
            result[index++] =   outer[j + 0] * inner[i + 0] +
                                outer[j + 1] * inner[i + 5] +
                                outer[j + 2] * inner[i + 10] +
                                outer[j + 3] * inner[i + 15];
        }
        result[index++] =   outer[j + 0] * inner[4] +
                            outer[j + 1] * inner[9] +
                            outer[j + 2] * inner[14] +
                            outer[j + 3] * inner[19] +
                            outer[j + 4];
    }
}

///////////////////////////////////////////////////////////////////////////////
//  End duplication
//////

bool SkColorMatrixFilter::onAppendStages(SkRasterPipeline* p, SkColorSpace* dst,
                                         SkArenaAlloc* scratch, bool shaderIsOpaque) const {
    bool willStayOpaque = shaderIsOpaque && (fFlags & kAlphaUnchanged_Flag);
    bool needsClamp0 = false,
         needsClamp1 = false;
    for (int i = 0; i < 4; i++) {
        float min = fColMajor[i+16],
              max = fColMajor[i+16];
        (fColMajor[i+ 0] < 0 ? min : max) += fColMajor[i+ 0];
        (fColMajor[i+ 4] < 0 ? min : max) += fColMajor[i+ 4];
        (fColMajor[i+ 8] < 0 ? min : max) += fColMajor[i+ 8];
        (fColMajor[i+12] < 0 ? min : max) += fColMajor[i+12];
        needsClamp0 = needsClamp0 || min < 0;
        needsClamp1 = needsClamp1 || max > 1;
    }

    if (!shaderIsOpaque) { p->append(SkRasterPipeline::unpremul); }
    if (           true) { p->append(SkRasterPipeline::matrix_4x5, fColMajor); }
    if (!willStayOpaque) { p->append(SkRasterPipeline::premul); }
    if (    needsClamp0) { p->append(SkRasterPipeline::clamp_0); }
    if (    needsClamp1) { p->append(SkRasterPipeline::clamp_a); }
    return true;
}

sk_sp<SkColorFilter> SkColorMatrixFilter::makeComposed(sk_sp<SkColorFilter> innerFilter) const {
    float innerMatrix[20];
    if (innerFilter->asColorMatrixColMajor(innerMatrix) && !needs_clamping(innerMatrix)) {
        float concat[20];
        set_concat(concat, fColMajor, innerMatrix);
        return sk_make_sp<SkColorMatrixFilter>(concat);
    }
    return nullptr;
}

#if SK_SUPPORT_GPU
#include "GrFragmentProcessor.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"

class ColorMatrixEffect : public GrFragmentProcessor {
public:
    static sk_sp<GrFragmentProcessor> Make(const float matrix[20]) {
        return sk_sp<GrFragmentProcessor>(new ColorMatrixEffect(matrix));
    }

    const char* name() const override { return "Color Matrix"; }

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    class GLSLProcessor : public GrGLSLFragmentProcessor {
    public:
        // this class always generates the same code.
        static void GenKey(const GrProcessor&, const GrShaderCaps&, GrProcessorKeyBuilder*) {}

        void emitCode(EmitArgs& args) override {
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
            fMatrixHandle = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                       kMat44f_GrSLType, kDefault_GrSLPrecision,
                                                       "ColorMatrix");
            fVectorHandle = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                       kVec4f_GrSLType, kDefault_GrSLPrecision,
                                                       "ColorMatrixVector");

            if (nullptr == args.fInputColor) {
                // could optimize this case, but we aren't for now.
                args.fInputColor = "vec4(1)";
            }
            GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
            // The max() is to guard against 0 / 0 during unpremul when the incoming color is
            // transparent black.
            fragBuilder->codeAppendf("\tfloat nonZeroAlpha = max(%s.a, 0.00001);\n",
                                     args.fInputColor);
            fragBuilder->codeAppendf("\t%s = %s * vec4(%s.rgb / nonZeroAlpha, nonZeroAlpha) + %s;\n",
                                     args.fOutputColor,
                                     uniformHandler->getUniformCStr(fMatrixHandle),
                                     args.fInputColor,
                                     uniformHandler->getUniformCStr(fVectorHandle));
            fragBuilder->codeAppendf("\t%s = clamp(%s, 0.0, 1.0);\n",
                                     args.fOutputColor, args.fOutputColor);
            fragBuilder->codeAppendf("\t%s.rgb *= %s.a;\n", args.fOutputColor, args.fOutputColor);
        }

    protected:
        void onSetData(const GrGLSLProgramDataManager& uniManager,
                       const GrProcessor& proc) override {
            const ColorMatrixEffect& cme = proc.cast<ColorMatrixEffect>();
            const float* m = cme.fMatrix;
            // The GL matrix is transposed from SkColorMatrix.
            float mt[]  = {
                m[0], m[5], m[10], m[15],
                m[1], m[6], m[11], m[16],
                m[2], m[7], m[12], m[17],
                m[3], m[8], m[13], m[18],
            };
            static const float kScale = 1.0f / 255.0f;
            float vec[] = {
                m[4] * kScale, m[9] * kScale, m[14] * kScale, m[19] * kScale,
            };
            uniManager.setMatrix4fv(fMatrixHandle, 1, mt);
            uniManager.set4fv(fVectorHandle, 1, vec);
        }

    private:
        GrGLSLProgramDataManager::UniformHandle fMatrixHandle;
        GrGLSLProgramDataManager::UniformHandle fVectorHandle;

        typedef GrGLSLFragmentProcessor INHERITED;
    };
private:
    // We could implement the constant input->constant output optimization but haven't. Other
    // optimizations would be matrix-dependent.
    ColorMatrixEffect(const float matrix[20]) : INHERITED(kNone_OptimizationFlags) {
        memcpy(fMatrix, matrix, sizeof(float) * 20);
        this->initClassID<ColorMatrixEffect>();
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        return new GLSLProcessor;
    }

    virtual void onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                       GrProcessorKeyBuilder* b) const override {
        GLSLProcessor::GenKey(*this, caps, b);
    }

    bool onIsEqual(const GrFragmentProcessor& s) const override {
        const ColorMatrixEffect& cme = s.cast<ColorMatrixEffect>();
        return 0 == memcmp(fMatrix, cme.fMatrix, sizeof(fMatrix));
    }

    float fMatrix[20];

    typedef GrFragmentProcessor INHERITED;
};

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(ColorMatrixEffect);

#if GR_TEST_UTILS
sk_sp<GrFragmentProcessor> ColorMatrixEffect::TestCreate(GrProcessorTestData* d) {
    float colorMatrix[20];
    for (size_t i = 0; i < SK_ARRAY_COUNT(colorMatrix); ++i) {
        colorMatrix[i] = d->fRandom->nextSScalar1();
    }
    return ColorMatrixEffect::Make(colorMatrix);
}
#endif

sk_sp<GrFragmentProcessor> SkColorMatrixFilter::asFragmentProcessor(
                                                                  GrContext*, SkColorSpace*) const {
    return ColorMatrixEffect::Make(fColMajor);
}

#endif

#ifndef SK_IGNORE_TO_STRING
void SkColorMatrixFilter::toString(SkString* str) const {
    str->append("SkColorMatrixFilter: ");

    str->append("matrix: (");
    for (int i = 0; i < 20; ++i) {
        str->appendScalar(fColMajor[i]);
        if (i < 19) {
            str->append(", ");
        }
    }
    str->append(")");
}
#endif

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkColorFilter> SkColorFilter::MakeMatrixColMajor(const float colMajor[20]) {
    return sk_sp<SkColorFilter>(new SkColorMatrixFilter(colMajor));
}

sk_sp<SkColorFilter> SkColorFilter::MakeMatrixFilterRowMajor255(const float rowMajor[20]) {
    float colMajor[20];
    transpose_and_scale01(colMajor, rowMajor);
    return sk_sp<SkColorFilter>(new SkColorMatrixFilter(colMajor));
}
