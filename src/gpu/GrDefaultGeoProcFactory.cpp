/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDefaultGeoProcFactory.h"

#include "GrCaps.h"
#include "SkRefCnt.h"
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLUtil.h"

/*
 * The default Geometry Processor simply takes position and multiplies it by the uniform view
 * matrix. It also leaves coverage untouched.  Behind the scenes, we may add per vertex color or
 * local coords.
 */

enum GPFlag {
    kColorAttribute_GPFlag          = 0x1,
    kColorAttributeIsSkColor_GPFlag = 0x2,
    kLocalCoordAttribute_GPFlag     = 0x4,
    kCoverageAttribute_GPFlag       = 0x8,
    kBonesAttribute_GPFlag          = 0x10,
};

static constexpr int kMaxBones = 80; // Due to GPU memory limitations, only up to 80 bone
                                     // matrices are accepted.

class DefaultGeoProc : public GrGeometryProcessor {
public:
    static sk_sp<GrGeometryProcessor> Make(const GrShaderCaps* shaderCaps,
                                           uint32_t gpTypeFlags,
                                           GrColor color,
                                           sk_sp<GrColorSpaceXform> colorSpaceXform,
                                           const SkMatrix& viewMatrix,
                                           const SkMatrix& localMatrix,
                                           bool localCoordsWillBeRead,
                                           uint8_t coverage,
                                           const float* bones,
                                           int boneCount) {
        return sk_sp<GrGeometryProcessor>(new DefaultGeoProc(
                shaderCaps, gpTypeFlags, color, std::move(colorSpaceXform), viewMatrix, localMatrix,
                coverage, localCoordsWillBeRead, bones, boneCount));
    }

    const char* name() const override { return "DefaultGeometryProcessor"; }

    GrColor color() const { return fColor; }
    bool hasVertexColor() const { return fInColor.isInitialized(); }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    const SkMatrix& localMatrix() const { return fLocalMatrix; }
    bool localCoordsWillBeRead() const { return fLocalCoordsWillBeRead; }
    uint8_t coverage() const { return fCoverage; }
    bool hasVertexCoverage() const { return fInCoverage.isInitialized(); }
    const float* bones() const { return fBones; }
    int boneCount() const { return fBoneCount; }
    bool hasBones() const { return SkToBool(fBones); }

    class GLSLProcessor : public GrGLSLGeometryProcessor {
    public:
        GLSLProcessor()
            : fViewMatrix(SkMatrix::InvalidMatrix()), fColor(GrColor_ILLEGAL), fCoverage(0xff) {}

        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
            const DefaultGeoProc& gp = args.fGP.cast<DefaultGeoProc>();
            GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
            GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
            GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            // emit attributes
            varyingHandler->emitAttributes(gp);

            // Setup pass through color
            if (gp.hasVertexColor()) {
                GrGLSLVarying varying(kHalf4_GrSLType);
                varyingHandler->addVarying("color", &varying);

                // There are several optional steps to process the color. Start with the attribute:
                vertBuilder->codeAppendf("half4 color = %s;", gp.fInColor.name());

                // For SkColor, do a red/blue swap, possible color space conversion, and premul
                if (gp.fFlags & kColorAttributeIsSkColor_GPFlag) {
                    vertBuilder->codeAppend("color = color.bgra;");

                    if (gp.fColorSpaceXform) {
                        fColorSpaceHelper.emitCode(uniformHandler, gp.fColorSpaceXform.get(),
                                                   kVertex_GrShaderFlag);
                        SkString xformedColor;
                        vertBuilder->appendColorGamutXform(&xformedColor, "color",
                                                           &fColorSpaceHelper);
                        vertBuilder->codeAppendf("color = %s;", xformedColor.c_str());
                    }

                    vertBuilder->codeAppend("color = half4(color.rgb * color.a, color.a);");
                }

                vertBuilder->codeAppendf("%s = color;\n", varying.vsOut());
                fragBuilder->codeAppendf("%s = %s;", args.fOutputColor, varying.fsIn());
            } else {
                this->setupUniformColor(fragBuilder, uniformHandler, args.fOutputColor,
                                        &fColorUniform);
            }

            // Setup bone transforms
            const char* transformedPositionName = gp.fInPosition.name();
            if (gp.hasBones()) {
                const char* vertBonesUniformName;
                fBonesUniform = uniformHandler->addUniformArray(kVertex_GrShaderFlag,
                                                                kFloat3x3_GrSLType,
                                                                "Bones",
                                                                kMaxBones,
                                                                &vertBonesUniformName);
                vertBuilder->codeAppendf(
                        "float3 originalPosition = %s[0] * float3(%s, 1);"
                        "float2 transformedPosition = float2(0);"
                        "for (int i = 0; i < 4; i++) {",
                        vertBonesUniformName,
                        gp.fInPosition.name());

                if (args.fShaderCaps->unsignedSupport()) {
                    vertBuilder->codeAppendf(
                        "    byte index = %s[i];",
                        gp.fInBoneIndices.name());
                } else {
                    vertBuilder->codeAppendf(
                        "    byte index = byte(floor(%s[i] * 255 + 0.5));",
                        gp.fInBoneIndices.name());
                }

                vertBuilder->codeAppendf(
                        "    float weight = %s[i];"
                        "    transformedPosition += (%s[index] * originalPosition * weight).xy;"
                        "}",
                        gp.fInBoneWeights.name(),
                        vertBonesUniformName);
                transformedPositionName = "transformedPosition";
            }

            // Setup position
            this->writeOutputPosition(vertBuilder,
                                      uniformHandler,
                                      gpArgs,
                                      transformedPositionName,
                                      gp.viewMatrix(),
                                      &fViewMatrixUniform);

            if (gp.fInLocalCoords.isInitialized()) {
                // emit transforms with explicit local coords
                this->emitTransforms(vertBuilder,
                                     varyingHandler,
                                     uniformHandler,
                                     gp.fInLocalCoords.asShaderVar(),
                                     gp.localMatrix(),
                                     args.fFPCoordTransformHandler);
            } else {
                // emit transforms with position
                this->emitTransforms(vertBuilder,
                                     varyingHandler,
                                     uniformHandler,
                                     gp.fInPosition.asShaderVar(),
                                     gp.localMatrix(),
                                     args.fFPCoordTransformHandler);
            }

            // Setup coverage as pass through
            if (gp.hasVertexCoverage()) {
                fragBuilder->codeAppendf("half alpha = 1.0;");
                varyingHandler->addPassThroughAttribute(gp.fInCoverage, "alpha");
                fragBuilder->codeAppendf("%s = half4(alpha);", args.fOutputCoverage);
            } else if (gp.coverage() == 0xff) {
                fragBuilder->codeAppendf("%s = half4(1);", args.fOutputCoverage);
            } else {
                const char* fragCoverage;
                fCoverageUniform = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                              kHalf_GrSLType,
                                                              "Coverage",
                                                              &fragCoverage);
                fragBuilder->codeAppendf("%s = half4(%s);", args.fOutputCoverage, fragCoverage);
            }
        }

        static inline void GenKey(const GrGeometryProcessor& gp,
                                  const GrShaderCaps&,
                                  GrProcessorKeyBuilder* b) {
            const DefaultGeoProc& def = gp.cast<DefaultGeoProc>();
            uint32_t key = def.fFlags;
            key |= (def.coverage() == 0xff) ? 0x20 : 0;
            key |= (def.localCoordsWillBeRead() && def.localMatrix().hasPerspective()) ? 0x40 : 0x0;
            key |= ComputePosKey(def.viewMatrix()) << 20;
            b->add32(key);
            b->add32(GrColorSpaceXform::XformKey(def.fColorSpaceXform.get()));
        }

        void setData(const GrGLSLProgramDataManager& pdman,
                     const GrPrimitiveProcessor& gp,
                     FPCoordTransformIter&& transformIter) override {
            const DefaultGeoProc& dgp = gp.cast<DefaultGeoProc>();

            if (!dgp.viewMatrix().isIdentity() && !fViewMatrix.cheapEqualTo(dgp.viewMatrix())) {
                fViewMatrix = dgp.viewMatrix();
                float viewMatrix[3 * 3];
                GrGLSLGetMatrix<3>(viewMatrix, fViewMatrix);
                pdman.setMatrix3f(fViewMatrixUniform, viewMatrix);
            }

            if (dgp.color() != fColor && !dgp.hasVertexColor()) {
                float c[4];
                GrColorToRGBAFloat(dgp.color(), c);
                pdman.set4fv(fColorUniform, 1, c);
                fColor = dgp.color();
            }

            if (dgp.coverage() != fCoverage && !dgp.hasVertexCoverage()) {
                pdman.set1f(fCoverageUniform, GrNormalizeByteToFloat(dgp.coverage()));
                fCoverage = dgp.coverage();
            }
            this->setTransformDataHelper(dgp.fLocalMatrix, pdman, &transformIter);

            fColorSpaceHelper.setData(pdman, dgp.fColorSpaceXform.get());

            if (dgp.hasBones()) {
                pdman.setMatrix3fv(fBonesUniform, dgp.boneCount(), dgp.bones());
            }
        }

    private:
        SkMatrix fViewMatrix;
        GrColor fColor;
        uint8_t fCoverage;
        UniformHandle fViewMatrixUniform;
        UniformHandle fColorUniform;
        UniformHandle fCoverageUniform;
        UniformHandle fBonesUniform;
        GrGLSLColorSpaceXformHelper fColorSpaceHelper;

        typedef GrGLSLGeometryProcessor INHERITED;
    };

    void getGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {
        GLSLProcessor::GenKey(*this, caps, b);
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override {
        return new GLSLProcessor();
    }

private:
    DefaultGeoProc(const GrShaderCaps* shaderCaps,
                   uint32_t gpTypeFlags,
                   GrColor color,
                   sk_sp<GrColorSpaceXform> colorSpaceXform,
                   const SkMatrix& viewMatrix,
                   const SkMatrix& localMatrix,
                   uint8_t coverage,
                   bool localCoordsWillBeRead,
                   const float* bones,
                   int boneCount)
            : INHERITED(kDefaultGeoProc_ClassID)
            , fColor(color)
            , fViewMatrix(viewMatrix)
            , fLocalMatrix(localMatrix)
            , fCoverage(coverage)
            , fFlags(gpTypeFlags)
            , fLocalCoordsWillBeRead(localCoordsWillBeRead)
            , fColorSpaceXform(std::move(colorSpaceXform))
            , fBones(bones)
            , fBoneCount(boneCount) {
        fInPosition = {"inPosition", kFloat2_GrVertexAttribType};
        int cnt = 1;
        if (fFlags & kColorAttribute_GPFlag) {
            fInColor = {"inColor", kUByte4_norm_GrVertexAttribType};
            ++cnt;
        }
        if (fFlags & kLocalCoordAttribute_GPFlag) {
            fInLocalCoords = {"inLocalCoord", kFloat2_GrVertexAttribType};
            ++cnt;
        }
        if (fFlags & kCoverageAttribute_GPFlag) {
            fInCoverage = {"inCoverage", kHalf_GrVertexAttribType};
            ++cnt;
        }
        if (fFlags & kBonesAttribute_GPFlag) {
            SkASSERT(bones && (boneCount > 0));
            // GLSL 1.10 and 1.20 don't support integer attributes.
            GrVertexAttribType indicesAttribType =
                    shaderCaps->unsignedSupport() ? kByte4_GrVertexAttribType :
                                                    kUByte4_norm_GrVertexAttribType;
            fInBoneIndices = {"inBoneIndices", indicesAttribType};
            ++cnt;
            fInBoneWeights = {"inBoneWeights", kUByte4_norm_GrVertexAttribType};
            ++cnt;
        }
        this->setVertexAttributeCnt(cnt);
    }

    const Attribute& onVertexAttribute(int i) const override {
        return IthInitializedAttribute(i,
                                       fInPosition,
                                       fInColor,
                                       fInLocalCoords,
                                       fInCoverage,
                                       fInBoneIndices,
                                       fInBoneWeights);
    }

    Attribute fInPosition;
    Attribute fInColor;
    Attribute fInLocalCoords;
    Attribute fInCoverage;
    Attribute fInBoneIndices;
    Attribute fInBoneWeights;
    GrColor fColor;
    SkMatrix fViewMatrix;
    SkMatrix fLocalMatrix;
    uint8_t fCoverage;
    uint32_t fFlags;
    bool fLocalCoordsWillBeRead;
    sk_sp<GrColorSpaceXform> fColorSpaceXform;
    const float* fBones;
    int fBoneCount;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST

    typedef GrGeometryProcessor INHERITED;
};

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(DefaultGeoProc);

#if GR_TEST_UTILS
static constexpr int kNumFloatsPerSkMatrix = 9;
static constexpr int kTestBoneCount = 4;
static constexpr float kTestBones[kTestBoneCount * kNumFloatsPerSkMatrix] = {
    1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
};

sk_sp<GrGeometryProcessor> DefaultGeoProc::TestCreate(GrProcessorTestData* d) {
    uint32_t flags = 0;
    if (d->fRandom->nextBool()) {
        flags |= kColorAttribute_GPFlag;
    }
    if (d->fRandom->nextBool()) {
        flags |= kColorAttributeIsSkColor_GPFlag;
    }
    if (d->fRandom->nextBool()) {
        flags |= kCoverageAttribute_GPFlag;
    }
    if (d->fRandom->nextBool()) {
        flags |= kLocalCoordAttribute_GPFlag;
    }
    if (d->fRandom->nextBool()) {
        flags |= kBonesAttribute_GPFlag;
    }

    return DefaultGeoProc::Make(d->caps()->shaderCaps(),
                                flags,
                                GrRandomColor(d->fRandom),
                                GrTest::TestColorXform(d->fRandom),
                                GrTest::TestMatrix(d->fRandom),
                                GrTest::TestMatrix(d->fRandom),
                                d->fRandom->nextBool(),
                                GrRandomCoverage(d->fRandom),
                                kTestBones,
                                kTestBoneCount);
}
#endif

sk_sp<GrGeometryProcessor> GrDefaultGeoProcFactory::Make(const GrShaderCaps* shaderCaps,
                                                         const Color& color,
                                                         const Coverage& coverage,
                                                         const LocalCoords& localCoords,
                                                         const SkMatrix& viewMatrix) {
    uint32_t flags = 0;
    if (Color::kPremulGrColorAttribute_Type == color.fType) {
        flags |= kColorAttribute_GPFlag;
    } else if (Color::kUnpremulSkColorAttribute_Type == color.fType) {
        flags |= kColorAttribute_GPFlag | kColorAttributeIsSkColor_GPFlag;
    }
    flags |= coverage.fType == Coverage::kAttribute_Type ? kCoverageAttribute_GPFlag : 0;
    flags |= localCoords.fType == LocalCoords::kHasExplicit_Type ? kLocalCoordAttribute_GPFlag : 0;

    uint8_t inCoverage = coverage.fCoverage;
    bool localCoordsWillBeRead = localCoords.fType != LocalCoords::kUnused_Type;

    GrColor inColor = color.fColor;
    return DefaultGeoProc::Make(shaderCaps,
                                flags,
                                inColor,
                                color.fColorSpaceXform,
                                viewMatrix,
                                localCoords.fMatrix ? *localCoords.fMatrix : SkMatrix::I(),
                                localCoordsWillBeRead,
                                inCoverage,
                                nullptr,
                                0);
}

sk_sp<GrGeometryProcessor> GrDefaultGeoProcFactory::MakeForDeviceSpace(
                                                                     const GrShaderCaps* shaderCaps,
                                                                     const Color& color,
                                                                     const Coverage& coverage,
                                                                     const LocalCoords& localCoords,
                                                                     const SkMatrix& viewMatrix) {
    SkMatrix invert = SkMatrix::I();
    if (LocalCoords::kUnused_Type != localCoords.fType) {
        SkASSERT(LocalCoords::kUsePosition_Type == localCoords.fType);
        if (!viewMatrix.isIdentity() && !viewMatrix.invert(&invert)) {
            return nullptr;
        }

        if (localCoords.hasLocalMatrix()) {
            invert.preConcat(*localCoords.fMatrix);
        }
    }

    LocalCoords inverted(LocalCoords::kUsePosition_Type, &invert);
    return Make(shaderCaps, color, coverage, inverted, SkMatrix::I());
}

sk_sp<GrGeometryProcessor> GrDefaultGeoProcFactory::MakeWithBones(const GrShaderCaps* shaderCaps,
                                                                  const Color& color,
                                                                  const Coverage& coverage,
                                                                  const LocalCoords& localCoords,
                                                                  const Bones& bones,
                                                                  const SkMatrix& viewMatrix) {
    uint32_t flags = 0;
    if (Color::kPremulGrColorAttribute_Type == color.fType) {
        flags |= kColorAttribute_GPFlag;
    } else if (Color::kUnpremulSkColorAttribute_Type == color.fType) {
        flags |= kColorAttribute_GPFlag | kColorAttributeIsSkColor_GPFlag;
    }
    flags |= coverage.fType == Coverage::kAttribute_Type ? kCoverageAttribute_GPFlag : 0;
    flags |= localCoords.fType == LocalCoords::kHasExplicit_Type ? kLocalCoordAttribute_GPFlag : 0;
    flags |= kBonesAttribute_GPFlag;

    uint8_t inCoverage = coverage.fCoverage;
    bool localCoordsWillBeRead = localCoords.fType != LocalCoords::kUnused_Type;

    GrColor inColor = color.fColor;
    return DefaultGeoProc::Make(shaderCaps,
                                flags,
                                inColor,
                                color.fColorSpaceXform,
                                viewMatrix,
                                localCoords.fMatrix ? *localCoords.fMatrix : SkMatrix::I(),
                                localCoordsWillBeRead,
                                inCoverage,
                                bones.fBones,
                                bones.fBoneCount);
}
