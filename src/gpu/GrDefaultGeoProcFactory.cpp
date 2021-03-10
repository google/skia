/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrDefaultGeoProcFactory.h"

#include "include/core/SkRefCnt.h"
#include "src/core/SkArenaAlloc.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

/*
 * The default Geometry Processor simply takes position and multiplies it by the uniform view
 * matrix. It also leaves coverage untouched.  Behind the scenes, we may add per vertex color or
 * local coords.
 */

enum GPFlag {
    kColorAttribute_GPFlag          = 0x1,
    kColorAttributeIsWide_GPFlag    = 0x2,
    kLocalCoordAttribute_GPFlag     = 0x4,
    kCoverageAttribute_GPFlag       = 0x8,
    kCoverageAttributeTweak_GPFlag  = 0x10,
};

class DefaultGeoProc : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Make(SkArenaAlloc* arena,
                                     uint32_t gpTypeFlags,
                                     const SkPMColor4f& color,
                                     const SkMatrix& viewMatrix,
                                     const SkMatrix& localMatrix,
                                     bool localCoordsWillBeRead,
                                     uint8_t coverage) {
        return arena->make([&](void* ptr) {
            return new (ptr) DefaultGeoProc(gpTypeFlags, color, viewMatrix, localMatrix, coverage,
                                            localCoordsWillBeRead);
        });
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
            : fViewMatrixPrev(SkMatrix::InvalidMatrix())
            , fLocalMatrixPrev(SkMatrix::InvalidMatrix())
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
            fragBuilder->codeAppendf("half4 %s;", args.fOutputColor);
            if (gp.hasVertexColor() || tweakAlpha) {
                GrGLSLVarying varying(kHalf4_GrSLType);
                varyingHandler->addVarying("color", &varying);

                // Start with the attribute or with uniform color
                if (gp.hasVertexColor()) {
                    vertBuilder->codeAppendf("half4 color = %s;", gp.fInColor.name());
                } else {
                    const char* colorUniformName;
                    fColorUniform = uniformHandler->addUniform(nullptr,
                                                               kVertex_GrShaderFlag,
                                                               kHalf4_GrSLType,
                                                               "Color",
                                                               &colorUniformName);
                    vertBuilder->codeAppendf("half4 color = %s;", colorUniformName);
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

            // emit transforms using either explicit local coords or positions
            if (gp.fInLocalCoords.isInitialized()) {
                SkASSERT(gp.localMatrix().isIdentity());
                gpArgs->fLocalCoordVar = gp.fInLocalCoords.asShaderVar();
            } else if (gp.fLocalCoordsWillBeRead) {
                this->writeLocalCoord(vertBuilder, uniformHandler, gpArgs,
                                      gp.fInPosition.asShaderVar(), gp.localMatrix(),
                                      &fLocalMatrixUniform);
            }

            // Setup coverage as pass through
            if (gp.hasVertexCoverage() && !tweakAlpha) {
                fragBuilder->codeAppendf("half alpha = 1.0;");
                varyingHandler->addPassThroughAttribute(gp.fInCoverage, "alpha");
                fragBuilder->codeAppendf("half4 %s = half4(alpha);", args.fOutputCoverage);
            } else if (gp.coverage() == 0xff) {
                fragBuilder->codeAppendf("const half4 %s = half4(1);", args.fOutputCoverage);
            } else {
                const char* fragCoverage;
                fCoverageUniform = uniformHandler->addUniform(nullptr,
                                                              kFragment_GrShaderFlag,
                                                              kHalf_GrSLType,
                                                              "Coverage",
                                                              &fragCoverage);
                fragBuilder->codeAppendf("half4 %s = half4(%s);",
                                         args.fOutputCoverage, fragCoverage);
            }
        }

        static inline void GenKey(const GrGeometryProcessor& gp,
                                  const GrShaderCaps&,
                                  GrProcessorKeyBuilder* b) {
            const DefaultGeoProc& def = gp.cast<DefaultGeoProc>();
            uint32_t key = def.fFlags;
            key |= (def.coverage() == 0xff) ? 0x80 : 0;
            key |= def.localCoordsWillBeRead() ? 0x100 : 0;

            bool usesLocalMatrix = def.localCoordsWillBeRead() &&
                                   !def.fInLocalCoords.isInitialized();
            key = AddMatrixKeys(key, def.viewMatrix(),
                                usesLocalMatrix ? def.localMatrix() : SkMatrix::I());
            b->add32(key);
        }

        void setData(const GrGLSLProgramDataManager& pdman,
                     const GrPrimitiveProcessor& gp) override {
            const DefaultGeoProc& dgp = gp.cast<DefaultGeoProc>();

            this->setTransform(pdman, fViewMatrixUniform, dgp.viewMatrix(), &fViewMatrixPrev);
            this->setTransform(pdman, fLocalMatrixUniform, dgp.localMatrix(), &fLocalMatrixPrev);

            if (!dgp.hasVertexColor() && dgp.color() != fColor) {
                pdman.set4fv(fColorUniform, 1, dgp.color().vec());
                fColor = dgp.color();
            }

            if (dgp.coverage() != fCoverage && !dgp.hasVertexCoverage()) {
                pdman.set1f(fCoverageUniform, GrNormalizeByteToFloat(dgp.coverage()));
                fCoverage = dgp.coverage();
            }
        }

    private:
        SkMatrix fViewMatrixPrev;
        SkMatrix fLocalMatrixPrev;
        SkPMColor4f fColor;
        uint8_t fCoverage;
        UniformHandle fViewMatrixUniform;
        UniformHandle fLocalMatrixUniform;
        UniformHandle fColorUniform;
        UniformHandle fCoverageUniform;

        using INHERITED = GrGLSLGeometryProcessor;
    };

    void getGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {
        GLSLProcessor::GenKey(*this, caps, b);
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override {
        return new GLSLProcessor();
    }

private:
    DefaultGeoProc(uint32_t gpTypeFlags,
                   const SkPMColor4f& color,
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
            , fLocalCoordsWillBeRead(localCoordsWillBeRead) {
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

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST

    using INHERITED = GrGeometryProcessor;
};

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(DefaultGeoProc);

#if GR_TEST_UTILS
GrGeometryProcessor* DefaultGeoProc::TestCreate(GrProcessorTestData* d) {
    uint32_t flags = 0;
    if (d->fRandom->nextBool()) {
        flags |= kColorAttribute_GPFlag;
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

    return DefaultGeoProc::Make(d->allocator(),
                                flags,
                                SkPMColor4f::FromBytes_RGBA(GrRandomColor(d->fRandom)),
                                GrTest::TestMatrix(d->fRandom),
                                GrTest::TestMatrix(d->fRandom),
                                d->fRandom->nextBool(),
                                GrRandomCoverage(d->fRandom));
}
#endif

GrGeometryProcessor* GrDefaultGeoProcFactory::Make(SkArenaAlloc* arena,
                                                   const Color& color,
                                                   const Coverage& coverage,
                                                   const LocalCoords& localCoords,
                                                   const SkMatrix& viewMatrix) {
    uint32_t flags = 0;
    if (Color::kPremulGrColorAttribute_Type == color.fType) {
        flags |= kColorAttribute_GPFlag;
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

    return DefaultGeoProc::Make(arena,
                                flags,
                                color.fColor,
                                viewMatrix,
                                localCoords.fMatrix ? *localCoords.fMatrix : SkMatrix::I(),
                                localCoordsWillBeRead,
                                inCoverage);
}

GrGeometryProcessor* GrDefaultGeoProcFactory::MakeForDeviceSpace(SkArenaAlloc* arena,
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
    return Make(arena, color, coverage, inverted, SkMatrix::I());
}
