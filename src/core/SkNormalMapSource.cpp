/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkNormalMapSource.h"

#include "include/core/SkMatrix.h"
#include "include/private/SkArenaAlloc.h"
#include "src/core/SkNormalSource.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkLightingShader.h"

#if SK_SUPPORT_GPU
#include "src/gpu/GrCoordTransform.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"

class NormalMapFP : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> mapFP,
                                                     const SkMatrix& invCTM) {
        return std::unique_ptr<GrFragmentProcessor>(new NormalMapFP(std::move(mapFP), invCTM));
    }

    const char* name() const override { return "NormalMapFP"; }

    const SkMatrix& invCTM() const { return fInvCTM; }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return Make(this->childProcessor(0).clone(), fInvCTM);
    }

private:
    class GLSLNormalMapFP : public GrGLSLFragmentProcessor {
    public:
        GLSLNormalMapFP() : fColumnMajorInvCTM22{0.0f} {}

        void emitCode(EmitArgs& args) override {
            GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            // add uniform
            const char* xformUniName = nullptr;
            fXformUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kFloat2x2_GrSLType,
                                                   "Xform", &xformUniName);

            SkString dstNormalColorName("dstNormalColor");
            this->emitChild(0, &dstNormalColorName, args);
            fragBuilder->codeAppendf("float3 normal = normalize(%s.rgb - float3(0.5));",
                                     dstNormalColorName.c_str());

            // If there's no x & y components, return (0, 0, +/- 1) instead to avoid division by 0
            fragBuilder->codeAppend( "if (abs(normal.z) > 0.999) {");
            fragBuilder->codeAppendf("    %s = normalize(half4(0.0, 0.0, half(normal.z), 0.0));",
                    args.fOutputColor);
            // Else, Normalizing the transformed X and Y, while keeping constant both Z and the
            // vector's angle in the XY plane. This maintains the "slope" for the surface while
            // appropriately rotating the normal regardless of any anisotropic scaling that occurs.
            // Here, we call 'scaling factor' the number that must divide the transformed X and Y so
            // that the normal's length remains equal to 1.
            fragBuilder->codeAppend( "} else {");
            fragBuilder->codeAppendf("    float2 transformed = %s * normal.xy;",
                    xformUniName);
            fragBuilder->codeAppend( "    float scalingFactorSquared = "
                                                 "( (transformed.x * transformed.x) "
                                                   "+ (transformed.y * transformed.y) )"
                                                 "/(1.0 - (normal.z * normal.z));");
            fragBuilder->codeAppendf("    %s = half4(half2(transformed * "
                                                          "inversesqrt(scalingFactorSquared)),"
                                                    "half(normal.z), 0.0);",
                    args.fOutputColor);
            fragBuilder->codeAppend( "}");
        }

        static void GenKey(const GrProcessor&, const GrShaderCaps&, GrProcessorKeyBuilder* b) {
            b->add32(0x0);
        }

    private:
        void onSetData(const GrGLSLProgramDataManager& pdman,
                       const GrFragmentProcessor& proc) override {
            const NormalMapFP& normalMapFP = proc.cast<NormalMapFP>();

            const SkMatrix& invCTM = normalMapFP.invCTM();
            fColumnMajorInvCTM22[0] = invCTM.get(SkMatrix::kMScaleX);
            fColumnMajorInvCTM22[1] = invCTM.get(SkMatrix::kMSkewY);
            fColumnMajorInvCTM22[2] = invCTM.get(SkMatrix::kMSkewX);
            fColumnMajorInvCTM22[3] = invCTM.get(SkMatrix::kMScaleY);
            pdman.setMatrix2f(fXformUni, fColumnMajorInvCTM22);
        }

    private:
        // Upper-right 2x2 corner of the inverse of the CTM in column-major form
        float fColumnMajorInvCTM22[4];
        GrGLSLProgramDataManager::UniformHandle fXformUni;
    };

    void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {
        GLSLNormalMapFP::GenKey(*this, caps, b);
    }
    NormalMapFP(std::unique_ptr<GrFragmentProcessor> mapFP, const SkMatrix& invCTM)
            : INHERITED(kMappedNormalsFP_ClassID, kNone_OptimizationFlags)
            , fInvCTM(invCTM) {
        this->registerChildProcessor(std::move(mapFP));
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override { return new GLSLNormalMapFP; }

    bool onIsEqual(const GrFragmentProcessor& proc) const override {
        const NormalMapFP& normalMapFP = proc.cast<NormalMapFP>();
        return fInvCTM == normalMapFP.fInvCTM;
    }

    SkMatrix fInvCTM;

    typedef GrFragmentProcessor INHERITED;
};

std::unique_ptr<GrFragmentProcessor> SkNormalMapSourceImpl::asFragmentProcessor(
                                                                       const GrFPArgs& args) const {
    std::unique_ptr<GrFragmentProcessor> mapFP = as_SB(fMapShader)->asFragmentProcessor(args);
    if (!mapFP) {
        return nullptr;
    }

    return NormalMapFP::Make(std::move(mapFP), fInvCTM);
}

#endif // SK_SUPPORT_GPU

////////////////////////////////////////////////////////////////////////////

SkNormalMapSourceImpl::Provider::Provider(const SkNormalMapSourceImpl& source,
                                          SkShaderBase::Context* mapContext)
    : fSource(source)
    , fMapContext(mapContext) {}

SkNormalSource::Provider* SkNormalMapSourceImpl::asProvider(const SkShaderBase::ContextRec &rec,
                                                            SkArenaAlloc* alloc) const {
    SkMatrix normTotalInv;
    if (!this->computeNormTotalInverse(rec, &normTotalInv)) {
        return nullptr;
    }

    // Normals really aren't colors, so to ensure we can always make the context, we ignore
    // the rec's colorspace
    SkColorSpace* dstColorSpace = nullptr;

    // Overriding paint's alpha because we need the normal map's RGB channels to be unpremul'd
    SkPaint overridePaint {*(rec.fPaint)};
    overridePaint.setAlpha(0xFF);
    SkShaderBase::ContextRec overrideRec(overridePaint, *(rec.fMatrix), rec.fLocalMatrix,
                                         rec.fDstColorType, dstColorSpace);

    auto* context = as_SB(fMapShader)->makeContext(overrideRec, alloc);
    if (!context) {
        return nullptr;
    }

    return alloc->make<Provider>(*this, context);
}

bool SkNormalMapSourceImpl::computeNormTotalInverse(const SkShaderBase::ContextRec& rec,
                                                    SkMatrix* normTotalInverse) const {
    SkMatrix total = SkMatrix::Concat(*rec.fMatrix, as_SB(fMapShader)->getLocalMatrix());
    if (rec.fLocalMatrix) {
        total.preConcat(*rec.fLocalMatrix);
    }

    return total.invert(normTotalInverse);
}

#define BUFFER_MAX 16
void SkNormalMapSourceImpl::Provider::fillScanLine(int x, int y, SkPoint3 output[],
                                                   int count) const {
    SkPMColor tmpNormalColors[BUFFER_MAX];

    do {
        int n = SkTMin(count, BUFFER_MAX);

        fMapContext->shadeSpan(x, y, tmpNormalColors, n);

        for (int i = 0; i < n; i++) {
            SkPoint3 tempNorm;

            tempNorm.set(SkIntToScalar(SkGetPackedR32(tmpNormalColors[i])) - 127.0f,
                         SkIntToScalar(SkGetPackedG32(tmpNormalColors[i])) - 127.0f,
                         SkIntToScalar(SkGetPackedB32(tmpNormalColors[i])) - 127.0f);

            tempNorm.normalize();


            if (!SkScalarNearlyEqual(SkScalarAbs(tempNorm.fZ), 1.0f)) {
                SkVector transformed = fSource.fInvCTM.mapVector(tempNorm.fX, tempNorm.fY);

                // Normalizing the transformed X and Y, while keeping constant both Z and the
                // vector's angle in the XY plane. This maintains the "slope" for the surface while
                // appropriately rotating the normal for any anisotropic scaling that occurs.
                // Here, we call scaling factor the number that must divide the transformed X and Y
                // so that the normal's length remains equal to 1.
                SkScalar scalingFactorSquared =
                        (SkScalarSquare(transformed.fX) + SkScalarSquare(transformed.fY))
                        / (1.0f - SkScalarSquare(tempNorm.fZ));
                SkScalar invScalingFactor = SkScalarInvert(SkScalarSqrt(scalingFactorSquared));

                output[i].fX = transformed.fX * invScalingFactor;
                output[i].fY = transformed.fY * invScalingFactor;
                output[i].fZ = tempNorm.fZ;
            } else {
                output[i] = {0.0f, 0.0f, tempNorm.fZ};
                output[i].normalize();
            }

            SkASSERT(SkScalarNearlyEqual(output[i].length(), 1.0f));
        }

        output += n;
        x += n;
        count -= n;
    } while (count > 0);
}

////////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkNormalMapSourceImpl::CreateProc(SkReadBuffer& buf) {

    sk_sp<SkShader> mapShader = buf.readFlattenable<SkShaderBase>();

    SkMatrix invCTM;
    buf.readMatrix(&invCTM);

    return sk_make_sp<SkNormalMapSourceImpl>(std::move(mapShader), invCTM);
}

void SkNormalMapSourceImpl::flatten(SkWriteBuffer& buf) const {
    this->INHERITED::flatten(buf);

    buf.writeFlattenable(fMapShader.get());
    buf.writeMatrix(fInvCTM);
}

////////////////////////////////////////////////////////////////////////////

sk_sp<SkNormalSource> SkNormalSource::MakeFromNormalMap(sk_sp<SkShader> map, const SkMatrix& ctm) {
    SkMatrix invCTM;

    if (!ctm.invert(&invCTM) || !map) {
        return nullptr;
    }

    return sk_make_sp<SkNormalMapSourceImpl>(std::move(map), invCTM);
}
