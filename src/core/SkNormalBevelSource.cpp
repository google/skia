/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkNormalBevelSource.h"

#include "SkArenaAlloc.h"
#include "SkNormalSource.h"
#include "SkNormalSourcePriv.h"
#include "SkPoint3.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "SkGr.h"

/** \class NormalBevelFP
 *
 *  Fragment processor for the SkNormalBevelSource.
 *
 *  @param bevelType    type of the bevel
 *  @param bevelWidth   width of the bevel in device space
 *  @param bevelHeight  height of the bevel in device space
 */
class NormalBevelFP : public GrFragmentProcessor {
public:
    NormalBevelFP(SkNormalSource::BevelType bevelType, SkScalar bevelWidth, SkScalar bevelHeight)
            : INHERITED(kNone_OptimizationFlags)
            , fBevelType(bevelType)
            , fBevelWidth(bevelWidth)
            , fBevelHeight(bevelHeight) {
        this->initClassID<NormalBevelFP>();

        this->setWillUseDistanceVectorField();
    }

    class GLSLNormalBevelFP : public GLSLNormalFP {
    public:
        GLSLNormalBevelFP() {
            fPrevWidth = SkFloatToScalar(0.0f);
            fPrevHeight = SkFloatToScalar(0.0f);
        }

        void onEmitCode(EmitArgs& args) override {
            GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
            const NormalBevelFP& fp = args.fFp.cast<NormalBevelFP>();
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            // Determining necessary uniforms and initializing them
            bool needWidth = true;
            bool needHeight = (fp.fBevelType == SkNormalSource::BevelType::kRoundedOut ||
                               fp.fBevelType == SkNormalSource::BevelType::kRoundedIn);
            bool needNormalized = (fp.fBevelType == SkNormalSource::BevelType::kLinear);

            const char *widthUniName = nullptr;
            if (needWidth) {
                fWidthUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kFloat_GrSLType,
                                                       kDefault_GrSLPrecision, "Width",
                                                       &widthUniName);
            }

            const char* heightUniName = nullptr;
            if (needHeight) {
                fHeightUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kFloat_GrSLType,
                                                        kDefault_GrSLPrecision, "Height",
                                                        &heightUniName);
            }

            const char* normalizedWidthUniName = nullptr;
            const char* normalizedHeightUniName = nullptr;
            if (needNormalized) {
                fNormalizedWidthUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                                 kFloat_GrSLType,
                                                                 kDefault_GrSLPrecision,
                                                                 "NormalizedWidth",
                                                                 &normalizedWidthUniName);
                fNormalizedHeightUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                                  kFloat_GrSLType,
                                                                  kDefault_GrSLPrecision,
                                                                  "NormalizedHeight",
                                                                  &normalizedHeightUniName);
            }

            // Here we are splitting the distance vector into length and normalized direction
            fragBuilder->codeAppendf("float dv_length = %s.z;",
                                     fragBuilder->distanceVectorName());
            fragBuilder->codeAppendf("vec2 dv_norm = %s.xy;",
                                     fragBuilder->distanceVectorName());

            // Asserting presence of necessary uniforms
            SkASSERT(widthUniName);

            fragBuilder->codeAppend( "vec3 normal;");
            fragBuilder->codeAppendf("if (dv_length >= %s) {", widthUniName);
            fragBuilder->codeAppend( "    normal = vec3(0.0, 0.0, 1.0);");
            fragBuilder->codeAppend( "} else {");
            this->emitMath(fragBuilder, fp.fBevelType, widthUniName, heightUniName,
                           normalizedWidthUniName, normalizedHeightUniName);
            fragBuilder->codeAppend( "}");
            fragBuilder->codeAppendf("%s = vec4(normal, 0.0);", args.fOutputColor);
        }

        static void GenKey(const GrProcessor& proc, const GrShaderCaps&, GrProcessorKeyBuilder* b) {
            const NormalBevelFP& fp = proc.cast<NormalBevelFP>();
            b->add32(static_cast<int>(fp.fBevelType));
        }

    protected:
        void setNormalData(const GrGLSLProgramDataManager& pdman,
                           const GrFragmentProcessor& proc) override {
            const NormalBevelFP& normalBevelFP = proc.cast<NormalBevelFP>();

            // Updating uniform if bevel type requires it and data has changed

            bool needWidth = true;
            bool needHeight = (normalBevelFP.fBevelType == SkNormalSource::BevelType::kRoundedOut ||
                               normalBevelFP.fBevelType == SkNormalSource::BevelType::kRoundedIn);
            bool needNormalized = (normalBevelFP.fBevelType == SkNormalSource::BevelType::kLinear);

            bool dirtyWidth = (fPrevWidth  != normalBevelFP.fBevelWidth);
            bool dirtyHeight = (fPrevHeight != normalBevelFP.fBevelHeight);
            bool dirtyNormalized = (dirtyHeight || dirtyWidth);


            if (needWidth && dirtyWidth) {
                pdman.set1f(fWidthUni, normalBevelFP.fBevelWidth);
                fPrevWidth = normalBevelFP.fBevelWidth;
            }
            if (needHeight && dirtyHeight) {
                pdman.set1f(fHeightUni, normalBevelFP.fBevelHeight);
                fPrevHeight = normalBevelFP.fBevelHeight;
            }
            if (needNormalized && dirtyNormalized) {
                SkScalar height = normalBevelFP.fBevelHeight;
                SkScalar width  = normalBevelFP.fBevelWidth;

                SkScalar length = SkScalarSqrt(SkScalarSquare(height) + SkScalarSquare(width));
                pdman.set1f(fNormalizedHeightUni, height/length);
                pdman.set1f(fNormalizedWidthUni, width/length);
            }
        }

        // This method emits the code that calculates the normal orthgonal to the simulated beveled
        // surface. In the comments inside the function, the math involved is described. For this
        // purpose, the d-axis is defined to be the axis co-linear to the distance vector, where the
        // origin is the end of the bevel inside the shape.
        void emitMath(GrGLSLFPFragmentBuilder* fb, SkNormalSource::BevelType type,
                      const char* width, const char* height, const char* normalizedWidth,
                      const char* normalizedHeight) {
            switch (type) {
                case SkNormalSource::BevelType::kLinear:
                    // Asserting presence of necessary uniforms
                    SkASSERT(normalizedHeight);
                    SkASSERT(normalizedWidth);

                    // Because the slope of the bevel is -height/width, the vector
                    // normalized(vec2(height, width)) is the d- and z-components of the normal
                    // vector that is orthogonal to the linear bevel. Multiplying the d-component
                    // to the normalized distance vector splits it into x- and y-components.
                    fb->codeAppendf("normal = vec3(%s * dv_norm, %s);",
                                    normalizedHeight, normalizedWidth);
                    break;
                case SkNormalSource::BevelType::kRoundedOut:
                    // Fall through
                case SkNormalSource::BevelType::kRoundedIn:
                    // Asserting presence of necessary uniforms
                    SkASSERT(height);
                    SkASSERT(width);

                    // Setting the current position in the d-axis to the distance from the end of
                    // the bevel as opposed to the beginning if the bevel is rounded in, essentially
                    // flipping the bevel calculations.
                    if ( type == SkNormalSource::BevelType::kRoundedIn ) {
                        fb->codeAppendf("float currentPos_d = %s - dv_length;", width);
                    } else if (type == SkNormalSource::BevelType::kRoundedOut) {
                        fb->codeAppendf("float currentPos_d = dv_length;");
                    }

                    fb->codeAppendf("float rootDOverW = sqrt(currentPos_d/%s);", width);

                    // Calculating the d- and z-components of the normal, where 'd' is the axis
                    // co-linear to the distance vector. Equation was derived from the formula for
                    // a bezier curve by solving the parametric equation for d(t) and z(t), then
                    // with those, calculate d'(t), z'(t) and t(d), and from these, d'(d) and z'(d).
                    // z'(d)/d'(d) results in the slope of the bevel at d, so we construct an
                    // orthogonal vector of slope -d'(d)/z'(d) and length 1.
                    fb->codeAppendf("vec2 unnormalizedNormal_dz = vec2(%s*(1.0-rootDOverW), "
                                                                       "%s*rootDOverW);",
                                    height, width);
                    fb->codeAppendf("vec2 normal_dz = normalize(unnormalizedNormal_dz);");

                    // Multiplying the d-component to the normalized distance vector splits it into
                    // x- and y-components.
                    fb->codeAppendf("normal = vec3(normal_dz.x*dv_norm, normal_dz.y);");

                    break;
                default:
                    SkDEBUGFAIL("Invalid bevel type passed to emitMath");
            }
        }

    private:
        SkScalar fPrevWidth;
        GrGLSLProgramDataManager::UniformHandle fWidthUni;

        SkScalar fPrevHeight;
        GrGLSLProgramDataManager::UniformHandle fHeightUni;

        // width / length(<width,height>)
        GrGLSLProgramDataManager::UniformHandle fNormalizedWidthUni;
        // height / length(<width,height>)
        GrGLSLProgramDataManager::UniformHandle fNormalizedHeightUni;
    };

    void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {
        GLSLNormalBevelFP::GenKey(*this, caps, b);
    }

    const char* name() const override { return "NormalBevelFP"; }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override { return new GLSLNormalBevelFP; }

    bool onIsEqual(const GrFragmentProcessor& proc) const override {
        const NormalBevelFP& normalBevelFP = proc.cast<NormalBevelFP>();
        return fBevelType   == normalBevelFP.fBevelType &&
               fBevelWidth  == normalBevelFP.fBevelWidth &&
               fBevelHeight == normalBevelFP.fBevelHeight;
    }

    SkNormalSource::BevelType fBevelType;
    SkScalar fBevelWidth;
    SkScalar fBevelHeight;

    typedef GrFragmentProcessor INHERITED;
};

sk_sp<GrFragmentProcessor> SkNormalBevelSourceImpl::asFragmentProcessor(
        const SkShader::AsFPArgs& args) const {

    // This assumes a uniform scale. Anisotropic scaling might not be handled gracefully.
    SkScalar maxScale = args.fViewMatrix->getMaxScale();

    // Providing device-space width and height
    return sk_make_sp<NormalBevelFP>(fType, maxScale * fWidth, maxScale * fHeight);
}

#endif // SK_SUPPORT_GPU

////////////////////////////////////////////////////////////////////////////

SkNormalBevelSourceImpl::Provider::Provider() {}

SkNormalBevelSourceImpl::Provider::~Provider() {}

SkNormalSource::Provider* SkNormalBevelSourceImpl::asProvider(const SkShader::ContextRec &rec,
                                                              SkArenaAlloc* alloc) const {
    return alloc->make<Provider>();
}

// TODO Implement feature for the CPU pipeline
void SkNormalBevelSourceImpl::Provider::fillScanLine(int x, int y, SkPoint3 output[],
                                                     int count) const {
    for (int i = 0; i < count; i++) {
        output[i] = {0.0f, 0.0f, 1.0f};
    }
}

////////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkNormalBevelSourceImpl::CreateProc(SkReadBuffer& buf) {

    auto type = static_cast<SkNormalSource::BevelType>(buf.readInt());
    SkScalar width = buf.readScalar();
    SkScalar height = buf.readScalar();

    return sk_make_sp<SkNormalBevelSourceImpl>(type, width, height);
}

void SkNormalBevelSourceImpl::flatten(SkWriteBuffer& buf) const {
    this->INHERITED::flatten(buf);

    buf.writeInt(static_cast<int>(fType));
    buf.writeScalar(fWidth);
    buf.writeScalar(fHeight);
}

////////////////////////////////////////////////////////////////////////////

sk_sp<SkNormalSource> SkNormalSource::MakeBevel(BevelType type, SkScalar width, SkScalar height) {
    /* TODO make sure these checks are tolerant enough to account for loss of conversion when GPUs
       use 16-bit float types. We don't want to assume stuff is non-zero on the GPU and be wrong.*/
    SkASSERT(width > 0.0f && !SkScalarNearlyZero(width));
    if (SkScalarNearlyZero(height)) {
        return SkNormalSource::MakeFlat();
    }

    return sk_make_sp<SkNormalBevelSourceImpl>(type, width, height);
}
