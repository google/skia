/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorMatrixFilterRowMajor255.h"
#include "SkColorData.h"
#include "SkNx.h"
#include "SkRasterPipeline.h"
#include "SkReadBuffer.h"
#include "SkRefCnt.h"
#include "SkString.h"
#include "SkUnPreMultiply.h"
#include "SkWriteBuffer.h"

void SkColorMatrixFilterRowMajor255::initState() {
    const float* srcR = fMatrix + 0;
    const float* srcG = fMatrix + 5;
    const float* srcB = fMatrix + 10;
    const float* srcA = fMatrix + 15;

    fFlags = (srcA[0] == 0 && srcA[1] == 0 && srcA[2] == 0 && srcA[3] == 1 && srcA[4] == 0)
        ? kAlphaUnchanged_Flag : 0;

    for (int i = 0; i < 16; i += 4) {
        fTranspose[i + 0] = *srcR++;
        fTranspose[i + 1] = *srcG++;
        fTranspose[i + 2] = *srcB++;
        fTranspose[i + 3] = *srcA++;
    }
    // Might as well scale these translates down to [0,1] here instead of every filter call.
    fTranspose[16] = *srcR * (1/255.0f);
    fTranspose[17] = *srcG * (1/255.0f);
    fTranspose[18] = *srcB * (1/255.0f);
    fTranspose[19] = *srcA * (1/255.0f);
}

SkColorMatrixFilterRowMajor255::SkColorMatrixFilterRowMajor255(const SkScalar array[20]) {
    memcpy(fMatrix, array, 20 * sizeof(SkScalar));
    this->initState();
}

uint32_t SkColorMatrixFilterRowMajor255::getFlags() const {
    return this->INHERITED::getFlags() | fFlags;
}

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

void SkColorMatrixFilterRowMajor255::onAppendStages(SkRasterPipeline* p,
                                                    SkColorSpace* dst,
                                                    SkArenaAlloc* scratch,
                                                    const bool shaderIsOpaque) const {
    const bool willStayOpaque = shaderIsOpaque && (fFlags & kAlphaUnchanged_Flag);

    if (!shaderIsOpaque) { p->append(SkRasterPipeline::unpremul); }
    if (           true) { p->append(SkRasterPipeline::matrix_4x5, fTranspose); }
    if (           true) { p->append(SkRasterPipeline::clamp_0); }
    if (           true) { p->append(SkRasterPipeline::clamp_1); }
    if (!willStayOpaque) { p->append(SkRasterPipeline::premul); }
}

#if SK_SUPPORT_GPU
#include "GrFragmentProcessor.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"

class ColorMatrixEffect : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(const SkScalar matrix[20]) {
        return std::unique_ptr<GrFragmentProcessor>(new ColorMatrixEffect(matrix));
    }

    const char* name() const override { return "Color Matrix"; }

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    std::unique_ptr<GrFragmentProcessor> clone() const override { return Make(fMatrix); }

private:
    class GLSLProcessor : public GrGLSLFragmentProcessor {
    public:
        // this class always generates the same code.
        static void GenKey(const GrProcessor&, const GrShaderCaps&, GrProcessorKeyBuilder*) {}

        void emitCode(EmitArgs& args) override {
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
            fMatrixHandle = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf4x4_GrSLType,
                                                       "ColorMatrix");
            fVectorHandle = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf4_GrSLType,
                                                       "ColorMatrixVector");

            GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
            // The max() is to guard against 0 / 0 during unpremul when the incoming color is
            // transparent black.
            fragBuilder->codeAppendf("\thalf nonZeroAlpha = max(%s.a, 0.00001);\n",
                                     args.fInputColor);
            fragBuilder->codeAppendf("\t%s = %s * half4(%s.rgb / nonZeroAlpha, nonZeroAlpha) + "
                                     "%s;\n",
                                     args.fOutputColor,
                                     uniformHandler->getUniformCStr(fMatrixHandle),
                                     args.fInputColor,
                                     uniformHandler->getUniformCStr(fVectorHandle));
            fragBuilder->codeAppendf("\t%s = saturate(%s);\n",
                                     args.fOutputColor, args.fOutputColor);
            fragBuilder->codeAppendf("\t%s.rgb *= %s.a;\n", args.fOutputColor, args.fOutputColor);
        }

    protected:
        void onSetData(const GrGLSLProgramDataManager& uniManager,
                       const GrFragmentProcessor& proc) override {
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

    // We could implement the constant input->constant output optimization but haven't. Other
    // optimizations would be matrix-dependent.
    ColorMatrixEffect(const SkScalar matrix[20])
    : INHERITED(kColorMatrixEffect_ClassID, kNone_OptimizationFlags) {
        memcpy(fMatrix, matrix, sizeof(SkScalar) * 20);
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

    SkScalar fMatrix[20];

    typedef GrFragmentProcessor INHERITED;
};

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(ColorMatrixEffect);

#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> ColorMatrixEffect::TestCreate(GrProcessorTestData* d) {
    SkScalar colorMatrix[20];
    for (size_t i = 0; i < SK_ARRAY_COUNT(colorMatrix); ++i) {
        colorMatrix[i] = d->fRandom->nextSScalar1();
    }
    return ColorMatrixEffect::Make(colorMatrix);
}

#endif

std::unique_ptr<GrFragmentProcessor> SkColorMatrixFilterRowMajor255::asFragmentProcessor(
        GrRecordingContext*, const GrColorSpaceInfo&) const {
    return ColorMatrixEffect::Make(fMatrix);
}

#endif

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkColorFilter> SkColorFilter::MakeMatrixFilterRowMajor255(const SkScalar array[20]) {
    if (!SkScalarsAreFinite(array, 20)) {
        return nullptr;
    }

    return sk_sp<SkColorFilter>(new SkColorMatrixFilterRowMajor255(array));
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkColorFilter>
SkColorMatrixFilterRowMajor255::MakeSingleChannelOutput(const SkScalar row[5]) {
    if (!SkScalarsAreFinite(row, 5)) {
        return nullptr;
    }

    SkASSERT(row);
    auto cf = sk_make_sp<SkColorMatrixFilterRowMajor255>();
    static_assert(sizeof(SkScalar) * 5 * 4 == sizeof(cf->fMatrix), "sizes don't match");
    for (int i = 0; i < 4; ++i) {
        memcpy(cf->fMatrix + 5 * i, row, sizeof(SkScalar) * 5);
    }
    cf->initState();
    return std::move(cf);
}
