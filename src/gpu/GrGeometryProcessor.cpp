/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGeometryProcessor.h"

#include "gl/GrGLGeometryProcessor.h"
#include "GrInvariantOutput.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

void GrGeometryProcessor::getInvariantOutputColor(GrInitInvariantOutput* out) const {
    if (fHasVertexColor) {
        if (fOpaqueVertexColors) {
            out->setUnknownOpaqueFourComponents();
        } else {
            out->setUnknownFourComponents();
        }
    } else {
        out->setKnownFourComponents(fColor);
    }
    this->onGetInvariantOutputColor(out);
}

void GrGeometryProcessor::getInvariantOutputCoverage(GrInitInvariantOutput* out) const {
    this->onGetInvariantOutputCoverage(out);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "gl/builders/GrGLProgramBuilder.h"

void GrGLGeometryProcessor::setupColorPassThrough(GrGLGPBuilder* pb,
                                                  GrGPInput inputType,
                                                  const char* outputName,
                                                  const GrGeometryProcessor::GrAttribute* colorAttr,
                                                  UniformHandle* colorUniform) {
    GrGLGPFragmentBuilder* fs = pb->getFragmentShaderBuilder();
    if (kUniform_GrGPInput == inputType) {
        SkASSERT(colorUniform);
        const char* stagedLocalVarName;
        *colorUniform = pb->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                       kVec4f_GrSLType,
                                       kDefault_GrSLPrecision,
                                       "Color",
                                       &stagedLocalVarName);
        fs->codeAppendf("%s = %s;", outputName, stagedLocalVarName);
    } else if (kAttribute_GrGPInput == inputType) {
        SkASSERT(colorAttr);
        pb->addPassThroughAttribute(colorAttr, outputName);
    } else if (kAllOnes_GrGPInput == inputType) {
        fs->codeAppendf("%s = vec4(1);", outputName);
    }
}

void GrGLGeometryProcessor::addUniformViewMatrix(GrGLGPBuilder* pb) {
    fViewMatrixUniform = pb->addUniform(GrGLProgramBuilder::kVertex_Visibility,
                                        kMat33f_GrSLType, kDefault_GrSLPrecision,
                                        "uViewM",
                                        &fViewMatrixName);
}

void GrGLGeometryProcessor::setUniformViewMatrix(const GrGLProgramDataManager& pdman,
                                                 const SkMatrix& viewMatrix) {
    if (!fViewMatrix.cheapEqualTo(viewMatrix)) {
        SkASSERT(fViewMatrixUniform.isValid());
        fViewMatrix = viewMatrix;

        GrGLfloat viewMatrix[3 * 3];
        GrGLGetMatrix<3>(viewMatrix, fViewMatrix);
        pdman.setMatrix3f(fViewMatrixUniform, viewMatrix);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

struct PathBatchTracker {
    GrGPInput fInputColorType;
    GrGPInput fInputCoverageType;
    GrColor fColor;
    bool fUsesLocalCoords;
};

class GrGLPathProcessor : public GrGLGeometryProcessor {
public:
    GrGLPathProcessor(const GrPathProcessor&, const GrBatchTracker&)
        : fColor(GrColor_ILLEGAL) {}

    void emitCode(const EmitArgs& args) SK_OVERRIDE {
        GrGLGPBuilder* pb = args.fPB;
        GrGLGPFragmentBuilder* fs = args.fPB->getFragmentShaderBuilder();
        const PathBatchTracker& local = args.fBT.cast<PathBatchTracker>();

        // Setup uniform color
        if (kUniform_GrGPInput == local.fInputColorType) {
            const char* stagedLocalVarName;
            fColorUniform = pb->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                           kVec4f_GrSLType,
                                           kDefault_GrSLPrecision,
                                           "Color",
                                           &stagedLocalVarName);
            fs->codeAppendf("%s = %s;", args.fOutputColor, stagedLocalVarName);
        }

        // setup constant solid coverage
        if (kAllOnes_GrGPInput == local.fInputCoverageType) {
            fs->codeAppendf("%s = vec4(1);", args.fOutputCoverage);
        }
    }

    static inline void GenKey(const GrPathProcessor&,
                              const GrBatchTracker& bt,
                              const GrGLCaps&,
                              GrProcessorKeyBuilder* b) {
        const PathBatchTracker& local = bt.cast<PathBatchTracker>();
        b->add32(local.fInputColorType | local.fInputCoverageType << 16);
    }

    void setData(const GrGLProgramDataManager& pdman,
                 const GrPrimitiveProcessor& primProc,
                 const GrBatchTracker& bt) SK_OVERRIDE {
        const PathBatchTracker& local = bt.cast<PathBatchTracker>();
        if (kUniform_GrGPInput == local.fInputColorType && local.fColor != fColor) {
            GrGLfloat c[4];
            GrColorToRGBAFloat(local.fColor, c);
            pdman.set4fv(fColorUniform, 1, c);
            fColor = local.fColor;
        }
    }

private:
    UniformHandle fColorUniform;
    GrColor fColor;

    typedef GrGLGeometryProcessor INHERITED;
};

GrPathProcessor::GrPathProcessor(GrColor color,
                                 const SkMatrix& viewMatrix,
                                 const SkMatrix& localMatrix)
    : INHERITED(viewMatrix, localMatrix)
    , fColor(color) {
    this->initClassID<GrPathProcessor>();
}

void GrPathProcessor::getInvariantOutputColor(GrInitInvariantOutput* out) const {
    out->setKnownFourComponents(fColor);
}

void GrPathProcessor::getInvariantOutputCoverage(GrInitInvariantOutput* out) const {
    out->setKnownSingleComponent(0xff);
}

void GrPathProcessor::initBatchTracker(GrBatchTracker* bt, const InitBT& init) const {
    PathBatchTracker* local = bt->cast<PathBatchTracker>();
    if (init.fColorIgnored) {
        local->fInputColorType = kIgnored_GrGPInput;
        local->fColor = GrColor_ILLEGAL;
    } else {
        local->fInputColorType = kUniform_GrGPInput;
        local->fColor = GrColor_ILLEGAL == init.fOverrideColor ? this->color() :
                                                                 init.fOverrideColor;
    }

    local->fInputCoverageType = init.fCoverageIgnored ? kIgnored_GrGPInput : kAllOnes_GrGPInput;
    local->fUsesLocalCoords = init.fUsesLocalCoords;
}

bool GrPathProcessor::canMakeEqual(const GrBatchTracker& m,
                                   const GrPrimitiveProcessor& that,
                                   const GrBatchTracker& t) const {
    if (this->classID() != that.classID() || !this->hasSameTextureAccesses(that)) {
        return false;
    }

    if (!this->viewMatrix().cheapEqualTo(that.viewMatrix())) {
        return false;
    }

    const PathBatchTracker& mine = m.cast<PathBatchTracker>();
    const PathBatchTracker& theirs = t.cast<PathBatchTracker>();
    return CanCombineLocalMatrices(*this, mine.fUsesLocalCoords,
                                   that, theirs.fUsesLocalCoords) &&
           CanCombineOutput(mine.fInputColorType, mine.fColor,
                            theirs.fInputColorType, theirs.fColor) &&
           CanCombineOutput(mine.fInputCoverageType, 0xff,
                            theirs.fInputCoverageType, 0xff);
}

void GrPathProcessor::getGLProcessorKey(const GrBatchTracker& bt,
                                        const GrGLCaps& caps,
                                        GrProcessorKeyBuilder* b) const {
    GrGLPathProcessor::GenKey(*this, bt, caps, b);
}

GrGLGeometryProcessor* GrPathProcessor::createGLInstance(const GrBatchTracker& bt) const {
    return SkNEW_ARGS(GrGLPathProcessor, (*this, bt));
}
