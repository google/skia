/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDefaultGeoProcFactory.h"

#include "gl/builders/GrGLProgramBuilder.h"
#include "gl/GrGLGeometryProcessor.h"
#include "GrDrawState.h"
#include "GrInvariantOutput.h"
#include "GrTBackendProcessorFactory.h"

/*
 * The default Geometry Processor simply takes position and multiplies it by the uniform view
 * matrix. It also leaves coverage untouched.  Behind the scenes, we may add per vertex color or
 * local coords.
 */
class DefaultGeoProc : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Create() {
        GR_CREATE_STATIC_PROCESSOR(gDefaultGeoProc, DefaultGeoProc, ());
        return SkRef(gDefaultGeoProc);
    }

    static const char* Name() { return "DefaultGeometryProcessor"; }

    virtual const GrBackendGeometryProcessorFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendGeometryProcessorFactory<DefaultGeoProc>::getInstance();
    }

    class GLProcessor : public GrGLGeometryProcessor {
    public:
        GLProcessor(const GrBackendProcessorFactory& factory, const GrProcessor&)
            : INHERITED (factory) {}

        virtual void emitCode(const EmitArgs& args) SK_OVERRIDE {
            GrGLVertexBuilder* vs = args.fPB->getVertexShaderBuilder();

            // setup position varying
            vs->codeAppendf("%s = %s * vec3(%s, 1);", vs->glPosition(), vs->uViewM(),
                            vs->inPosition());

            // output coverage in FS(pass through)
            GrGLGPFragmentBuilder* fs = args.fPB->getFragmentShaderBuilder();
            fs->codeAppendf("%s = %s;", args.fOutput, GrGLSLExpr4(args.fInput).c_str());
        }

        static inline void GenKey(const GrProcessor&, const GrGLCaps&, GrProcessorKeyBuilder*) {}

        virtual void setData(const GrGLProgramDataManager&, const GrProcessor&) SK_OVERRIDE {}

    private:
        typedef GrGLGeometryProcessor INHERITED;
    };

private:
    DefaultGeoProc() {}

    virtual bool onIsEqual(const GrGeometryProcessor& other) const SK_OVERRIDE {
        return true;
    }

    virtual void onComputeInvariantOutput(GrInvariantOutput* inout) const SK_OVERRIDE {
        inout->mulByUnknownAlpha();
    }

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(DefaultGeoProc);

GrGeometryProcessor* DefaultGeoProc::TestCreate(SkRandom* random,
                                                GrContext*,
                                                const GrDrawTargetCaps& caps,
                                                GrTexture*[]) {
    return DefaultGeoProc::Create();
}

// We use these arrays to customize our default GP.  We only need 4 because we omit coverage if
// coverage is not requested in the flags to the create function.
GrVertexAttrib kDefaultPositionGeoProc[] = {
    { kVec2f_GrVertexAttribType, 0,                kPosition_GrVertexAttribBinding },
    { kVec4ub_GrVertexAttribType, sizeof(SkPoint), kCoverage_GrVertexAttribBinding },
};

GrVertexAttrib kDefaultPosColorGeoProc[] = {
    { kVec2f_GrVertexAttribType, 0,                                  kPosition_GrVertexAttribBinding },
    { kVec4ub_GrVertexAttribType, sizeof(SkPoint),                   kColor_GrVertexAttribBinding },
    { kVec4ub_GrVertexAttribType, sizeof(SkPoint) + sizeof(GrColor), kCoverage_GrVertexAttribBinding },
};

GrVertexAttrib kDefaultPosUVGeoProc[] = {
    { kVec2f_GrVertexAttribType, 0,                    kPosition_GrVertexAttribBinding },
    { kVec2f_GrVertexAttribType, sizeof(SkPoint),      kLocalCoord_GrVertexAttribBinding },
    { kVec4ub_GrVertexAttribType, 2 * sizeof(SkPoint), kCoverage_GrVertexAttribBinding },
};

GrVertexAttrib kDefaultPosColUVGeoProc[] = {
    { kVec2f_GrVertexAttribType, 0,                                      kPosition_GrVertexAttribBinding },
    { kVec4ub_GrVertexAttribType, sizeof(SkPoint),                       kColor_GrVertexAttribBinding },
    { kVec2f_GrVertexAttribType, sizeof(SkPoint) + sizeof(GrColor),      kLocalCoord_GrVertexAttribBinding },
    { kVec4ub_GrVertexAttribType, 2 * sizeof(SkPoint) + sizeof(GrColor), kCoverage_GrVertexAttribBinding },
};

static size_t get_size(GrDefaultGeoProcFactory::GPType flag) {
    switch (flag) {
        case GrDefaultGeoProcFactory::kPosition_GPType:
            return GrVertexAttribTypeSize(kVec2f_GrVertexAttribType);
        case GrDefaultGeoProcFactory::kColor_GPType:
            return GrVertexAttribTypeSize(kVec4ub_GrVertexAttribType);
        case GrDefaultGeoProcFactory::kLocalCoord_GPType:
            return GrVertexAttribTypeSize(kVec2f_GrVertexAttribType);
        case GrDefaultGeoProcFactory::kCoverage_GPType:
            return GrVertexAttribTypeSize(kVec4ub_GrVertexAttribType);
        default:
            SkFAIL("Should never get here");
            return 0;
    }
}

const GrGeometryProcessor*
GrDefaultGeoProcFactory::CreateAndSetAttribs(GrDrawState* ds, uint32_t gpTypeFlags) {
    SkASSERT(ds);
    // always atleast position in the GP
    size_t size = get_size(kPosition_GPType);
    int count = 1;

    bool hasColor = SkToBool(gpTypeFlags & kColor_GPType);
    bool hasLocalCoord = SkToBool(gpTypeFlags & kLocalCoord_GPType);
    bool hasCoverage = SkToBool(gpTypeFlags & kCoverage_GPType);

    if (hasColor) {
        size += get_size(kColor_GPType);
        count++;
        if (hasLocalCoord) {
            size += get_size(kLocalCoord_GPType);
            count++;
            if (hasCoverage) {
                size += get_size(kCoverage_GPType);
                count++;
                ds->setVertexAttribs<kDefaultPosColUVGeoProc>(count, size);
            } else {
                ds->setVertexAttribs<kDefaultPosColUVGeoProc>(count, size);

            }
        } else {
            if (hasCoverage) {
                size += get_size(kCoverage_GPType);
                count++;
                ds->setVertexAttribs<kDefaultPosColorGeoProc>(count, size);
            } else {
                ds->setVertexAttribs<kDefaultPosColorGeoProc>(count, size);
            }
        }
    } else if (hasLocalCoord) {
        size += get_size(kLocalCoord_GPType);
        count++;
        if (hasCoverage) {
            size += get_size(kCoverage_GPType);
            count++;
            ds->setVertexAttribs<kDefaultPosUVGeoProc>(count, size);
        } else {
            ds->setVertexAttribs<kDefaultPosUVGeoProc>(count, size);
        }
    } else if (hasCoverage) {
        size += get_size(kCoverage_GPType);
        count++;
        ds->setVertexAttribs<kDefaultPositionGeoProc>(count, size);
    } else {
        // Just position
        ds->setVertexAttribs<kDefaultPositionGeoProc>(count, size);
    }
    return DefaultGeoProc::Create();
}

const GrGeometryProcessor* GrDefaultGeoProcFactory::Create() {
    return DefaultGeoProc::Create();
}
