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
#include "SkReadBuffer.h"
#include "SkRefCnt.h"
#include "SkString.h"
#include "SkUnPreMultiply.h"
#include "SkWriteBuffer.h"

static void transpose(float dst[20], const float src[20]) {
    const float* srcR = src + 0;
    const float* srcG = src + 5;
    const float* srcB = src + 10;
    const float* srcA = src + 15;

    for (int i = 0; i < 20; i += 4) {
        dst[i + 0] = *srcR++;
        dst[i + 1] = *srcG++;
        dst[i + 2] = *srcB++;
        dst[i + 3] = *srcA++;
    }
}

void SkColorMatrixFilterRowMajor255::initState() {
    transpose(fTranspose, fMatrix);

    const float* array = fMatrix;

    // check if we have to munge Alpha
    bool changesAlpha = (array[15] || array[16] || array[17] || (array[18] - 1) || array[19]);
    bool usesAlpha = (array[3] || array[8] || array[13]);

    if (changesAlpha || usesAlpha) {
        fFlags = changesAlpha ? 0 : kAlphaUnchanged_Flag;
    } else {
        fFlags = kAlphaUnchanged_Flag;
    }
}

///////////////////////////////////////////////////////////////////////////////

SkColorMatrixFilterRowMajor255::SkColorMatrixFilterRowMajor255(const SkScalar array[20]) {
    memcpy(fMatrix, array, 20 * sizeof(SkScalar));
    this->initState();
}

uint32_t SkColorMatrixFilterRowMajor255::getFlags() const {
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
    // c0-c3 are already in [0,1].
    const Sk4f c0 = Sk4f::Load(array + 0);
    const Sk4f c1 = Sk4f::Load(array + 4);
    const Sk4f c2 = Sk4f::Load(array + 8);
    const Sk4f c3 = Sk4f::Load(array + 12);
    // c4 (the translate vector) is in [0, 255].  Bring it back to [0,1].
    const Sk4f c4 = Sk4f::Load(array + 16)*Sk4f(1.0f/255);

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
        return to_4f(c) * Sk4f(1.0f/255);
    }
};
void SkColorMatrixFilterRowMajor255::filterSpan(const SkPMColor src[], int count, SkPMColor dst[]) const {
    filter_span<SkPMColorAdaptor>(fTranspose, src, count, dst);
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
void SkColorMatrixFilterRowMajor255::filterSpan4f(const SkPM4f src[], int count, SkPM4f dst[]) const {
    filter_span<SkPM4fAdaptor>(fTranspose, src, count, dst);
}

///////////////////////////////////////////////////////////////////////////////

void SkColorMatrixFilterRowMajor255::flatten(SkWriteBuffer& buffer) const {
    SkASSERT(sizeof(fMatrix)/sizeof(SkScalar) == 20);
    buffer.writeScalarArray(fMatrix, 20);
}

sk_sp<SkFlattenable> SkColorMatrixFilterRowMajor255::CreateProc(SkReadBuffer& buffer) {
    SkScalar matrix[20];
    if (buffer.readScalarArray(matrix, 20)) {
        return sk_make_sp<SkColorMatrixFilterRowMajor255>(matrix);
    }
    return nullptr;
}

bool SkColorMatrixFilterRowMajor255::asColorMatrix(SkScalar matrix[20]) const {
    if (matrix) {
        memcpy(matrix, fMatrix, 20 * sizeof(SkScalar));
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
static bool component_needs_clamping(const SkScalar row[5]) {
    SkScalar maxValue = row[4] / 255;
    SkScalar minValue = row[4] / 255;
    for (int i = 0; i < 4; ++i) {
        if (row[i] > 0)
            maxValue += row[i];
        else
            minValue += row[i];
    }
    return (maxValue > 1) || (minValue < 0);
}

static bool needs_clamping(const SkScalar matrix[20]) {
    return component_needs_clamping(matrix)
        || component_needs_clamping(matrix+5)
        || component_needs_clamping(matrix+10)
        || component_needs_clamping(matrix+15);
}

static void set_concat(SkScalar result[20], const SkScalar outer[20], const SkScalar inner[20]) {
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

sk_sp<SkColorFilter>
SkColorMatrixFilterRowMajor255::makeComposed(sk_sp<SkColorFilter> innerFilter) const {
    SkScalar innerMatrix[20];
    if (innerFilter->asColorMatrix(innerMatrix) && !needs_clamping(innerMatrix)) {
        SkScalar concat[20];
        set_concat(concat, fMatrix, innerMatrix);
        return sk_make_sp<SkColorMatrixFilterRowMajor255>(concat);
    }
    return nullptr;
}

#if SK_SUPPORT_GPU
#include "GrFragmentProcessor.h"
#include "GrInvariantOutput.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"

class ColorMatrixEffect : public GrFragmentProcessor {
public:
    static sk_sp<GrFragmentProcessor> Make(const SkScalar matrix[20]) {
        return sk_sp<GrFragmentProcessor>(new ColorMatrixEffect(matrix));
    }

    const char* name() const override { return "Color Matrix"; }

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    class GLSLProcessor : public GrGLSLFragmentProcessor {
    public:
        // this class always generates the same code.
        static void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*) {}

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
    ColorMatrixEffect(const SkScalar matrix[20]) {
        memcpy(fMatrix, matrix, sizeof(SkScalar) * 20);
        this->initClassID<ColorMatrixEffect>();
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        return new GLSLProcessor;
    }

    virtual void onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                       GrProcessorKeyBuilder* b) const override {
        GLSLProcessor::GenKey(*this, caps, b);
    }

    bool onIsEqual(const GrFragmentProcessor& s) const override {
        const ColorMatrixEffect& cme = s.cast<ColorMatrixEffect>();
        return 0 == memcmp(fMatrix, cme.fMatrix, sizeof(fMatrix));
    }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        // We only bother to check whether the alpha channel will be constant. If SkColorMatrix had
        // type flags it might be worth checking the other components.

        // The matrix is defined such the 4th row determines the output alpha. The first four
        // columns of that row multiply the input r, g, b, and a, respectively, and the last column
        // is the "translation".
        static const uint32_t kRGBAFlags[] = {
            kR_GrColorComponentFlag,
            kG_GrColorComponentFlag,
            kB_GrColorComponentFlag,
            kA_GrColorComponentFlag
        };
        static const int kShifts[] = {
            GrColor_SHIFT_R, GrColor_SHIFT_G, GrColor_SHIFT_B, GrColor_SHIFT_A,
        };
        enum {
            kAlphaRowStartIdx = 15,
            kAlphaRowTranslateIdx = 19,
        };

        SkScalar outputA = 0;
        for (int i = 0; i < 4; ++i) {
            // If any relevant component of the color to be passed through the matrix is non-const
            // then we can't know the final result.
            if (0 != fMatrix[kAlphaRowStartIdx + i]) {
                if (!(inout->validFlags() & kRGBAFlags[i])) {
                    inout->setToUnknown(GrInvariantOutput::kWill_ReadInput);
                    return;
                } else {
                    uint32_t component = (inout->color() >> kShifts[i]) & 0xFF;
                    outputA += fMatrix[kAlphaRowStartIdx + i] * component;
                }
            }
        }
        outputA += fMatrix[kAlphaRowTranslateIdx];
        // We pin the color to [0,1]. This would happen to the *final* color output from the frag
        // shader but currently the effect does not pin its own output. So in the case of over/
        // underflow this may deviate from the actual result. Maybe the effect should pin its
        // result if the matrix could over/underflow for any component?
        inout->setToOther(kA_GrColorComponentFlag,
                          static_cast<uint8_t>(SkScalarPin(outputA, 0, 255)) << GrColor_SHIFT_A,
                          GrInvariantOutput::kWill_ReadInput);
    }

    SkScalar fMatrix[20];

    typedef GrFragmentProcessor INHERITED;
};

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(ColorMatrixEffect);

sk_sp<GrFragmentProcessor> ColorMatrixEffect::TestCreate(GrProcessorTestData* d) {
    SkScalar colorMatrix[20];
    for (size_t i = 0; i < SK_ARRAY_COUNT(colorMatrix); ++i) {
        colorMatrix[i] = d->fRandom->nextSScalar1();
    }
    return ColorMatrixEffect::Make(colorMatrix);
}

sk_sp<GrFragmentProcessor> SkColorMatrixFilterRowMajor255::asFragmentProcessor(GrContext*) const {
    return ColorMatrixEffect::Make(fMatrix);
}

#endif

#ifndef SK_IGNORE_TO_STRING
void SkColorMatrixFilterRowMajor255::toString(SkString* str) const {
    str->append("SkColorMatrixFilterRowMajor255: ");

    str->append("matrix: (");
    for (int i = 0; i < 20; ++i) {
        str->appendScalar(fMatrix[i]);
        if (i < 19) {
            str->append(", ");
        }
    }
    str->append(")");
}
#endif

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkColorFilter> SkColorFilter::MakeMatrixFilterRowMajor255(const SkScalar array[20]) {
    return sk_sp<SkColorFilter>(new SkColorMatrixFilterRowMajor255(array));
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkColorFilter>
SkColorMatrixFilterRowMajor255::MakeSingleChannelOutput(const SkScalar row[5]) {
    SkASSERT(row);
    auto cf = sk_make_sp<SkColorMatrixFilterRowMajor255>();
    static_assert(sizeof(SkScalar) * 5 * 4 == sizeof(cf->fMatrix), "sizes don't match");
    for (int i = 0; i < 4; ++i) {
        memcpy(cf->fMatrix + 5 * i, row, sizeof(SkScalar) * 5);
    }
    cf->initState();
    return cf;
}
