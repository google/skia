/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorMatrixFilter.h"
#include "SkColorMatrix.h"
#include "SkColorPriv.h"
#include "SkNx.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkUnPreMultiply.h"
#include "SkString.h"

#define SK_PMORDER_INDEX_A  (SK_A32_SHIFT / 8)
#define SK_PMORDER_INDEX_R  (SK_R32_SHIFT / 8)
#define SK_PMORDER_INDEX_G  (SK_G32_SHIFT / 8)
#define SK_PMORDER_INDEX_B  (SK_B32_SHIFT / 8)

static void transpose_to_pmorder(float dst[20], const float src[20]) {
    const float* srcR = src + 0;
    const float* srcG = src + 5;
    const float* srcB = src + 10;
    const float* srcA = src + 15;

    for (int i = 0; i < 20; i += 4) {
        dst[i + SK_PMORDER_INDEX_A] = *srcA++;
        dst[i + SK_PMORDER_INDEX_R] = *srcR++;
        dst[i + SK_PMORDER_INDEX_G] = *srcG++;
        dst[i + SK_PMORDER_INDEX_B] = *srcB++;
    }
}

// src is [20] but some compilers won't accept __restrict__ on anything
// but an raw pointer or reference
void SkColorMatrixFilter::initState(const SkScalar* SK_RESTRICT src) {
    transpose_to_pmorder(fTranspose, src);

    const float* array = fMatrix.fMat;

    // check if we have to munge Alpha
    bool changesAlpha = (array[15] || array[16] || array[17] || (array[18] - 1) || array[19]);
    bool usesAlpha = (array[3] || array[8] || array[13]);

    if (changesAlpha || usesAlpha) {
        fFlags = changesAlpha ? 0 : SkColorFilter::kAlphaUnchanged_Flag;
    } else {
        fFlags = SkColorFilter::kAlphaUnchanged_Flag;
    }
}

///////////////////////////////////////////////////////////////////////////////

SkColorMatrixFilter::SkColorMatrixFilter(const SkColorMatrix& cm) : fMatrix(cm) {
    this->initState(cm.fMat);
}

SkColorMatrixFilter::SkColorMatrixFilter(const SkScalar array[20]) {
    memcpy(fMatrix.fMat, array, 20 * sizeof(SkScalar));
    this->initState(array);
}

uint32_t SkColorMatrixFilter::getFlags() const {
    return this->INHERITED::getFlags() | fFlags;
}

static Sk4f scale_rgb(float scale) {
    static_assert(SK_A32_SHIFT == 24, "Alpha is lane 3");
    return Sk4f(scale, scale, scale, 1);
}

static Sk4f premul(const Sk4f& x) {
    return x * scale_rgb(x.kth<SK_A32_SHIFT/8>());
}

static Sk4f unpremul(const Sk4f& x) {
    return x * scale_rgb(1 / x.kth<SK_A32_SHIFT/8>());  // TODO: fast/approx invert?
}

static Sk4f clamp_0_1(const Sk4f& x) {
    return Sk4f::Max(Sk4f::Min(x, Sk4f(1)), Sk4f(0));
}

static SkPMColor round(const Sk4f& x) {
    SkPMColor c;
    SkNx_cast<uint8_t>(x * Sk4f(255) + Sk4f(0.5f)).store((uint8_t*)&c);
    return c;
}

void SkColorMatrixFilter::filterSpan(const SkPMColor src[], int count, SkPMColor dst[]) const {
    // c0-c3 are already in [0,1].
    const Sk4f c0 = Sk4f::Load(fTranspose + 0);
    const Sk4f c1 = Sk4f::Load(fTranspose + 4);
    const Sk4f c2 = Sk4f::Load(fTranspose + 8);
    const Sk4f c3 = Sk4f::Load(fTranspose + 12);
    // c4 (the translate vector) is in [0, 255].  Bring it back to [0,1].
    const Sk4f c4 = Sk4f::Load(fTranspose + 16)*Sk4f(1.0f/255);

    // todo: we could cache this in the constructor...
    SkPMColor matrix_translate_pmcolor = round(premul(clamp_0_1(c4)));

    for (int i = 0; i < count; i++) {
        const SkPMColor src_c = src[i];
        if (0 == src_c) {
            dst[i] = matrix_translate_pmcolor;
            continue;
        }

        Sk4f srcf = SkNx_cast<float>(Sk4b::Load((const uint8_t*)&src_c)) * Sk4f(1.0f/255);

        if (0xFF != SkGetPackedA32(src_c)) {
            srcf = unpremul(srcf);
        }

        Sk4f r4 = SkNx_dup<SK_R32_SHIFT/8>(srcf);
        Sk4f g4 = SkNx_dup<SK_G32_SHIFT/8>(srcf);
        Sk4f b4 = SkNx_dup<SK_B32_SHIFT/8>(srcf);
        Sk4f a4 = SkNx_dup<SK_A32_SHIFT/8>(srcf);

        // apply matrix
        Sk4f dst4 = c0 * r4 + c1 * g4 + c2 * b4 + c3 * a4 + c4;

        // clamp, re-premul, and write
        dst[i] = round(premul(clamp_0_1(dst4)));
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkColorMatrixFilter::flatten(SkWriteBuffer& buffer) const {
    SkASSERT(sizeof(fMatrix.fMat)/sizeof(SkScalar) == 20);
    buffer.writeScalarArray(fMatrix.fMat, 20);
}

SkFlattenable* SkColorMatrixFilter::CreateProc(SkReadBuffer& buffer) {
    SkColorMatrix matrix;
    if (buffer.readScalarArray(matrix.fMat, 20)) {
        return Create(matrix);
    }
    return nullptr;
}

bool SkColorMatrixFilter::asColorMatrix(SkScalar matrix[20]) const {
    if (matrix) {
        memcpy(matrix, fMatrix.fMat, 20 * sizeof(SkScalar));
    }
    return true;
}

SkColorFilter* SkColorMatrixFilter::newComposed(const SkColorFilter* innerFilter) const {
    SkScalar innerMatrix[20];
    if (innerFilter->asColorMatrix(innerMatrix) && !SkColorMatrix::NeedsClamping(innerMatrix)) {
        SkScalar concat[20];
        SkColorMatrix::SetConcat(concat, fMatrix.fMat, innerMatrix);
        return SkColorMatrixFilter::Create(concat);
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
    static const GrFragmentProcessor* Create(const SkColorMatrix& matrix) {
        return new ColorMatrixEffect(matrix);
    }

    const char* name() const override { return "Color Matrix"; }

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    class GLSLProcessor : public GrGLSLFragmentProcessor {
    public:
        // this class always generates the same code.
        static void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder* b) {}

        GLSLProcessor(const GrProcessor&) {}

        virtual void emitCode(EmitArgs& args) override {
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
            fMatrixHandle = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                                       kMat44f_GrSLType, kDefault_GrSLPrecision,
                                                       "ColorMatrix");
            fVectorHandle = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
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
        virtual void onSetData(const GrGLSLProgramDataManager& uniManager,
                               const GrProcessor& proc) override {
            const ColorMatrixEffect& cme = proc.cast<ColorMatrixEffect>();
            const float* m = cme.fMatrix.fMat;
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
    ColorMatrixEffect(const SkColorMatrix& matrix) : fMatrix(matrix) {
        this->initClassID<ColorMatrixEffect>();
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        return new GLSLProcessor(*this);
    }

    virtual void onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                       GrProcessorKeyBuilder* b) const override {
        GLSLProcessor::GenKey(*this, caps, b);
    }

    bool onIsEqual(const GrFragmentProcessor& s) const override {
        const ColorMatrixEffect& cme = s.cast<ColorMatrixEffect>();
        return cme.fMatrix == fMatrix;
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
            if (0 != fMatrix.fMat[kAlphaRowStartIdx + i]) {
                if (!(inout->validFlags() & kRGBAFlags[i])) {
                    inout->setToUnknown(GrInvariantOutput::kWill_ReadInput);
                    return;
                } else {
                    uint32_t component = (inout->color() >> kShifts[i]) & 0xFF;
                    outputA += fMatrix.fMat[kAlphaRowStartIdx + i] * component;
                }
            }
        }
        outputA += fMatrix.fMat[kAlphaRowTranslateIdx];
        // We pin the color to [0,1]. This would happen to the *final* color output from the frag
        // shader but currently the effect does not pin its own output. So in the case of over/
        // underflow this may deviate from the actual result. Maybe the effect should pin its
        // result if the matrix could over/underflow for any component?
        inout->setToOther(kA_GrColorComponentFlag,
                          static_cast<uint8_t>(SkScalarPin(outputA, 0, 255)) << GrColor_SHIFT_A,
                          GrInvariantOutput::kWill_ReadInput);
    }

    SkColorMatrix fMatrix;

    typedef GrFragmentProcessor INHERITED;
};

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(ColorMatrixEffect);

const GrFragmentProcessor* ColorMatrixEffect::TestCreate(GrProcessorTestData* d) {
    SkColorMatrix colorMatrix;
    for (size_t i = 0; i < SK_ARRAY_COUNT(colorMatrix.fMat); ++i) {
        colorMatrix.fMat[i] = d->fRandom->nextSScalar1();
    }
    return ColorMatrixEffect::Create(colorMatrix);
}

const GrFragmentProcessor* SkColorMatrixFilter::asFragmentProcessor(GrContext*) const {
    return ColorMatrixEffect::Create(fMatrix);
}

#endif

#ifndef SK_IGNORE_TO_STRING
void SkColorMatrixFilter::toString(SkString* str) const {
    str->append("SkColorMatrixFilter: ");

    str->append("matrix: (");
    for (int i = 0; i < 20; ++i) {
        str->appendScalar(fMatrix.fMat[i]);
        if (i < 19) {
            str->append(", ");
        }
    }
    str->append(")");
}
#endif

///////////////////////////////////////////////////////////////////////////////

static SkScalar byte_to_scale(U8CPU byte) {
    if (0xFF == byte) {
        // want to get this exact
        return 1;
    } else {
        return byte * 0.00392156862745f;
    }
}

SkColorFilter* SkColorMatrixFilter::CreateLightingFilter(SkColor mul, SkColor add) {
    SkColorMatrix matrix;
    matrix.setScale(byte_to_scale(SkColorGetR(mul)),
                    byte_to_scale(SkColorGetG(mul)),
                    byte_to_scale(SkColorGetB(mul)),
                    1);
    matrix.postTranslate(SkIntToScalar(SkColorGetR(add)),
                         SkIntToScalar(SkColorGetG(add)),
                         SkIntToScalar(SkColorGetB(add)),
                         0);
    return SkColorMatrixFilter::Create(matrix);
}
