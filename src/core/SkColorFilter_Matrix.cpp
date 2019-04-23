/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkUnPreMultiply.h"
#include "include/effects/SkColorMatrix.h"
#include "include/private/SkColorData.h"
#include "include/private/SkNx.h"
#include "src/core/SkColorFilter_Matrix.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

static void scale_last_column(float rowMajor[20], float scale) {
    for (int i = 0; i < 4; ++i) {
        rowMajor[5*i + 4] *= scale;
    }
}

void SkColorFilter_Matrix::initState() {
    const float* srcA = fMatrix + 15;
    fFlags = (srcA[0] == 0 && srcA[1] == 0 && srcA[2] == 0 && srcA[3] == 1 && srcA[4] == 0)
           ? kAlphaUnchanged_Flag : 0;
}

SkColorFilter_Matrix::SkColorFilter_Matrix(const float array[20]) {
    memcpy(fMatrix, array, 20 * sizeof(float));
    this->initState();
}

uint32_t SkColorFilter_Matrix::getFlags() const {
    return this->INHERITED::getFlags() | fFlags;
}

void SkColorFilter_Matrix::flatten(SkWriteBuffer& buffer) const {
    SkASSERT(sizeof(fMatrix)/sizeof(float) == 20);
    buffer.writeScalarArray(fMatrix, 20);
}

sk_sp<SkFlattenable> SkColorFilter_Matrix::CreateProc(SkReadBuffer& buffer) {
    float matrix[20];
    if (buffer.readScalarArray(matrix, 20)) {
        return SkColorFilters::Matrix(matrix);
    }
    return nullptr;
}

bool SkColorFilter_Matrix::asColorMatrix(float matrix[20]) const {
    if (matrix) {
        memcpy(matrix, fMatrix, 20 * sizeof(float));
        scale_last_column(matrix, 255);
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
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

class ColorMatrixEffect : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(const float matrix[20]) {
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
            float vec[] = {
                m[4], m[9], m[14], m[19],
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
    ColorMatrixEffect(const float matrix[20])
    : INHERITED(kColorMatrixEffect_ClassID, kNone_OptimizationFlags) {
        memcpy(fMatrix, matrix, sizeof(float) * 20);
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
std::unique_ptr<GrFragmentProcessor> ColorMatrixEffect::TestCreate(GrProcessorTestData* d) {
    float colorMatrix[20];
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

sk_sp<SkColorFilter> SkColorFilters::Matrix(const float array[20]) {
    if (!sk_floats_are_finite(array, 20)) {
        return nullptr;
    }
    return sk_sp<SkColorFilter>(new SkColorFilter_Matrix(array));
}

sk_sp<SkColorFilter> SkColorFilters::Matrix(const SkColorMatrix& cm) {
    return Matrix(cm.fMat);
}

// DEPRECATED
sk_sp<SkColorFilter> SkColorFilters::MatrixRowMajor255(const float array[20]) {
    float tmp[20];
    memcpy(tmp, array, sizeof(tmp));
    scale_last_column(tmp, 1.0f/255);
    return Matrix(tmp);
}

void SkColorFilter_Matrix::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkColorFilter_Matrix);

    // This subclass was removed 4/2019
    SkFlattenable::Register("SkColorMatrixFilterRowMajor255",
                            [](SkReadBuffer& buffer) -> sk_sp<SkFlattenable> {
        float matrix[20];
        if (buffer.readScalarArray(matrix, 20)) {
            scale_last_column(matrix, 1.0f/255);
            return SkColorFilters::Matrix(matrix);
        }
        return nullptr;
    });
}
