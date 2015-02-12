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
typedef GrDefaultGeoProcFactory Flag;

class DefaultGeoProc : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Create(uint32_t gpTypeFlags,
                                       GrColor color,
                                       const SkMatrix& viewMatrix,
                                       const SkMatrix& localMatrix,
                                       bool opaqueVertexColors,
                                       uint8_t coverage) {
        return SkNEW_ARGS(DefaultGeoProc, (gpTypeFlags,
                                           color,
                                           viewMatrix,
                                           localMatrix,
                                           opaqueVertexColors,
                                           coverage));
    }

    const char* name() const SK_OVERRIDE { return "DefaultGeometryProcessor"; }

    const Attribute* inPosition() const { return fInPosition; }
    const Attribute* inColor() const { return fInColor; }
    const Attribute* inLocalCoords() const { return fInLocalCoords; }
    const Attribute* inCoverage() const { return fInCoverage; }
    uint8_t coverage() const { return fCoverage; }

    void initBatchTracker(GrBatchTracker* bt, const GrPipelineInfo& init) const SK_OVERRIDE {
        BatchTracker* local = bt->cast<BatchTracker>();
        local->fInputColorType = GetColorInputType(&local->fColor, this->color(), init,
                                                   SkToBool(fInColor));

        bool hasVertexCoverage = SkToBool(fInCoverage) && !init.fCoverageIgnored;
        bool covIsSolidWhite = !hasVertexCoverage && 0xff == this->coverage();
        if (covIsSolidWhite) {
            local->fInputCoverageType = kAllOnes_GrGPInput;
        } else if (!hasVertexCoverage) {
            local->fInputCoverageType = kUniform_GrGPInput;
            local->fCoverage = this->coverage();
        } else if (hasVertexCoverage) {
            SkASSERT(fInCoverage);
            local->fInputCoverageType = kAttribute_GrGPInput;
        } else {
            local->fInputCoverageType = kIgnored_GrGPInput;
        }

        local->fUsesLocalCoords = init.fUsesLocalCoords;
    }

    bool onCanMakeEqual(const GrBatchTracker& m,
                        const GrGeometryProcessor& that,
                        const GrBatchTracker& t) const SK_OVERRIDE {
        const BatchTracker& mine = m.cast<BatchTracker>();
        const BatchTracker& theirs = t.cast<BatchTracker>();
        return CanCombineLocalMatrices(*this, mine.fUsesLocalCoords,
                                       that, theirs.fUsesLocalCoords) &&
               CanCombineOutput(mine.fInputColorType, mine.fColor,
                                theirs.fInputColorType, theirs.fColor) &&
               CanCombineOutput(mine.fInputCoverageType, mine.fCoverage,
                                theirs.fInputCoverageType, theirs.fCoverage);
    }

    class GLProcessor : public GrGLGeometryProcessor {
    public:
        GLProcessor(const GrGeometryProcessor& gp, const GrBatchTracker&)
            : fColor(GrColor_ILLEGAL), fCoverage(0xff) {}

        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) SK_OVERRIDE {
            const DefaultGeoProc& gp = args.fGP.cast<DefaultGeoProc>();
            GrGLGPBuilder* pb = args.fPB;
            GrGLVertexBuilder* vsBuilder = pb->getVertexShaderBuilder();
            GrGLGPFragmentBuilder* fs = args.fPB->getFragmentShaderBuilder();
            const BatchTracker& local = args.fBT.cast<BatchTracker>();

            // emit attributes
            vsBuilder->emitAttributes(gp);

            // Setup pass through color
            this->setupColorPassThrough(pb, local.fInputColorType, args.fOutputColor, gp.inColor(),
                                        &fColorUniform);
            // Setup position
            this->setupPosition(pb, gpArgs, gp.inPosition()->fName, gp.viewMatrix());

            if (gp.inLocalCoords()) {
                // emit transforms with explicit local coords
                this->emitTransforms(pb, gpArgs->fPositionVar, gp.inLocalCoords()->fName,
                                     gp.localMatrix(), args.fTransformsIn, args.fTransformsOut);
            } else {
                // emit transforms with position
                this->emitTransforms(pb, gpArgs->fPositionVar, gp.inPosition()->fName,
                                     gp.localMatrix(), args.fTransformsIn, args.fTransformsOut);
            }

            // Setup coverage as pass through
            if (kUniform_GrGPInput == local.fInputCoverageType) {
                const char* fragCoverage;
                fCoverageUniform = pb->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                                  kFloat_GrSLType,
                                                  kDefault_GrSLPrecision,
                                                  "Coverage",
                                                  &fragCoverage);
                fs->codeAppendf("%s = vec4(%s);", args.fOutputCoverage, fragCoverage);
            } else if (kAttribute_GrGPInput == local.fInputCoverageType) {
                SkASSERT(gp.inCoverage());
                fs->codeAppendf("float alpha = 1.0;");
                args.fPB->addPassThroughAttribute(gp.inCoverage(), "alpha");
                fs->codeAppendf("%s = vec4(alpha);", args.fOutputCoverage);
            } else if (kAllOnes_GrGPInput == local.fInputCoverageType) {
                fs->codeAppendf("%s = vec4(1);", args.fOutputCoverage);
            }
        }

        static inline void GenKey(const GrGeometryProcessor& gp,
                                  const GrBatchTracker& bt,
                                  const GrGLCaps&,
                                  GrProcessorKeyBuilder* b) {
            const DefaultGeoProc& def = gp.cast<DefaultGeoProc>();
            const BatchTracker& local = bt.cast<BatchTracker>();
            uint32_t key = def.fFlags;
            key |= local.fInputColorType << 8 | local.fInputCoverageType << 16;
            key |= local.fUsesLocalCoords && gp.localMatrix().hasPerspective() ? 0x1 << 24 : 0x0;
            key |= ComputePosKey(gp.viewMatrix()) << 25;
            b->add32(key);
        }

        virtual void setData(const GrGLProgramDataManager& pdman,
                             const GrPrimitiveProcessor& gp,
                             const GrBatchTracker& bt) SK_OVERRIDE {
            this->setUniformViewMatrix(pdman, gp.viewMatrix());

            const BatchTracker& local = bt.cast<BatchTracker>();
            if (kUniform_GrGPInput == local.fInputColorType && local.fColor != fColor) {
                GrGLfloat c[4];
                GrColorToRGBAFloat(local.fColor, c);
                pdman.set4fv(fColorUniform, 1, c);
                fColor = local.fColor;
            }
            if (kUniform_GrGPInput == local.fInputCoverageType && local.fCoverage != fCoverage) {
                pdman.set1f(fCoverageUniform, GrNormalizeByteToFloat(local.fCoverage));
                fCoverage = local.fCoverage;
            }
        }

    private:
        GrColor fColor;
        uint8_t fCoverage;
        UniformHandle fColorUniform;
        UniformHandle fCoverageUniform;

        typedef GrGLGeometryProcessor INHERITED;
    };

    virtual void getGLProcessorKey(const GrBatchTracker& bt,
                                   const GrGLCaps& caps,
                                   GrProcessorKeyBuilder* b) const SK_OVERRIDE {
        GLProcessor::GenKey(*this, bt, caps, b);
    }

    virtual GrGLPrimitiveProcessor* createGLInstance(const GrBatchTracker& bt,
                                                     const GrGLCaps&) const SK_OVERRIDE {
        return SkNEW_ARGS(GLProcessor, (*this, bt));
    }

private:
    DefaultGeoProc(uint32_t gpTypeFlags,
                   GrColor color,
                   const SkMatrix& viewMatrix,
                   const SkMatrix& localMatrix,
                   bool opaqueVertexColors,
                   uint8_t coverage)
        : INHERITED(color, viewMatrix, localMatrix, opaqueVertexColors)
        , fInPosition(NULL)
        , fInColor(NULL)
        , fInLocalCoords(NULL)
        , fInCoverage(NULL)
        , fCoverage(coverage)
        , fFlags(gpTypeFlags) {
        this->initClassID<DefaultGeoProc>();
        bool hasColor = SkToBool(gpTypeFlags & GrDefaultGeoProcFactory::kColor_GPType);
        bool hasLocalCoord = SkToBool(gpTypeFlags & GrDefaultGeoProcFactory::kLocalCoord_GPType);
        bool hasCoverage = SkToBool(gpTypeFlags & GrDefaultGeoProcFactory::kCoverage_GPType);
        fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType));
        if (hasColor) {
            fInColor = &this->addVertexAttrib(Attribute("inColor", kVec4ub_GrVertexAttribType));
            this->setHasVertexColor();
        }
        if (hasLocalCoord) {
            fInLocalCoords = &this->addVertexAttrib(Attribute("inLocalCoord",
                                                                kVec2f_GrVertexAttribType));
            this->setHasLocalCoords();
        }
        if (hasCoverage) {
            fInCoverage = &this->addVertexAttrib(Attribute("inCoverage",
                                                             kFloat_GrVertexAttribType));
        }
    }

    bool onIsEqual(const GrGeometryProcessor& other) const SK_OVERRIDE {
        const DefaultGeoProc& gp = other.cast<DefaultGeoProc>();
        return gp.fFlags == this->fFlags;
    }

    void onGetInvariantOutputCoverage(GrInitInvariantOutput* out) const SK_OVERRIDE {
        if (fInCoverage) {
            out->setUnknownSingleComponent();
        } else {
            // uniform coverage
            out->setKnownSingleComponent(this->coverage());
        }
    }

    struct BatchTracker {
        GrGPInput fInputColorType;
        GrGPInput fInputCoverageType;
        GrColor  fColor;
        GrColor  fCoverage;
        bool fUsesLocalCoords;
    };

    const Attribute* fInPosition;
    const Attribute* fInColor;
    const Attribute* fInLocalCoords;
    const Attribute* fInCoverage;
    uint8_t fCoverage;
    uint32_t fFlags;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(DefaultGeoProc);

GrGeometryProcessor* DefaultGeoProc::TestCreate(SkRandom* random,
                                                GrContext*,
                                                const GrDrawTargetCaps& caps,
                                                GrTexture*[]) {
    uint32_t flags = 0;
    if (random->nextBool()) {
        flags |= GrDefaultGeoProcFactory::kColor_GPType;
    }
    if (random->nextBool()) {
        flags |= GrDefaultGeoProcFactory::kCoverage_GPType;
    }
    if (random->nextBool()) {
        flags |= GrDefaultGeoProcFactory::kLocalCoord_GPType;
    }

    return DefaultGeoProc::Create(flags,
                                  GrRandomColor(random),
                                  GrProcessorUnitTest::TestMatrix(random),
                                  GrProcessorUnitTest::TestMatrix(random),
                                  random->nextBool(),
                                  GrRandomCoverage(random));
}

const GrGeometryProcessor* GrDefaultGeoProcFactory::Create(uint32_t gpTypeFlags,
                                                           GrColor color,
                                                           const SkMatrix& viewMatrix,
                                                           const SkMatrix& localMatrix,
                                                           bool opaqueVertexColors,
                                                           uint8_t coverage) {
    return DefaultGeoProc::Create(gpTypeFlags,
                                  color,
                                  viewMatrix,
                                  localMatrix,
                                  opaqueVertexColors,
                                  coverage);
}
