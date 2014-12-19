/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDefaultGeoProcFactory.h"

#include "GrDrawState.h"
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
    static GrGeometryProcessor* Create(GrColor color, uint8_t coverage, uint32_t gpTypeFlags,
                                       bool opaqueVertexColors) {
        return SkNEW_ARGS(DefaultGeoProc, (color, coverage, gpTypeFlags, opaqueVertexColors));
    }

    virtual const char* name() const SK_OVERRIDE { return "DefaultGeometryProcessor"; }

    const GrAttribute* inPosition() const { return fInPosition; }
    const GrAttribute* inColor() const { return fInColor; }
    const GrAttribute* inLocalCoords() const { return fInLocalCoords; }
    const GrAttribute* inCoverage() const { return fInCoverage; }
    uint8_t coverage() const { return fCoverage; }

    void initBatchTracker(GrBatchTracker* bt, const InitBT& init) const SK_OVERRIDE {
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

        virtual void emitCode(const EmitArgs& args) SK_OVERRIDE {
            const DefaultGeoProc& gp = args.fGP.cast<DefaultGeoProc>();
            GrGLGPBuilder* pb = args.fPB;
            GrGLVertexBuilder* vs = pb->getVertexShaderBuilder();
            GrGLGPFragmentBuilder* fs = args.fPB->getFragmentShaderBuilder();
            const BatchTracker& local = args.fBT.cast<BatchTracker>();

            vs->codeAppendf("%s = %s;", vs->positionCoords(), gp.inPosition()->fName);

            // Setup pass through color
            this->setupColorPassThrough(pb, local.fInputColorType, args.fOutputColor, gp.inColor(),
                                        &fColorUniform);

            // Setup local coords if needed
            if (gp.inLocalCoords()) {
                vs->codeAppendf("%s = %s;", vs->localCoords(), gp.inLocalCoords()->fName);
            } else {
                vs->codeAppendf("%s = %s;", vs->localCoords(), gp.inPosition()->fName);
            }

            // setup position varying
            vs->codeAppendf("%s = %s * vec3(%s, 1);", vs->glPosition(), vs->uViewM(),
                            gp.inPosition()->fName);

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
            b->add32(def.fFlags);

            const BatchTracker& local = bt.cast<BatchTracker>();
            b->add32(local.fInputColorType | local.fInputCoverageType << 16);
        }

        virtual void setData(const GrGLProgramDataManager& pdman,
                             const GrPrimitiveProcessor& gp,
                             const GrBatchTracker& bt) SK_OVERRIDE {
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

    virtual GrGLGeometryProcessor* createGLInstance(const GrBatchTracker& bt) const SK_OVERRIDE {
        return SkNEW_ARGS(GLProcessor, (*this, bt));
    }

private:
    DefaultGeoProc(GrColor color, uint8_t coverage, uint32_t gpTypeFlags, bool opaqueVertexColors)
        : INHERITED(color, opaqueVertexColors)
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
        fInPosition = &this->addVertexAttrib(GrAttribute("inPosition", kVec2f_GrVertexAttribType));
        if (hasColor) {
            fInColor = &this->addVertexAttrib(GrAttribute("inColor", kVec4ub_GrVertexAttribType));
            this->setHasVertexColor();
        }
        if (hasLocalCoord) {
            fInLocalCoords = &this->addVertexAttrib(GrAttribute("inLocalCoord",
                                                                kVec2f_GrVertexAttribType));
            this->setHasLocalCoords();
        }
        if (hasCoverage) {
            fInCoverage = &this->addVertexAttrib(GrAttribute("inCoverage",
                                                             kFloat_GrVertexAttribType));
        }
    }

    virtual bool onIsEqual(const GrGeometryProcessor& other) const SK_OVERRIDE {
        const DefaultGeoProc& gp = other.cast<DefaultGeoProc>();
        return gp.fFlags == this->fFlags;
    }

    virtual void onGetInvariantOutputCoverage(GrInitInvariantOutput* out) const SK_OVERRIDE {
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

    const GrAttribute* fInPosition;
    const GrAttribute* fInColor;
    const GrAttribute* fInLocalCoords;
    const GrAttribute* fInCoverage;
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

    return DefaultGeoProc::Create(GrRandomColor(random), GrRandomCoverage(random),
                                  flags, random->nextBool());
}

const GrGeometryProcessor* GrDefaultGeoProcFactory::Create(GrColor color,
                                                           uint32_t gpTypeFlags,
                                                           bool opaqueVertexColors,
                                                           uint8_t coverage) {
    return DefaultGeoProc::Create(color, coverage, gpTypeFlags, opaqueVertexColors);
}
