/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDefaultGeoProcFactory.h"

#include "SkRefCnt.h"
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLVertexShaderBuilder.h"
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

    kLinearizeColorAttribute_GPFlag = 0x10,
};

class DefaultGeoProc : public GrGeometryProcessor {
public:
    static sk_sp<GrGeometryProcessor> Make(uint32_t gpTypeFlags,
                                           GrColor color,
                                           sk_sp<GrColorSpaceXform> colorSpaceXform,
                                           const SkMatrix& viewMatrix,
                                           const SkMatrix& localMatrix,
                                           bool localCoordsWillBeRead,
                                           uint8_t coverage) {
        return sk_sp<GrGeometryProcessor>(new DefaultGeoProc(
                gpTypeFlags, color, std::move(colorSpaceXform), viewMatrix, localMatrix, coverage,
                localCoordsWillBeRead));
    }

    const char* name() const override { return "DefaultGeometryProcessor"; }

    const Attribute* inPosition() const { return fInPosition; }
    const Attribute* inColor() const { return fInColor; }
    const Attribute* inLocalCoords() const { return fInLocalCoords; }
    const Attribute* inCoverage() const { return fInCoverage; }
    GrColor color() const { return fColor; }
    bool hasVertexColor() const { return SkToBool(fInColor); }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    const SkMatrix& localMatrix() const { return fLocalMatrix; }
    bool localCoordsWillBeRead() const { return fLocalCoordsWillBeRead; }
    uint8_t coverage() const { return fCoverage; }
    bool hasVertexCoverage() const { return SkToBool(fInCoverage); }
    bool linearizeColor() const {
        // Linearization should only happen with SkColor
        bool linearize = SkToBool(fFlags & kLinearizeColorAttribute_GPFlag);
        SkASSERT(!linearize || (fFlags & kColorAttributeIsSkColor_GPFlag));
        return linearize;
    }

    class GLSLProcessor : public GrGLSLGeometryProcessor {
    public:
        GLSLProcessor()
            : fViewMatrix(SkMatrix::InvalidMatrix()), fColor(GrColor_ILLEGAL), fCoverage(0xff) {}

        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
            const DefaultGeoProc& gp = args.fGP.cast<DefaultGeoProc>();
            GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
            GrGLSLPPFragmentBuilder* fragBuilder = args.fFragBuilder;
            GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            // emit attributes
            varyingHandler->emitAttributes(gp);

            // Setup pass through color
            if (gp.hasVertexColor()) {
                GrGLSLVertToFrag varying(kVec4f_GrSLType);
                varyingHandler->addVarying("color", &varying);

                // There are several optional steps to process the color. Start with the attribute:
                vertBuilder->codeAppendf("vec4 color = %s;", gp.inColor()->fName);

                // Linearize
                if (gp.linearizeColor()) {
                    SkString srgbFuncName;
                    static const GrShaderVar gSrgbArgs[] = {
                        GrShaderVar("x", kFloat_GrSLType),
                    };
                    vertBuilder->emitFunction(kFloat_GrSLType,
                                              "srgb_to_linear",
                                              SK_ARRAY_COUNT(gSrgbArgs),
                                              gSrgbArgs,
                                              "return (x <= 0.04045) ? (x / 12.92) "
                                              ": pow((x + 0.055) / 1.055, 2.4);",
                                              &srgbFuncName);
                    vertBuilder->codeAppendf("color = vec4(%s(%s.r), %s(%s.g), %s(%s.b), %s.a);",
                                             srgbFuncName.c_str(), gp.inColor()->fName,
                                             srgbFuncName.c_str(), gp.inColor()->fName,
                                             srgbFuncName.c_str(), gp.inColor()->fName,
                                             gp.inColor()->fName);
                }

                // For SkColor, do a red/blue swap and premul
                if (gp.fFlags & kColorAttributeIsSkColor_GPFlag) {
                    vertBuilder->codeAppend("color = vec4(color.a * color.bgr, color.a);");
                }

                // Do color-correction to destination gamut
                if (gp.linearizeColor()) {
                    fColorSpaceHelper.emitCode(uniformHandler, gp.fColorSpaceXform.get(),
                                               kVertex_GrShaderFlag);
                    if (fColorSpaceHelper.isValid()) {
                        SkString xformedColor;
                        vertBuilder->appendColorGamutXform(&xformedColor, "color",
                                                           &fColorSpaceHelper);
                        vertBuilder->codeAppendf("color = %s;", xformedColor.c_str());
                    }
                }
                vertBuilder->codeAppendf("%s = color;\n", varying.vsOut());
                fragBuilder->codeAppendf("%s = %s;", args.fOutputColor, varying.fsIn());
            } else {
                this->setupUniformColor(fragBuilder, uniformHandler, args.fOutputColor,
                                        &fColorUniform);
            }

            // Setup position
            this->setupPosition(vertBuilder,
                                uniformHandler,
                                gpArgs,
                                gp.inPosition()->fName,
                                gp.viewMatrix(),
                                &fViewMatrixUniform);

            if (gp.hasExplicitLocalCoords()) {
                // emit transforms with explicit local coords
                this->emitTransforms(vertBuilder,
                                     varyingHandler,
                                     uniformHandler,
                                     gpArgs->fPositionVar,
                                     gp.inLocalCoords()->fName,
                                     gp.localMatrix(),
                                     args.fFPCoordTransformHandler);
            } else {
                // emit transforms with position
                this->emitTransforms(vertBuilder,
                                     varyingHandler,
                                     uniformHandler,
                                     gpArgs->fPositionVar,
                                     gp.inPosition()->fName,
                                     gp.localMatrix(),
                                     args.fFPCoordTransformHandler);
            }

            // Setup coverage as pass through
            if (gp.hasVertexCoverage()) {
                fragBuilder->codeAppendf("float alpha = 1.0;");
                varyingHandler->addPassThroughAttribute(gp.inCoverage(), "alpha");
                fragBuilder->codeAppendf("%s = vec4(alpha);", args.fOutputCoverage);
            } else if (gp.coverage() == 0xff) {
                fragBuilder->codeAppendf("%s = vec4(1);", args.fOutputCoverage);
            } else {
                const char* fragCoverage;
                fCoverageUniform = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                              kFloat_GrSLType,
                                                              kDefault_GrSLPrecision,
                                                              "Coverage",
                                                              &fragCoverage);
                fragBuilder->codeAppendf("%s = vec4(%s);", args.fOutputCoverage, fragCoverage);
            }
        }

        static inline void GenKey(const GrGeometryProcessor& gp,
                                  const GrShaderCaps&,
                                  GrProcessorKeyBuilder* b) {
            const DefaultGeoProc& def = gp.cast<DefaultGeoProc>();
            uint32_t key = def.fFlags;
            key |= (def.coverage() == 0xff) ? 0x10 : 0;
            key |= (def.localCoordsWillBeRead() && def.localMatrix().hasPerspective()) ? 0x20 : 0x0;
            key |= ComputePosKey(def.viewMatrix()) << 20;
            b->add32(key);
            if (def.linearizeColor()) {
                b->add32(GrColorSpaceXform::XformKey(def.fColorSpaceXform.get()));
            }
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

            if (dgp.linearizeColor() && dgp.fColorSpaceXform) {
                fColorSpaceHelper.setData(pdman, dgp.fColorSpaceXform.get());
            }
        }

    private:
        SkMatrix fViewMatrix;
        GrColor fColor;
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
    DefaultGeoProc(uint32_t gpTypeFlags,
                   GrColor color,
                   sk_sp<GrColorSpaceXform> colorSpaceXform,
                   const SkMatrix& viewMatrix,
                   const SkMatrix& localMatrix,
                   uint8_t coverage,
                   bool localCoordsWillBeRead)
            : fColor(color)
            , fViewMatrix(viewMatrix)
            , fLocalMatrix(localMatrix)
            , fCoverage(coverage)
            , fFlags(gpTypeFlags)
            , fLocalCoordsWillBeRead(localCoordsWillBeRead)
            , fColorSpaceXform(std::move(colorSpaceXform)) {
        this->initClassID<DefaultGeoProc>();
        fInPosition = &this->addVertexAttrib("inPosition", kVec2f_GrVertexAttribType,
                                             kHigh_GrSLPrecision);
        if (fFlags & kColorAttribute_GPFlag) {
            fInColor = &this->addVertexAttrib("inColor", kVec4ub_GrVertexAttribType);
        }
        if (fFlags & kLocalCoordAttribute_GPFlag) {
            fInLocalCoords = &this->addVertexAttrib("inLocalCoord", kVec2f_GrVertexAttribType,
                                                    kHigh_GrSLPrecision);
            this->setHasExplicitLocalCoords();
        }
        if (fFlags & kCoverageAttribute_GPFlag) {
            fInCoverage = &this->addVertexAttrib("inCoverage", kFloat_GrVertexAttribType);
        }
    }

    const Attribute* fInPosition = nullptr;
    const Attribute* fInColor = nullptr;
    const Attribute* fInLocalCoords = nullptr;
    const Attribute* fInCoverage = nullptr;
    GrColor fColor;
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
        flags |= kCoverageAttribute_GPFlag;
    }
    if (d->fRandom->nextBool()) {
        flags |= kLocalCoordAttribute_GPFlag;
    }

    return DefaultGeoProc::Make(flags,
                                GrRandomColor(d->fRandom),
                                GrTest::TestColorXform(d->fRandom),
                                GrTest::TestMatrix(d->fRandom),
                                GrTest::TestMatrix(d->fRandom),
                                d->fRandom->nextBool(),
                                GrRandomCoverage(d->fRandom));
}
#endif

sk_sp<GrGeometryProcessor> GrDefaultGeoProcFactory::Make(const Color& color,
                                                         const Coverage& coverage,
                                                         const LocalCoords& localCoords,
                                                         const SkMatrix& viewMatrix) {
    uint32_t flags = 0;
    if (Color::kPremulGrColorAttribute_Type == color.fType) {
        flags |= kColorAttribute_GPFlag;
    } else if (Color::kUnpremulSkColorAttribute_Type == color.fType) {
        flags |= kColorAttribute_GPFlag | kColorAttributeIsSkColor_GPFlag;
    }
    if (color.fLinearize) {
        // It only makes sense to linearize SkColors (which are always sRGB). GrColor values should
        // have been linearized and gamut-converted during paint conversion
        SkASSERT(Color::kUnpremulSkColorAttribute_Type == color.fType);
        flags |= kLinearizeColorAttribute_GPFlag;
    }
    flags |= coverage.fType == Coverage::kAttribute_Type ? kCoverageAttribute_GPFlag : 0;
    flags |= localCoords.fType == LocalCoords::kHasExplicit_Type ? kLocalCoordAttribute_GPFlag : 0;

    uint8_t inCoverage = coverage.fCoverage;
    bool localCoordsWillBeRead = localCoords.fType != LocalCoords::kUnused_Type;

    GrColor inColor = color.fColor;
    return DefaultGeoProc::Make(flags,
                                inColor,
                                color.fColorSpaceXform,
                                viewMatrix,
                                localCoords.fMatrix ? *localCoords.fMatrix : SkMatrix::I(),
                                localCoordsWillBeRead,
                                inCoverage);
}

sk_sp<GrGeometryProcessor> GrDefaultGeoProcFactory::MakeForDeviceSpace(
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
    return Make(color, coverage, inverted, SkMatrix::I());
}
