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
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

/*
 * The default Geometry Processor simply takes position and multiplies it by the uniform view
 * matrix. It also leaves coverage untouched.  Behind the scenes, we may add per vertex color or
 * local coords.
 */

enum GPFlag {
    kColorAttribute_GPFlag              = 0x1,
    kColorAttributeIsWide_GPFlag        = 0x2,
    kLocalCoordAttribute_GPFlag         = 0x4,
    kCoverageAttribute_GPFlag           = 0x8,
    kCoverageAttributeTweak_GPFlag      = 0x10,
    kCoverageAttributeUnclamped_GPFlag  = 0x20,
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

    void addToKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {
        uint32_t key = fFlags;
        key |= fCoverage == 0xff      ?  0x80 : 0;
        key |= fLocalCoordsWillBeRead ? 0x100 : 0;

        bool usesLocalMatrix = fLocalCoordsWillBeRead && !fInLocalCoords.isInitialized();
        key = ProgramImpl::AddMatrixKeys(caps,
                                         key,
                                         fViewMatrix,
                                         usesLocalMatrix ? fLocalMatrix : SkMatrix::I());
        b->add32(key);
    }

    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const override {
        return std::make_unique<Impl>();
    }

private:
    class Impl : public ProgramImpl {
    public:
        void setData(const GrGLSLProgramDataManager& pdman,
                     const GrShaderCaps& shaderCaps,
                     const GrGeometryProcessor& geomProc) override {
            const DefaultGeoProc& dgp = geomProc.cast<DefaultGeoProc>();

            SetTransform(pdman, shaderCaps, fViewMatrixUniform, dgp.fViewMatrix, &fViewMatrixPrev);
            SetTransform(pdman,
                         shaderCaps,
                         fLocalMatrixUniform,
                         dgp.fLocalMatrix,
                         &fLocalMatrixPrev);

            if (!dgp.hasVertexColor() && dgp.fColor != fColor) {
                pdman.set4fv(fColorUniform, 1, dgp.fColor.vec());
                fColor = dgp.fColor;
            }

            if (dgp.fCoverage != fCoverage && !dgp.hasVertexCoverage()) {
                pdman.set1f(fCoverageUniform, GrNormalizeByteToFloat(dgp.fCoverage));
                fCoverage = dgp.fCoverage;
            }
        }

    private:
        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
            const DefaultGeoProc& gp = args.fGeomProc.cast<DefaultGeoProc>();
            GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
            GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
            GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            // emit attributes
            varyingHandler->emitAttributes(gp);

            bool tweakAlpha = SkToBool(gp.fFlags & kCoverageAttributeTweak_GPFlag);
            bool coverageNeedsSaturate = SkToBool(gp.fFlags & kCoverageAttributeUnclamped_GPFlag);
            SkASSERT(!tweakAlpha || gp.hasVertexCoverage());
            SkASSERT(!tweakAlpha || !coverageNeedsSaturate);

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
            WriteOutputPosition(vertBuilder,
                                uniformHandler,
                                *args.fShaderCaps,
                                gpArgs,
                                gp.fInPosition.name(),
                                gp.fViewMatrix,
                                &fViewMatrixUniform);

            // emit transforms using either explicit local coords or positions
            if (gp.fInLocalCoords.isInitialized()) {
                SkASSERT(gp.fLocalMatrix.isIdentity());
                gpArgs->fLocalCoordVar = gp.fInLocalCoords.asShaderVar();
            } else if (gp.fLocalCoordsWillBeRead) {
                WriteLocalCoord(vertBuilder,
                                uniformHandler,
                                *args.fShaderCaps,
                                gpArgs,
                                gp.fInPosition.asShaderVar(),
                                gp.fLocalMatrix,
                                &fLocalMatrixUniform);
            }

            // Setup coverage as pass through
            if (gp.hasVertexCoverage() && !tweakAlpha) {
                fragBuilder->codeAppendf("half alpha = 1.0;");
                varyingHandler->addPassThroughAttribute(gp.fInCoverage.asShaderVar(), "alpha");
                if (coverageNeedsSaturate) {
                    fragBuilder->codeAppendf("half4 %s = half4(saturate(alpha));",
                                             args.fOutputCoverage);
                } else {
                    fragBuilder->codeAppendf("half4 %s = half4(alpha);", args.fOutputCoverage);
                }
            } else if (gp.fCoverage == 0xff) {
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

        SkMatrix    fViewMatrixPrev  = SkMatrix::InvalidMatrix();
        SkMatrix    fLocalMatrixPrev = SkMatrix::InvalidMatrix();
        SkPMColor4f fColor           = SK_PMColor4fILLEGAL;
        uint8_t     fCoverage        = 0xFF;

        UniformHandle fViewMatrixUniform;
        UniformHandle fLocalMatrixUniform;
        UniformHandle fColorUniform;
        UniformHandle fCoverageUniform;
    };

    bool hasVertexColor() const { return fInColor.isInitialized(); }
    bool hasVertexCoverage() const { return fInCoverage.isInitialized(); }

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
            flags |= (d->fRandom->nextBool()) ? kCoverageAttributeTweak_GPFlag
                                              : kCoverageAttributeUnclamped_GPFlag;
        }
    }
    if (d->fRandom->nextBool()) {
        flags |= kLocalCoordAttribute_GPFlag;
    }

    GrColor color = GrTest::RandomColor(d->fRandom);
    SkMatrix viewMtx = GrTest::TestMatrix(d->fRandom);
    SkMatrix localMtx = GrTest::TestMatrix(d->fRandom);
    bool readsLocalCoords = d->fRandom->nextBool();
    uint8_t coverage = GrTest::RandomCoverage(d->fRandom);
    return DefaultGeoProc::Make(d->allocator(),
                                flags,
                                SkPMColor4f::FromBytes_RGBA(color),
                                viewMtx,
                                localMtx,
                                readsLocalCoords,
                                coverage);
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
    } else if (Coverage::kAttributeUnclamped_Type == coverage.fType) {
        flags |= kCoverageAttribute_GPFlag | kCoverageAttributeUnclamped_GPFlag;
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
