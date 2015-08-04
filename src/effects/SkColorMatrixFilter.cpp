/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorMatrixFilter.h"
#include "SkColorMatrix.h"
#include "SkColorPriv.h"
#include "SkPMFloat.h"
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

static int32_t rowmul4(const int32_t array[], unsigned r, unsigned g,
                          unsigned b, unsigned a) {
    return array[0] * r + array[1] * g  + array[2] * b + array[3] * a + array[4];
}

static int32_t rowmul3(const int32_t array[], unsigned r, unsigned g,
                       unsigned b) {
    return array[0] * r + array[1] * g  + array[2] * b + array[4];
}

static void General(const SkColorMatrixFilter::State& state,
                    unsigned r, unsigned g, unsigned b, unsigned a,
                    int32_t* SK_RESTRICT result) {
    const int32_t* SK_RESTRICT array = state.fArray;
    const int shift = state.fShift;

    result[0] = rowmul4(&array[0], r, g, b, a) >> shift;
    result[1] = rowmul4(&array[5], r, g, b, a) >> shift;
    result[2] = rowmul4(&array[10], r, g, b, a) >> shift;
    result[3] = rowmul4(&array[15], r, g, b, a) >> shift;
}

static void General16(const SkColorMatrixFilter::State& state,
                      unsigned r, unsigned g, unsigned b, unsigned a,
                      int32_t* SK_RESTRICT result) {
    const int32_t* SK_RESTRICT array = state.fArray;

    result[0] = rowmul4(&array[0], r, g, b, a) >> 16;
    result[1] = rowmul4(&array[5], r, g, b, a) >> 16;
    result[2] = rowmul4(&array[10], r, g, b, a) >> 16;
    result[3] = rowmul4(&array[15], r, g, b, a) >> 16;
}

static void AffineAdd(const SkColorMatrixFilter::State& state,
                      unsigned r, unsigned g, unsigned b, unsigned a,
                      int32_t* SK_RESTRICT result) {
    const int32_t* SK_RESTRICT array = state.fArray;
    const int shift = state.fShift;

    result[0] = rowmul3(&array[0], r, g, b) >> shift;
    result[1] = rowmul3(&array[5], r, g, b) >> shift;
    result[2] = rowmul3(&array[10], r, g, b) >> shift;
    result[3] = a;
}

static void AffineAdd16(const SkColorMatrixFilter::State& state,
                        unsigned r, unsigned g, unsigned b, unsigned a,
                        int32_t* SK_RESTRICT result) {
    const int32_t* SK_RESTRICT array = state.fArray;

    result[0] = rowmul3(&array[0], r, g, b) >> 16;
    result[1] = rowmul3(&array[5], r, g, b) >> 16;
    result[2] = rowmul3(&array[10], r, g, b) >> 16;
    result[3] = a;
}

static void ScaleAdd(const SkColorMatrixFilter::State& state,
                     unsigned r, unsigned g, unsigned b, unsigned a,
                     int32_t* SK_RESTRICT result) {
    const int32_t* SK_RESTRICT array = state.fArray;
    const int shift = state.fShift;

    // cast to (int) to keep the expression signed for the shift
    result[0] = (array[SkColorMatrix::kR_Scale] * (int)r + array[4]) >> shift;
    result[1] = (array[SkColorMatrix::kG_Scale] * (int)g + array[9]) >> shift;
    result[2] = (array[SkColorMatrix::kB_Scale] * (int)b + array[14]) >> shift;
    result[3] = a;
}

static void ScaleAdd16(const SkColorMatrixFilter::State& state,
                       unsigned r, unsigned g, unsigned b, unsigned a,
                       int32_t* SK_RESTRICT result) {
    const int32_t* SK_RESTRICT array = state.fArray;

    // cast to (int) to keep the expression signed for the shift
    result[0] = (array[SkColorMatrix::kR_Scale] * (int)r + array[4]) >> 16;
    result[1] = (array[SkColorMatrix::kG_Scale] * (int)g + array[9]) >> 16;
    result[2] = (array[SkColorMatrix::kB_Scale] * (int)b + array[14]) >> 16;
    result[3] = a;
}

static void Add(const SkColorMatrixFilter::State& state,
                unsigned r, unsigned g, unsigned b, unsigned a,
                int32_t* SK_RESTRICT result) {
    const int32_t* SK_RESTRICT array = state.fArray;
    const int shift = state.fShift;

    result[0] = r + (array[SkColorMatrix::kR_Trans] >> shift);
    result[1] = g + (array[SkColorMatrix::kG_Trans] >> shift);
    result[2] = b + (array[SkColorMatrix::kB_Trans] >> shift);
    result[3] = a;
}

static void Add16(const SkColorMatrixFilter::State& state,
                  unsigned r, unsigned g, unsigned b, unsigned a,
                  int32_t* SK_RESTRICT result) {
    const int32_t* SK_RESTRICT array = state.fArray;

    result[0] = r + (array[SkColorMatrix::kR_Trans] >> 16);
    result[1] = g + (array[SkColorMatrix::kG_Trans] >> 16);
    result[2] = b + (array[SkColorMatrix::kB_Trans] >> 16);
    result[3] = a;
}

// src is [20] but some compilers won't accept __restrict__ on anything
// but an raw pointer or reference
void SkColorMatrixFilter::initState(const SkScalar* SK_RESTRICT src) {
    transpose_to_pmorder(fTranspose, src);

    int32_t* array = fState.fArray;
    SkFixed max = 0;
    for (int i = 0; i < 20; i++) {
        SkFixed value = SkScalarToFixed(src[i]);
        array[i] = value;
        value = SkAbs32(value);
        max = SkMax32(max, value);
    }

    /*  All of fArray[] values must fit in 23 bits, to safely allow me to
        multiply them by 8bit unsigned values, and get a signed answer without
        overflow. This means clz needs to be 9 or bigger
    */
    int bits = SkCLZ(max);
    int32_t one = SK_Fixed1;

    fState.fShift = 16; // we are starting out as fixed 16.16
    if (bits < 9) {
        bits = 9 - bits;
        fState.fShift -= bits;
        for (int i = 0; i < 20; i++) {
            array[i] >>= bits;
        }
        one >>= bits;
    }

    // check if we have to munge Alpha
    int32_t changesAlpha = (array[15] | array[16] | array[17] |
                            (array[18] - one) | array[19]);
    int32_t usesAlpha = (array[3] | array[8] | array[13]);
    bool shiftIs16 = (16 == fState.fShift);

    if (changesAlpha | usesAlpha) {
        fProc = shiftIs16 ? General16 : General;
        fFlags = changesAlpha ? 0 : SkColorFilter::kAlphaUnchanged_Flag;
    } else {
        fFlags = SkColorFilter::kAlphaUnchanged_Flag;

        int32_t needsScale = (array[SkColorMatrix::kR_Scale] - one) |
                             (array[SkColorMatrix::kG_Scale] - one) |
                             (array[SkColorMatrix::kB_Scale] - one);

        int32_t needs3x3 =  array[1] | array[2] |     // red off-axis
                            array[5] | array[7] |     // green off-axis
                            array[10] | array[11];    // blue off-axis

        if (needs3x3) {
            fProc = shiftIs16 ? AffineAdd16 : AffineAdd;
        } else if (needsScale) {
            fProc = shiftIs16 ? ScaleAdd16 : ScaleAdd;
        } else if (array[SkColorMatrix::kR_Trans] |
                   array[SkColorMatrix::kG_Trans] |
                   array[SkColorMatrix::kB_Trans]) {
            fProc = shiftIs16 ? Add16 : Add;
        } else {
            fProc = NULL;   // identity
        }
    }

    /*  preround our add values so we get a rounded shift. We do this after we
        analyze the array, so we don't miss the case where the caller has zeros
        which could make us accidentally take the General or Add case.
    */
    if (fProc) {
        int32_t add = 1 << (fState.fShift - 1);
        array[4] += add;
        array[9] += add;
        array[14] += add;
        array[19] += add;
    }
}

///////////////////////////////////////////////////////////////////////////////

static int32_t pin(int32_t value, int32_t max) {
    if (value < 0) {
        value = 0;
    }
    if (value > max) {
        value = max;
    }
    return value;
}

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

static Sk4f premul(const Sk4f& x) {
    float scale = SkPMFloat(x).a();
    Sk4f pm = x * SkPMFloat(1, scale, scale, scale);

#ifdef SK_DEBUG
    SkPMFloat pmf(pm);
    SkASSERT(pmf.isValid());
#endif

    return pm;
}

static Sk4f unpremul(const SkPMFloat& pm) {
    float scale = 1 / pm.a(); // candidate for fast/approx invert?
    return pm * SkPMFloat(1, scale, scale, scale);
}

static Sk4f clamp_0_1(const Sk4f& value) {
    return Sk4f::Max(Sk4f::Min(value, Sk4f(1)), Sk4f(0));
}

void SkColorMatrixFilter::filterSpan(const SkPMColor src[], int count, SkPMColor dst[]) const {
    Proc proc = fProc;
    if (NULL == proc) {
        if (src != dst) {
            memcpy(dst, src, count * sizeof(SkPMColor));
        }
        return;
    }

#ifdef SK_SUPPORT_LEGACY_INT_COLORMATRIX
    const bool use_floats = false;
#else
    const bool use_floats = true;
#endif

    if (use_floats) {
        // c0-c3 are already in [0,1].
        const Sk4f c0 = Sk4f::Load(fTranspose + 0);
        const Sk4f c1 = Sk4f::Load(fTranspose + 4);
        const Sk4f c2 = Sk4f::Load(fTranspose + 8);
        const Sk4f c3 = Sk4f::Load(fTranspose + 12);
        // c4 (the translate vector) is in [0, 255].  Bring it back to [0,1].
        const Sk4f c4 = Sk4f::Load(fTranspose + 16)*Sk4f(1.0f/255);

        // todo: we could cache this in the constructor...
        SkPMColor matrix_translate_pmcolor = SkPMFloat(premul(clamp_0_1(c4))).round();

        for (int i = 0; i < count; i++) {
            const SkPMColor src_c = src[i];
            if (0 == src_c) {
                dst[i] = matrix_translate_pmcolor;
                continue;
            }

            SkPMFloat srcf(src_c);

            if (0xFF != SkGetPackedA32(src_c)) {
                srcf = unpremul(srcf);
            }

            Sk4f r4 = Sk4f(srcf.r());
            Sk4f g4 = Sk4f(srcf.g());
            Sk4f b4 = Sk4f(srcf.b());
            Sk4f a4 = Sk4f(srcf.a());

            // apply matrix
            Sk4f dst4 = c0 * r4 + c1 * g4 + c2 * b4 + c3 * a4 + c4;

            // clamp, re-premul, and write
            dst[i] = SkPMFloat(premul(clamp_0_1(dst4))).round();
        }
    } else {
        const State& state = fState;
        int32_t result[4];
        const SkUnPreMultiply::Scale* table = SkUnPreMultiply::GetScaleTable();

        for (int i = 0; i < count; i++) {
            SkPMColor c = src[i];

            unsigned r = SkGetPackedR32(c);
            unsigned g = SkGetPackedG32(c);
            unsigned b = SkGetPackedB32(c);
            unsigned a = SkGetPackedA32(c);

            // need our components to be un-premultiplied
            if (255 != a) {
                SkUnPreMultiply::Scale scale = table[a];
                r = SkUnPreMultiply::ApplyScale(scale, r);
                g = SkUnPreMultiply::ApplyScale(scale, g);
                b = SkUnPreMultiply::ApplyScale(scale, b);

                SkASSERT(r <= 255);
                SkASSERT(g <= 255);
                SkASSERT(b <= 255);
            }

            proc(state, r, g, b, a, result);

            r = pin(result[0], SK_R32_MASK);
            g = pin(result[1], SK_G32_MASK);
            b = pin(result[2], SK_B32_MASK);
            a = pin(result[3], SK_A32_MASK);
            // re-prepremultiply if needed
            dst[i] = SkPremultiplyARGBInline(a, r, g, b);
        }
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
    return NULL;
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
    return NULL;
}

#if SK_SUPPORT_GPU
#include "GrFragmentProcessor.h"
#include "GrInvariantOutput.h"
#include "gl/GrGLFragmentProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"

class ColorMatrixEffect : public GrFragmentProcessor {
public:
    static GrFragmentProcessor* Create(const SkColorMatrix& matrix) {
        return SkNEW_ARGS(ColorMatrixEffect, (matrix));
    }

    const char* name() const override { return "Color Matrix"; }

    GrGLFragmentProcessor* createGLInstance() const override {
        return SkNEW_ARGS(GLProcessor, (*this));
    }


    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    class GLProcessor : public GrGLFragmentProcessor {
    public:
        // this class always generates the same code.
        static void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder* b) {}

        GLProcessor(const GrProcessor&) {}

        virtual void emitCode(EmitArgs& args) override {
            fMatrixHandle = args.fBuilder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                                kMat44f_GrSLType, kDefault_GrSLPrecision,
                                                "ColorMatrix");
            fVectorHandle = args.fBuilder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                                kVec4f_GrSLType, kDefault_GrSLPrecision,
                                                "ColorMatrixVector");

            if (NULL == args.fInputColor) {
                // could optimize this case, but we aren't for now.
                args.fInputColor = "vec4(1)";
            }
            GrGLFragmentBuilder* fsBuilder = args.fBuilder->getFragmentShaderBuilder();
            // The max() is to guard against 0 / 0 during unpremul when the incoming color is
            // transparent black.
            fsBuilder->codeAppendf("\tfloat nonZeroAlpha = max(%s.a, 0.00001);\n",
                                   args.fInputColor);
            fsBuilder->codeAppendf("\t%s = %s * vec4(%s.rgb / nonZeroAlpha, nonZeroAlpha) + %s;\n",
                                   args.fOutputColor,
                                   args.fBuilder->getUniformCStr(fMatrixHandle),
                                   args.fInputColor,
                                   args.fBuilder->getUniformCStr(fVectorHandle));
            fsBuilder->codeAppendf("\t%s = clamp(%s, 0.0, 1.0);\n",
                                   args.fOutputColor, args.fOutputColor);
            fsBuilder->codeAppendf("\t%s.rgb *= %s.a;\n", args.fOutputColor, args.fOutputColor);
        }

        virtual void setData(const GrGLProgramDataManager& uniManager,
                             const GrProcessor& proc) override {
            const ColorMatrixEffect& cme = proc.cast<ColorMatrixEffect>();
            const float* m = cme.fMatrix.fMat;
            // The GL matrix is transposed from SkColorMatrix.
            GrGLfloat mt[]  = {
                m[0], m[5], m[10], m[15],
                m[1], m[6], m[11], m[16],
                m[2], m[7], m[12], m[17],
                m[3], m[8], m[13], m[18],
            };
            static const float kScale = 1.0f / 255.0f;
            GrGLfloat vec[] = {
                m[4] * kScale, m[9] * kScale, m[14] * kScale, m[19] * kScale,
            };
            uniManager.setMatrix4fv(fMatrixHandle, 1, mt);
            uniManager.set4fv(fVectorHandle, 1, vec);
        }

    private:
        GrGLProgramDataManager::UniformHandle fMatrixHandle;
        GrGLProgramDataManager::UniformHandle fVectorHandle;

        typedef GrGLFragmentProcessor INHERITED;
    };

private:
    ColorMatrixEffect(const SkColorMatrix& matrix) : fMatrix(matrix) {
        this->initClassID<ColorMatrixEffect>();
    }

    virtual void onGetGLProcessorKey(const GrGLSLCaps& caps,
                                     GrProcessorKeyBuilder* b) const override {
        GLProcessor::GenKey(*this, caps, b);
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

GrFragmentProcessor* ColorMatrixEffect::TestCreate(GrProcessorTestData* d) {
    SkColorMatrix colorMatrix;
    for (size_t i = 0; i < SK_ARRAY_COUNT(colorMatrix.fMat); ++i) {
        colorMatrix.fMat[i] = d->fRandom->nextSScalar1();
    }
    return ColorMatrixEffect::Create(colorMatrix);
}

bool SkColorMatrixFilter::asFragmentProcessors(GrContext*, GrProcessorDataManager*,
                                               SkTDArray<GrFragmentProcessor*>* array) const {
    GrFragmentProcessor* frag = ColorMatrixEffect::Create(fMatrix);
    if (frag) {
        if (array) {
            *array->append() = frag;
        } else {
            frag->unref();
            SkDEBUGCODE(frag = NULL;)
        }
        return true;
    }
    return false;
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
