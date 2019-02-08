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
    kColorAttributeIsWide_GPFlag    = 0x4,
    kLocalCoordAttribute_GPFlag     = 0x8,
    kCoverageAttribute_GPFlag       = 0x10,
    kCoverageAttributeTweak_GPFlag  = 0x20,
};

class DefaultGeoProc : public GrGeometryProcessor {
public:
    static sk_sp<GrGeometryProcessor> Make(const GrShaderCaps* shaderCaps,
                                           uint32_t gpTypeFlags,
                                           const SkPMColor4f& color,
                                           sk_sp<GrColorSpaceXform> colorSpaceXform,
                                           const SkMatrix& viewMatrix,
                                           const SkMatrix& localMatrix,
                                           bool localCoordsWillBeRead,
                                           uint8_t coverage) {
        return sk_sp<GrGeometryProcessor>(new DefaultGeoProc(
                shaderCaps, gpTypeFlags, color, std::move(colorSpaceXform), viewMatrix, localMatrix,
                coverage, localCoordsWillBeRead));
    }

    const char* name() const override { return "DefaultGeometryProcessor"; }

    const SkPMColor4f& color() const { return fColor; }
    bool hasVertexColor() const { return fInColor.isInitialized(); }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    const SkMatrix& localMatrix() const { return fLocalMatrix; }
    bool localCoordsWillBeRead() const { return fLocalCoordsWillBeRead; }
    uint8_t coverage() const { return fCoverage; }
    bool hasVertexCoverage() const { return fInCoverage.isInitialized(); }

    class GLSLProcessor : public GrGLSLGeometryProcessor {
    public:
        GLSLProcessor()
            : fViewMatrix(SkMatrix::InvalidMatrix())
            , fColor(SK_PMColor4fILLEGAL)
            , fCoverage(0xff) {}

        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
            const DefaultGeoProc& gp = args.fGP.cast<DefaultGeoProc>();
            GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
            GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
            GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            // emit attributes
            varyingHandler->emitAttributes(gp);

            bool tweakAlpha = SkToBool(gp.fFlags & kCoverageAttributeTweak_GPFlag);
            SkASSERT(!tweakAlpha || gp.hasVertexCoverage());

            // Setup pass through color
            if (gp.hasVertexColor() || tweakAlpha) {
                GrGLSLVarying varying(kHalf4_GrSLType);
                varyingHandler->addVarying("color", &varying);

                // There are several optional steps to process the color. Start with the attribute,
                // or with uniform color (in the case of folding coverage into a uniform color):
                if (gp.hasVertexColor()) {
                    vertBuilder->codeAppendf("half4 color = %s;", gp.fInColor.name());
                } else {
                    const char* colorUniformName;
                    fColorUniform = uniformHandler->addUniform(kVertex_GrShaderFlag,
                                                               kHalf4_GrSLType,
                                                               "Color",
                                                               &colorUniformName);
                    vertBuilder->codeAppendf("half4 color = %s;", colorUniformName);
                }

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

                // Optionally fold coverage into alpha (color).
                if (tweakAlpha) {
                    vertBuilder->codeAppendf("color = color * %s;", gp.fInCoverage.name());
                }
                vertBuilder->codeAppendf("%s = color;\n", varying.vsOut());
                fragBuilder->codeAppendf("%s = %s;", args.fOutputColor, varying.fsIn());
            } else {
                this->setupUniformColor(fragBuilder, uniformHandler, args.fOutputColor,
                                        &fColorUniform);
            }

            // Setup position
            this->writeOutputPosition(vertBuilder,
                                      uniformHandler,
                                      gpArgs,
                                      gp.fInPosition.name(),
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
            if (gp.hasVertexCoverage() && !tweakAlpha) {
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
            key |= (def.coverage() == 0xff) ? 0x80 : 0;
            key |= (def.localCoordsWillBeRead() && def.localMatrix().hasPerspective()) ? 0x100 : 0;
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

            if (!dgp.hasVertexColor() && dgp.color() != fColor) {
                pdman.set4fv(fColorUniform, 1, dgp.color().vec());
                fColor = dgp.color();
            }

            if (dgp.coverage() != fCoverage && !dgp.hasVertexCoverage()) {
                pdman.set1f(fCoverageUniform, GrNormalizeByteToFloat(dgp.coverage()));
                fCoverage = dgp.coverage();
            }
            this->setTransformDataHelper(dgp.fLocalMatrix, pdman, &transformIter);

            fColorSpaceHelper.setData(pdman, dgp.fColorSpaceXform.get());
        }

    private:
        SkMatrix fViewMatrix;
        SkPMColor4f fColor;
        uint8_t fCoverage;
        UniformHandle fViewMatrixUniform;
        UniformHandle fColorUniform;
        UniformHandle fCoverageUniform;
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
                   const SkPMColor4f& color,
                   sk_sp<GrColorSpaceXform> colorSpaceXform,
                   const SkMatrix& viewMatrix,
                   const SkMatrix& localMatrix,
                   uint8_t coverage,
                   bool localCoordsWillBeRead)
            : INHERITED(kDefaultGeoProc_ClassID)
            , fColor(color)
            , fViewMatrix(viewMatrix)
            , fLocalMatrix(localMatrix)
            , fCoverage(coverage)
            , fFlags(gpTypeFlags)
            , fLocalCoordsWillBeRead(localCoordsWillBeRead)
            , fColorSpaceXform(std::move(colorSpaceXform)) {
        fInPosition = {"inPosition", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
        if (fFlags & kColorAttribute_GPFlag) {
            fInColor = MakeColorAttribute("inColor",
                                          SkToBool(fFlags & kColorAttributeIsWide_GPFlag));
        }
        if (fFlags & kLocalCoordAttribute_GPFlag) {
            fInLocalCoords = {"inLocalCoord", kFloat2_GrVertexAttribType,
                                              kFloat2_GrSLType};
        }
        if (fFlags & kCoverageAttribute_GPFlag) {
            fInCoverage = {"inCoverage", kFloat_GrVertexAttribType, kHalf_GrSLType};
        }
        this->setVertexAttributes(&fInPosition, 4);
    }

    Attribute fInPosition;
    Attribute fInColor;
    Attribute fInLocalCoords;
    Attribute fInCoverage;
    SkPMColor4f fColor;
    SkMatrix fViewMatrix;
    SkMatrix fLocalMatrix;
    uint8_t fCoverage;
    uint32_t fFlags;
    bool fLocalCoordsWillBeRead;
    sk_sp<GrColorSpaceXform> fColorSpaceXform;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST

    typedef GrGeometryProcessor INHERITED;
};

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(DefaultGeoProc);

#if GR_TEST_UTILS
sk_sp<GrGeometryProcessor> DefaultGeoProc::TestCreate(GrProcessorTestData* d) {
    uint32_t flags = 0;
    if (d->fRandom->nextBool()) {
        flags |= kColorAttribute_GPFlag;
    }
    if (d->fRandom->nextBool()) {
        flags |= kColorAttributeIsSkColor_GPFlag;
    }
    if (d->fRandom->nextBool()) {
        flags |= kColorAttributeIsWide_GPFlag;
    }
    if (d->fRandom->nextBool()) {
        flags |= kCoverageAttribute_GPFlag;
        if (d->fRandom->nextBool()) {
            flags |= kCoverageAttributeTweak_GPFlag;
        }
    }
    if (d->fRandom->nextBool()) {
        flags |= kLocalCoordAttribute_GPFlag;
    }

    return DefaultGeoProc::Make(d->caps()->shaderCaps(),
                                flags,
                                SkPMColor4f::FromBytes_RGBA(GrRandomColor(d->fRandom)),
                                GrTest::TestColorXform(d->fRandom),
                                GrTest::TestMatrix(d->fRandom),
                                GrTest::TestMatrix(d->fRandom),
                                d->fRandom->nextBool(),
                                GrRandomCoverage(d->fRandom));
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
    } else if (Color::kPremulWideColorAttribute_Type == color.fType) {
        flags |= kColorAttribute_GPFlag | kColorAttributeIsWide_GPFlag;
    }
    if (Coverage::kAttribute_Type == coverage.fType) {
        flags |= kCoverageAttribute_GPFlag;
    } else if (Coverage::kAttributeTweakAlpha_Type == coverage.fType) {
        flags |= kCoverageAttribute_GPFlag | kCoverageAttributeTweak_GPFlag;
    }
    flags |= localCoords.fType == LocalCoords::kHasExplicit_Type ? kLocalCoordAttribute_GPFlag : 0;

    uint8_t inCoverage = coverage.fCoverage;
    bool localCoordsWillBeRead = localCoords.fType != LocalCoords::kUnused_Type;

    return DefaultGeoProc::Make(shaderCaps,
                                flags,
                                color.fColor,
                                color.fColorSpaceXform,
                                viewMatrix,
                                localCoords.fMatrix ? *localCoords.fMatrix : SkMatrix::I(),
                                localCoordsWillBeRead,
                                inCoverage);
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
            invert.postConcat(*localCoords.fMatrix);
        }
    }

    LocalCoords inverted(LocalCoords::kUsePosition_Type, &invert);
    return Make(shaderCaps, color, coverage, inverted, SkMatrix::I());
}
