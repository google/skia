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
    static GrGeometryProcessor* Create(uint32_t gpTypeFlags) {
        switch (gpTypeFlags) {
            case Flag::kColor_GPType | Flag::kCoverage_GPType | Flag::kLocalCoord_GPType: {
                GR_CREATE_STATIC_PROCESSOR(gDefaultGeoProcColLocCov, DefaultGeoProc, (gpTypeFlags));
                return SkRef(gDefaultGeoProcColLocCov);
            }
            case Flag::kColor_GPType | Flag::kLocalCoord_GPType: {
                GR_CREATE_STATIC_PROCESSOR(gDefaultGeoProcColLoc, DefaultGeoProc, (gpTypeFlags));
                return SkRef(gDefaultGeoProcColLoc);
            }
            case Flag::kColor_GPType | Flag::kCoverage_GPType: {
                GR_CREATE_STATIC_PROCESSOR(gDefaultGeoProcColCov, DefaultGeoProc, (gpTypeFlags));
                return SkRef(gDefaultGeoProcColCov);
            }
            case Flag::kColor_GPType: {
                GR_CREATE_STATIC_PROCESSOR(gDefaultGeoProcCol, DefaultGeoProc, (gpTypeFlags));
                return SkRef(gDefaultGeoProcCol);
            }
            case Flag::kLocalCoord_GPType | Flag::kCoverage_GPType: {
                GR_CREATE_STATIC_PROCESSOR(gDefaultGeoProcLocCov, DefaultGeoProc, (gpTypeFlags));
                return SkRef(gDefaultGeoProcLocCov);
            }
            case Flag::kLocalCoord_GPType: {
                GR_CREATE_STATIC_PROCESSOR(gDefaultGeoProcLoc, DefaultGeoProc, (gpTypeFlags));
                return SkRef(gDefaultGeoProcLoc);
            }
            case Flag::kCoverage_GPType: {
                GR_CREATE_STATIC_PROCESSOR(gDefaultGeoProcCov, DefaultGeoProc, (gpTypeFlags));
                return SkRef(gDefaultGeoProcCov);
            }
            case Flag::kPosition_GPType: {
                GR_CREATE_STATIC_PROCESSOR(gDefaultGeoProc, DefaultGeoProc, (gpTypeFlags));
                return SkRef(gDefaultGeoProc);
            }
            default:
                SkFAIL("Incomplete Switch");
                return NULL;
        }
    }

    virtual const char* name() const SK_OVERRIDE { return "DefaultGeometryProcessor"; }

    const GrAttribute* inPosition() const { return fInPosition; }
    const GrAttribute* inColor() const { return fInColor; }
    const GrAttribute* inLocalCoords() const { return fInLocalCoords; }
    const GrAttribute* inCoverage() const { return fInCoverage; }

    class GLProcessor : public GrGLGeometryProcessor {
    public:
        GLProcessor(const GrGeometryProcessor&,
                    const GrBatchTracker&) {}

        virtual void emitCode(const EmitArgs& args) SK_OVERRIDE {
            const DefaultGeoProc& gp = args.fGP.cast<DefaultGeoProc>();
            GrGLVertexBuilder* vs = args.fPB->getVertexShaderBuilder();

            vs->codeAppendf("%s = %s;", vs->positionCoords(), gp.inPosition()->fName);

            // Setup pass through color
            if (gp.inColor()) {
                args.fPB->addPassThroughAttribute(gp.inColor(), args.fOutputColor);
            }

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
            GrGLGPFragmentBuilder* fs = args.fPB->getFragmentShaderBuilder();
            fs->codeAppendf("float alpha = 1.0;");
            if (gp.inCoverage()) {
                args.fPB->addPassThroughAttribute(gp.inCoverage(), "alpha");
            }
            fs->codeAppendf("%s = vec4(alpha);", args.fOutputCoverage);
        }

        static inline void GenKey(const GrGeometryProcessor& gp,
                                  const GrBatchTracker&,
                                  const GrGLCaps&,
                                  GrProcessorKeyBuilder* b) {
            const DefaultGeoProc& def = gp.cast<DefaultGeoProc>();
            b->add32(def.fFlags);
        }

        virtual void setData(const GrGLProgramDataManager&,
                             const GrGeometryProcessor&,
                             const GrBatchTracker&) SK_OVERRIDE {}

    private:
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
    DefaultGeoProc(uint32_t gpTypeFlags)
        : fInPosition(NULL)
        , fInColor(NULL)
        , fInLocalCoords(NULL)
        , fInCoverage(NULL)
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
            this->setHasVertexCoverage();
        }
    }

    virtual bool onIsEqual(const GrGeometryProcessor& other) const SK_OVERRIDE {
        const DefaultGeoProc& gp = other.cast<DefaultGeoProc>();
        return gp.fFlags == this->fFlags;
    }

    virtual void onComputeInvariantOutput(GrInvariantOutput* inout) const SK_OVERRIDE {
        if (fInCoverage) {
            inout->mulByUnknownAlpha();
        } else {
            inout->mulByKnownAlpha(255);
        }
    }

    const GrAttribute* fInPosition;
    const GrAttribute* fInColor;
    const GrAttribute* fInLocalCoords;
    const GrAttribute* fInCoverage;
    uint32_t fFlags;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
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

    return DefaultGeoProc::Create(flags);
}

const GrGeometryProcessor* GrDefaultGeoProcFactory::Create(uint32_t gpTypeFlags) {
    return DefaultGeoProc::Create(gpTypeFlags);
}
