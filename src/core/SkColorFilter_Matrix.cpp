/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilter_Matrix.h"
#include "SkColorData.h"
#include "SkColorMatrix.h"
#include "SkNx.h"
#include "SkRasterPipeline.h"
#include "SkReadBuffer.h"
#include "SkRefCnt.h"
#include "SkString.h"
#include "SkUnPreMultiply.h"
#include "SkWriteBuffer.h"

static void rowMajor_to_colMajor_div255(float dst[20], const float src[20]) {
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
    dst[16] = *srcR * (1/255.0f);
    dst[17] = *srcG * (1/255.0f);
    dst[18] = *srcB * (1/255.0f);
    dst[19] = *srcA * (1/255.0f);
}

static void colMajor_to_rowMajor_mul255(float dst[20], const float src[20]) {
    for (int i = 0; i < 4; ++i) {
        dst[i +  0] = *src++;
        dst[i +  5] = *src++;
        dst[i + 10] = *src++;
        dst[i + 15] = *src++;
    }
    dst[ 4] = src[0] * 255;
    dst[ 9] = src[1] * 255;
    dst[14] = src[2] * 255;
    dst[19] = src[3] * 255;
}

void SkColorFilter_Matrix::initState() {
    fFlags = (fMatrix[ 3] == 0 && fMatrix[ 7] == 0 && fMatrix[11] == 0 &&
              fMatrix[15] == 1 && fMatrix[19] == 0)
        ? kAlphaUnchanged_Flag : 0;
}

SkColorFilter_Matrix::SkColorFilter_Matrix(const SkScalar array[20]) {
    memcpy(fMatrix, array, sizeof(fMatrix));
    this->initState();
}

uint32_t SkColorFilter_Matrix::getFlags() const {
    return this->INHERITED::getFlags() | fFlags;
}

void SkColorFilter_Matrix::flatten(SkWriteBuffer& buffer) const {
    SkASSERT(sizeof(fMatrix)/sizeof(SkScalar) == 20);
    buffer.writeScalarArray(fMatrix, 20);
}

sk_sp<SkFlattenable> SkColorFilter_Matrix::CreateProc(SkReadBuffer& buffer) {

    SkScalar matrix[20];
    if (buffer.readScalarArray(matrix, 20)) {
        if (buffer.isVersionLT(SkReadBuffer::kColorMatrixColMajor_Version)) {
            SkScalar transposed[20];
            rowMajor_to_colMajor_div255(transposed, matrix);
            memcpy(matrix, transposed, sizeof(matrix));
        }
        return SkColorFilters::Matrix(matrix);
    }
    return nullptr;
}

bool SkColorFilter_Matrix::asColorMatrix(SkScalar matrix[20]) const {
    if (matrix) {
        // this virtual was defined when we were rowMajor, so transform into that
        colMajor_to_rowMajor_mul255(matrix, fMatrix);
    }
    return true;
}

bool SkColorFilter_Matrix::onAppendStages(const SkStageRec& rec,
                                                    bool shaderIsOpaque) const {
    const bool willStayOpaque = shaderIsOpaque && (fFlags & kAlphaUnchanged_Flag);

    SkRasterPipeline* p = rec.fPipeline;
    if (!shaderIsOpaque) { p->append(SkRasterPipeline::unpremul); }
    if (           true) { p->append(SkRasterPipeline::matrix_4x5, fMatrix); }
    if (           true) { p->append(SkRasterPipeline::clamp_0); }
    if (           true) { p->append(SkRasterPipeline::clamp_1); }
    if (!willStayOpaque) { p->append(SkRasterPipeline::premul); }
    return true;
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
            uniManager.setMatrix4fv(fMatrixHandle, 1, cme.fMatrix);
            uniManager.set4fv(fVectorHandle, 1, cme.fMatrix + 16);
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

std::unique_ptr<GrFragmentProcessor> SkColorFilter_Matrix::asFragmentProcessor(
        GrRecordingContext*, const GrColorSpaceInfo&) const {
    return ColorMatrixEffect::Make(fMatrix);
}

#endif

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkColorFilter> SkColorFilters::Matrix(const SkColorMatrix& cm) {
    return MatrixRowMajor255(cm.fMat);
}

sk_sp<SkColorFilter> SkColorFilters::MatrixRowMajor255(const float rowMajor[20]) {
    SkScalar array[20];
    rowMajor_to_colMajor_div255(array, rowMajor);
    return SkColorFilters::Matrix(array);
}

sk_sp<SkColorFilter> SkColorFilters::Matrix(const float array[20]) {
    if (!SkScalarsAreFinite(array, 20)) {
        return nullptr;
    }
    return sk_sp<SkColorFilter>(new SkColorFilter_Matrix(array));
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkColorFilter>
SkColorFilter_Matrix::MakeSingleChannelOutput(const SkScalar row[5]) {
    if (!SkScalarsAreFinite(row, 5)) {
        return nullptr;
    }

    SkScalar array[20];
    SkScalar* a = array;
    for (int i = 0; i < 4; ++i) {
        a[0] = a[1] = a[2] = a[3] = row[i];
        a += 4;
    }
    a[0] = a[1] = a[2] = a[3] = row[4] * (1.0f/255);

    return SkColorFilters::Matrix(array);
}
