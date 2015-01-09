/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBitmapTextGeoProc.h"
#include "GrInvariantOutput.h"
#include "GrTexture.h"
#include "gl/GrGLProcessor.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "gl/GrGLGeometryProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"

struct BitmapTextBatchTracker {
    GrGPInput fInputColorType;
    GrColor fColor;
    bool fUsesLocalCoords;
};

class GrGLBitmapTextGeoProc : public GrGLGeometryProcessor {
public:
    GrGLBitmapTextGeoProc(const GrGeometryProcessor&, const GrBatchTracker&)
        : fColor(GrColor_ILLEGAL) {}

    void emitCode(const EmitArgs& args) SK_OVERRIDE {
        const GrBitmapTextGeoProc& cte = args.fGP.cast<GrBitmapTextGeoProc>();
        const BitmapTextBatchTracker& local = args.fBT.cast<BitmapTextBatchTracker>();

        GrGLGPBuilder* pb = args.fPB;
        GrGLVertexBuilder* vsBuilder = pb->getVertexShaderBuilder();

        GrGLVertToFrag v(kVec2f_GrSLType);
        pb->addVarying("TextureCoords", &v);
        vsBuilder->codeAppendf("%s = %s;", v.vsOut(), cte.inTextureCoords()->fName);

        // Setup pass through color
        this->setupColorPassThrough(pb, local.fInputColorType, args.fOutputColor, cte.inColor(),
                                    &fColorUniform);

        // setup output coords
        vsBuilder->codeAppendf("%s = %s;", vsBuilder->positionCoords(), cte.inPosition()->fName);
        vsBuilder->codeAppendf("%s = %s;", vsBuilder->localCoords(), cte.inPosition()->fName);

        // setup uniform viewMatrix
        this->addUniformViewMatrix(pb);

        // setup position varying
        vsBuilder->codeAppendf("%s = %s * vec3(%s, 1);", vsBuilder->glPosition(), this->uViewM(),
                               cte.inPosition()->fName);

        GrGLGPFragmentBuilder* fsBuilder = pb->getFragmentShaderBuilder();
        fsBuilder->codeAppendf("%s = ", args.fOutputCoverage);
        fsBuilder->appendTextureLookup(args.fSamplers[0], v.fsIn(), kVec2f_GrSLType);
        fsBuilder->codeAppend(";");
    }

    virtual void setData(const GrGLProgramDataManager& pdman,
                         const GrPrimitiveProcessor& gp,
                         const GrBatchTracker& bt) SK_OVERRIDE {
        this->setUniformViewMatrix(pdman, gp.viewMatrix());

        const BitmapTextBatchTracker& local = bt.cast<BitmapTextBatchTracker>();
        if (kUniform_GrGPInput == local.fInputColorType && local.fColor != fColor) {
            GrGLfloat c[4];
            GrColorToRGBAFloat(local.fColor, c);
            pdman.set4fv(fColorUniform, 1, c);
            fColor = local.fColor;
        }
    }

    static inline void GenKey(const GrGeometryProcessor& proc,
                              const GrBatchTracker& bt,
                              const GrGLCaps&,
                              GrProcessorKeyBuilder* b) {
        const BitmapTextBatchTracker& local = bt.cast<BitmapTextBatchTracker>();
        // We have to put the optional vertex attribute as part of the key.  See the comment
        // on addVertexAttrib.
        // TODO When we have deferred geometry we can fix this
        const GrBitmapTextGeoProc& gp = proc.cast<GrBitmapTextGeoProc>();
        uint32_t key = 0;
        key |= SkToBool(gp.inColor()) ? 0x1 : 0x0;
        key |= local.fUsesLocalCoords && proc.localMatrix().hasPerspective() ? 0x2 : 0x0;
        b->add32(local.fInputColorType << 16 | key);
    }

private:
    GrColor fColor;
    UniformHandle fColorUniform;

    typedef GrGLGeometryProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrBitmapTextGeoProc::GrBitmapTextGeoProc(GrColor color, GrTexture* texture,
                                         const GrTextureParams& params, bool useColorAttrib,
                                         bool opaqueVertexColors, const SkMatrix& localMatrix)
    : INHERITED(color, SkMatrix::I(), localMatrix, opaqueVertexColors)
    , fTextureAccess(texture, params)
    , fInColor(NULL) {
    this->initClassID<GrBitmapTextGeoProc>();
    fInPosition = &this->addVertexAttrib(GrAttribute("inPosition", kVec2f_GrVertexAttribType));
    if (useColorAttrib) {
        fInColor = &this->addVertexAttrib(GrAttribute("inColor", kVec4ub_GrVertexAttribType));
        this->setHasVertexColor();
    }
    fInTextureCoords = &this->addVertexAttrib(GrAttribute("inTextureCoords",
                                                          kVec2f_GrVertexAttribType));
    this->addTextureAccess(&fTextureAccess);
}

bool GrBitmapTextGeoProc::onIsEqual(const GrGeometryProcessor& other) const {
    const GrBitmapTextGeoProc& gp = other.cast<GrBitmapTextGeoProc>();
    return SkToBool(this->inColor()) == SkToBool(gp.inColor());
}

void GrBitmapTextGeoProc::onGetInvariantOutputCoverage(GrInitInvariantOutput* out) const {
    if (GrPixelConfigIsAlphaOnly(this->texture(0)->config())) {
        out->setUnknownSingleComponent();
    } else if (GrPixelConfigIsOpaque(this->texture(0)->config())) {
        out->setUnknownOpaqueFourComponents();
        out->setUsingLCDCoverage();
    } else {
        out->setUnknownFourComponents();
        out->setUsingLCDCoverage();
    }
}

void GrBitmapTextGeoProc::getGLProcessorKey(const GrBatchTracker& bt,
                                            const GrGLCaps& caps,
                                            GrProcessorKeyBuilder* b) const {
    GrGLBitmapTextGeoProc::GenKey(*this, bt, caps, b);
}

GrGLGeometryProcessor*
GrBitmapTextGeoProc::createGLInstance(const GrBatchTracker& bt) const {
    return SkNEW_ARGS(GrGLBitmapTextGeoProc, (*this, bt));
}

void GrBitmapTextGeoProc::initBatchTracker(GrBatchTracker* bt, const InitBT& init) const {
    BitmapTextBatchTracker* local = bt->cast<BitmapTextBatchTracker>();
    local->fInputColorType = GetColorInputType(&local->fColor, this->color(), init,
                                               SkToBool(fInColor));
    local->fUsesLocalCoords = init.fUsesLocalCoords;
}

bool GrBitmapTextGeoProc::onCanMakeEqual(const GrBatchTracker& m,
                                         const GrGeometryProcessor& that,
                                         const GrBatchTracker& t) const {
    const BitmapTextBatchTracker& mine = m.cast<BitmapTextBatchTracker>();
    const BitmapTextBatchTracker& theirs = t.cast<BitmapTextBatchTracker>();
    return CanCombineLocalMatrices(*this, mine.fUsesLocalCoords,
                                   that, theirs.fUsesLocalCoords) &&
           CanCombineOutput(mine.fInputColorType, mine.fColor,
                            theirs.fInputColorType, theirs.fColor);
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrBitmapTextGeoProc);

GrGeometryProcessor* GrBitmapTextGeoProc::TestCreate(SkRandom* random,
                                                     GrContext*,
                                                     const GrDrawTargetCaps&,
                                                     GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                      GrProcessorUnitTest::kAlphaTextureIdx;
    static const SkShader::TileMode kTileModes[] = {
        SkShader::kClamp_TileMode,
        SkShader::kRepeat_TileMode,
        SkShader::kMirror_TileMode,
    };
    SkShader::TileMode tileModes[] = {
        kTileModes[random->nextULessThan(SK_ARRAY_COUNT(kTileModes))],
        kTileModes[random->nextULessThan(SK_ARRAY_COUNT(kTileModes))],
    };
    GrTextureParams params(tileModes, random->nextBool() ? GrTextureParams::kBilerp_FilterMode :
                                                           GrTextureParams::kNone_FilterMode);

    return GrBitmapTextGeoProc::Create(GrRandomColor(random), textures[texIdx], params,
                                       random->nextBool(), random->nextBool(),
                                       GrProcessorUnitTest::TestMatrix(random));
}
