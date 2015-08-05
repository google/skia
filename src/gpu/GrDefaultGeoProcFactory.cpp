/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDefaultGeoProcFactory.h"

#include "GrInvariantOutput.h"
#include "gl/GrGLGeometryProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"

/*
 * The default Geometry Processor simply takes position and multiplies it by the uniform view
 * matrix. It also leaves coverage untouched.  Behind the scenes, we may add per vertex color or
 * local coords.
 */

enum GPFlag {
    kColor_GPFlag =                 0x1,
    kLocalCoord_GPFlag =            0x2,
    kCoverage_GPFlag=               0x4,
    kTransformedLocalCoord_GPFlag = 0x8,
};

class DefaultGeoProc : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Create(uint32_t gpTypeFlags,
                                       GrColor color,
                                       const SkMatrix& viewMatrix,
                                       const SkMatrix& localMatrix,
                                       bool localCoordsWillBeRead,
                                       bool coverageWillBeIgnored,
                                       uint8_t coverage) {
        return SkNEW_ARGS(DefaultGeoProc, (gpTypeFlags,
                                           color,
                                           viewMatrix,
                                           localMatrix,
                                           coverage,
                                           localCoordsWillBeRead,
                                           coverageWillBeIgnored));
    }

    const char* name() const override { return "DefaultGeometryProcessor"; }

    const Attribute* inPosition() const { return fInPosition; }
    const Attribute* inColor() const { return fInColor; }
    const Attribute* inLocalCoords() const { return fInLocalCoords; }
    const Attribute* inCoverage() const { return fInCoverage; }
    GrColor color() const { return fColor; }
    bool colorIgnored() const { return GrColor_ILLEGAL == fColor; }
    bool hasVertexColor() const { return SkToBool(fInColor); }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    const SkMatrix& localMatrix() const { return fLocalMatrix; }
    bool localCoordsWillBeRead() const { return fLocalCoordsWillBeRead; }
    uint8_t coverage() const { return fCoverage; }
    bool coverageWillBeIgnored() const { return fCoverageWillBeIgnored; }
    bool hasVertexCoverage() const { return SkToBool(fInCoverage); }

    class GLProcessor : public GrGLGeometryProcessor {
    public:
        GLProcessor(const GrGeometryProcessor& gp, const GrBatchTracker&)
            : fViewMatrix(SkMatrix::InvalidMatrix()), fColor(GrColor_ILLEGAL), fCoverage(0xff) {}

        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
            const DefaultGeoProc& gp = args.fGP.cast<DefaultGeoProc>();
            GrGLGPBuilder* pb = args.fPB;
            GrGLVertexBuilder* vsBuilder = pb->getVertexShaderBuilder();
            GrGLFragmentBuilder* fs = args.fPB->getFragmentShaderBuilder();

            // emit attributes
            vsBuilder->emitAttributes(gp);

            // Setup pass through color
            if (!gp.colorIgnored()) {
                if (gp.hasVertexColor()) {
                    pb->addPassThroughAttribute(gp.inColor(), args.fOutputColor);
                } else {
                    this->setupUniformColor(pb, args.fOutputColor, &fColorUniform);
                }
            }

            // Setup position
            this->setupPosition(pb, gpArgs, gp.inPosition()->fName, gp.viewMatrix(),
                                &fViewMatrixUniform);

            if (gp.hasExplicitLocalCoords()) {
                // emit transforms with explicit local coords
                this->emitTransforms(pb, gpArgs->fPositionVar, gp.inLocalCoords()->fName,
                                     gp.localMatrix(), args.fTransformsIn, args.fTransformsOut);
            } else if(gp.hasTransformedLocalCoords()) {
                // transforms have already been applied to vertex attributes on the cpu
                this->emitTransforms(pb, gp.inLocalCoords()->fName,
                                     args.fTransformsIn, args.fTransformsOut);
            } else {
                // emit transforms with position
                this->emitTransforms(pb, gpArgs->fPositionVar, gp.inPosition()->fName,
                                     gp.localMatrix(), args.fTransformsIn, args.fTransformsOut);
            }

            // Setup coverage as pass through
            if (!gp.coverageWillBeIgnored()) {
                if (gp.hasVertexCoverage()) {
                    fs->codeAppendf("float alpha = 1.0;");
                    args.fPB->addPassThroughAttribute(gp.inCoverage(), "alpha");
                    fs->codeAppendf("%s = vec4(alpha);", args.fOutputCoverage);
                } else if (gp.coverage() == 0xff) {
                    fs->codeAppendf("%s = vec4(1);", args.fOutputCoverage);
                } else {
                    const char* fragCoverage;
                    fCoverageUniform = pb->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                                      kFloat_GrSLType,
                                                      kDefault_GrSLPrecision,
                                                      "Coverage",
                                                      &fragCoverage);
                    fs->codeAppendf("%s = vec4(%s);", args.fOutputCoverage, fragCoverage);
                }
            }
        }

        static inline void GenKey(const GrGeometryProcessor& gp,
                                  const GrBatchTracker& bt,
                                  const GrGLSLCaps&,
                                  GrProcessorKeyBuilder* b) {
            const DefaultGeoProc& def = gp.cast<DefaultGeoProc>();
            uint32_t key = def.fFlags;
            key |= def.colorIgnored() << 8;
            key |= def.coverageWillBeIgnored() << 9;
            key |= def.hasVertexColor() << 10;
            key |= def.hasVertexCoverage() << 11;
            key |= def.coverage() == 0xff ? 0x1 << 12 : 0;
            key |= def.localCoordsWillBeRead() && def.localMatrix().hasPerspective() ? 0x1 << 24 :
                                                                                       0x0;
            key |= ComputePosKey(def.viewMatrix()) << 25;
            b->add32(key);
        }

        virtual void setData(const GrGLProgramDataManager& pdman,
                             const GrPrimitiveProcessor& gp,
                             const GrBatchTracker& bt) override {
            const DefaultGeoProc& dgp = gp.cast<DefaultGeoProc>();

            if (!dgp.viewMatrix().isIdentity() && !fViewMatrix.cheapEqualTo(dgp.viewMatrix())) {
                fViewMatrix = dgp.viewMatrix();
                GrGLfloat viewMatrix[3 * 3];
                GrGLGetMatrix<3>(viewMatrix, fViewMatrix);
                pdman.setMatrix3f(fViewMatrixUniform, viewMatrix);
            }

            if (dgp.color() != fColor && !dgp.hasVertexColor()) {
                GrGLfloat c[4];
                GrColorToRGBAFloat(dgp.color(), c);
                pdman.set4fv(fColorUniform, 1, c);
                fColor = dgp.color();
            }

            if (!dgp.coverageWillBeIgnored() &&
                dgp.coverage() != fCoverage && !dgp.hasVertexCoverage()) {
                pdman.set1f(fCoverageUniform, GrNormalizeByteToFloat(dgp.coverage()));
                fCoverage = dgp.coverage();
            }
        }

        void setTransformData(const GrPrimitiveProcessor& primProc,
                              const GrGLProgramDataManager& pdman,
                              int index,
                              const SkTArray<const GrCoordTransform*, true>& transforms) override {
            this->setTransformDataHelper<DefaultGeoProc>(primProc, pdman, index, transforms);
        }

    private:
        SkMatrix fViewMatrix;
        GrColor fColor;
        uint8_t fCoverage;
        UniformHandle fViewMatrixUniform;
        UniformHandle fColorUniform;
        UniformHandle fCoverageUniform;

        typedef GrGLGeometryProcessor INHERITED;
    };

    virtual void getGLProcessorKey(const GrBatchTracker& bt,
                                   const GrGLSLCaps& caps,
                                   GrProcessorKeyBuilder* b) const override {
        GLProcessor::GenKey(*this, bt, caps, b);
    }

    virtual GrGLPrimitiveProcessor* createGLInstance(const GrBatchTracker& bt,
                                                     const GrGLSLCaps&) const override {
        return SkNEW_ARGS(GLProcessor, (*this, bt));
    }

private:
    DefaultGeoProc(uint32_t gpTypeFlags,
                   GrColor color,
                   const SkMatrix& viewMatrix,
                   const SkMatrix& localMatrix,
                   uint8_t coverage,
                   bool localCoordsWillBeRead,
                   bool coverageWillBeIgnored)
        : fInPosition(NULL)
        , fInColor(NULL)
        , fInLocalCoords(NULL)
        , fInCoverage(NULL)
        , fColor(color)
        , fViewMatrix(viewMatrix)
        , fLocalMatrix(localMatrix)
        , fCoverage(coverage)
        , fFlags(gpTypeFlags)
        , fLocalCoordsWillBeRead(localCoordsWillBeRead)
        , fCoverageWillBeIgnored(coverageWillBeIgnored) {
        this->initClassID<DefaultGeoProc>();
        bool hasColor = SkToBool(gpTypeFlags & kColor_GPFlag);
        bool hasExplicitLocalCoords = SkToBool(gpTypeFlags & kLocalCoord_GPFlag);
        bool hasTransformedLocalCoords = SkToBool(gpTypeFlags & kTransformedLocalCoord_GPFlag);
        bool hasLocalCoord = hasExplicitLocalCoords || hasTransformedLocalCoords;
        bool hasCoverage = SkToBool(gpTypeFlags & kCoverage_GPFlag);
        fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType,
                                                       kHigh_GrSLPrecision));
        if (hasColor) {
            fInColor = &this->addVertexAttrib(Attribute("inColor", kVec4ub_GrVertexAttribType));
        }
        if (hasLocalCoord) {
            fInLocalCoords = &this->addVertexAttrib(Attribute("inLocalCoord",
                                                              kVec2f_GrVertexAttribType));
            if (hasExplicitLocalCoords) {
                this->setHasExplicitLocalCoords();
            } else {
                SkASSERT(hasTransformedLocalCoords);
                this->setHasTransformedLocalCoords();
            }
        }
        if (hasCoverage) {
            fInCoverage = &this->addVertexAttrib(Attribute("inCoverage",
                                                           kFloat_GrVertexAttribType));
        }
    }

    const Attribute* fInPosition;
    const Attribute* fInColor;
    const Attribute* fInLocalCoords;
    const Attribute* fInCoverage;
    GrColor fColor;
    SkMatrix fViewMatrix;
    SkMatrix fLocalMatrix;
    uint8_t fCoverage;
    uint32_t fFlags;
    bool fLocalCoordsWillBeRead;
    bool fCoverageWillBeIgnored;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(DefaultGeoProc);

GrGeometryProcessor* DefaultGeoProc::TestCreate(GrProcessorTestData* d) {
    uint32_t flags = 0;
    if (d->fRandom->nextBool()) {
        flags |= kColor_GPFlag;
    }
    if (d->fRandom->nextBool()) {
        flags |= kCoverage_GPFlag;
    }
    if (d->fRandom->nextBool()) {
        flags |= kLocalCoord_GPFlag;
    }
    if (d->fRandom->nextBool()) {
        flags |= kTransformedLocalCoord_GPFlag;
    }

    return DefaultGeoProc::Create(flags,
                                  GrRandomColor(d->fRandom),
                                  GrTest::TestMatrix(d->fRandom),
                                  GrTest::TestMatrix(d->fRandom),
                                  d->fRandom->nextBool(),
                                  d->fRandom->nextBool(),
                                  GrRandomCoverage(d->fRandom));
}

const GrGeometryProcessor* GrDefaultGeoProcFactory::Create(const Color& color,
                                                           const Coverage& coverage,
                                                           const LocalCoords& localCoords,
                                                           const SkMatrix& viewMatrix) {
    uint32_t flags = 0;
    flags |= color.fType == Color::kAttribute_Type ? kColor_GPFlag : 0;
    flags |= coverage.fType == Coverage::kAttribute_Type ? kCoverage_GPFlag : 0;
    flags |= localCoords.fType == LocalCoords::kHasExplicit_Type ? kLocalCoord_GPFlag : 0;
    flags |= localCoords.fType == LocalCoords::kHasTransformed_Type ?
                                  kTransformedLocalCoord_GPFlag : 0;

    uint8_t inCoverage = coverage.fCoverage;
    bool coverageWillBeIgnored = coverage.fType == Coverage::kNone_Type;
    bool localCoordsWillBeRead = localCoords.fType != LocalCoords::kUnused_Type;

    GrColor inColor = color.fColor;
    return DefaultGeoProc::Create(flags,
                                  inColor,
                                  viewMatrix,
                                  localCoords.fMatrix ? *localCoords.fMatrix : SkMatrix::I(),
                                  localCoordsWillBeRead,
                                  coverageWillBeIgnored,
                                  inCoverage);
}

const GrGeometryProcessor* GrDefaultGeoProcFactory::CreateForDeviceSpace(
                                                                     const Color& color,
                                                                     const Coverage& coverage,
                                                                     const LocalCoords& localCoords,
                                                                     const SkMatrix& viewMatrix) {
    SkMatrix invert = SkMatrix::I();
    if (LocalCoords::kUnused_Type != localCoords.fType) {
        SkASSERT(LocalCoords::kUsePosition_Type == localCoords.fType);
        if (!viewMatrix.isIdentity() && !viewMatrix.invert(&invert)) {
            SkDebugf("Could not invert\n");
            return NULL;
        }

        if (localCoords.hasLocalMatrix()) {
            invert.preConcat(*localCoords.fMatrix);
        }
    }

    LocalCoords inverted(LocalCoords::kUsePosition_Type, &invert);
    return Create(color, coverage, inverted, SkMatrix::I());
}
