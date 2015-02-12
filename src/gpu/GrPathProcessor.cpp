#include "GrPathProcessor.h"

#include "gl/GrGLPathProcessor.h"
#include "gl/GrGLGpu.h"

GrPathProcessor::GrPathProcessor(GrColor color,
                                 const SkMatrix& viewMatrix,
                                 const SkMatrix& localMatrix)
    : INHERITED(viewMatrix, localMatrix, true)
    , fColor(color) {
    this->initClassID<GrPathProcessor>();
}

void GrPathProcessor::getInvariantOutputColor(GrInitInvariantOutput* out) const {
    out->setKnownFourComponents(fColor);
}

void GrPathProcessor::getInvariantOutputCoverage(GrInitInvariantOutput* out) const {
    out->setKnownSingleComponent(0xff);
}

void GrPathProcessor::initBatchTracker(GrBatchTracker* bt, const GrPipelineInfo& init) const {
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

GrGLPrimitiveProcessor* GrPathProcessor::createGLInstance(const GrBatchTracker& bt,
                                                          const GrGLCaps& caps) const {
    SkASSERT(caps.nvprSupport() != GrGLCaps::kNone_NvprSupport);
    if (caps.nvprSupport() == GrGLCaps::kLegacy_NvprSupport) {
        return SkNEW_ARGS(GrGLLegacyPathProcessor, (*this, bt,
                                                    caps.maxFixedFunctionTextureCoords()));
    } else {
        return SkNEW_ARGS(GrGLNormalPathProcessor, (*this, bt));
    }
}
