/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRODrawState.h"

#include "GrDrawTargetCaps.h"
#include "GrRenderTarget.h"

////////////////////////////////////////////////////////////////////////////////

GrRODrawState::GrRODrawState(const GrRODrawState& drawState) : INHERITED() {
    fRenderTarget.setResource(SkSafeRef(drawState.fRenderTarget.getResource()),
                              GrIORef::kWrite_IOType);
}

bool GrRODrawState::isEqual(const GrRODrawState& that) const {
    bool usingVertexColors = this->hasColorVertexAttribute();
    if (!usingVertexColors && this->fColor != that.fColor) {
        return false;
    }

    if (this->getRenderTarget() != that.getRenderTarget() ||
        this->fColorStages.count() != that.fColorStages.count() ||
        this->fCoverageStages.count() != that.fCoverageStages.count() ||
        !this->fViewMatrix.cheapEqualTo(that.fViewMatrix) ||
        this->fSrcBlend != that.fSrcBlend ||
        this->fDstBlend != that.fDstBlend ||
        this->fBlendConstant != that.fBlendConstant ||
        this->fFlagBits != that.fFlagBits ||
        this->fVACount != that.fVACount ||
        this->fVAStride != that.fVAStride ||
        memcmp(this->fVAPtr, that.fVAPtr, this->fVACount * sizeof(GrVertexAttrib)) ||
        this->fStencilSettings != that.fStencilSettings ||
        this->fDrawFace != that.fDrawFace) {
        return false;
    }

    bool usingVertexCoverage = this->hasCoverageVertexAttribute();
    if (!usingVertexCoverage && this->fCoverage != that.fCoverage) {
        return false;
    }

    bool explicitLocalCoords = this->hasLocalCoordAttribute();
    if (this->hasGeometryProcessor()) {
        if (!that.hasGeometryProcessor()) {
            return false;
        } else if (!GrProcessorStage::AreCompatible(*this->getGeometryProcessor(),
                                                    *that.getGeometryProcessor(),
                                                    explicitLocalCoords)) {
            return false;
        }
    } else if (that.hasGeometryProcessor()) {
        return false;
    }

    for (int i = 0; i < this->numColorStages(); i++) {
        if (!GrProcessorStage::AreCompatible(this->getColorStage(i), that.getColorStage(i),
                                             explicitLocalCoords)) {
            return false;
        }
    }
    for (int i = 0; i < this->numCoverageStages(); i++) {
        if (!GrProcessorStage::AreCompatible(this->getCoverageStage(i), that.getCoverageStage(i),
                                             explicitLocalCoords)) {
            return false;
        }
    }

    SkASSERT(0 == memcmp(this->fFixedFunctionVertexAttribIndices,
                            that.fFixedFunctionVertexAttribIndices,
                            sizeof(this->fFixedFunctionVertexAttribIndices)));

    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool GrRODrawState::validateVertexAttribs() const {
    // check consistency of effects and attributes
    GrSLType slTypes[kMaxVertexAttribCnt];
    for (int i = 0; i < kMaxVertexAttribCnt; ++i) {
        slTypes[i] = static_cast<GrSLType>(-1);
    }

    if (this->hasGeometryProcessor()) {
        const GrGeometryStage& stage = *this->getGeometryProcessor();
        const GrGeometryProcessor* gp = stage.getGeometryProcessor();
        SkASSERT(gp);
        // make sure that any attribute indices have the correct binding type, that the attrib
        // type and effect's shader lang type are compatible, and that attributes shared by
        // multiple effects use the same shader lang type.
        const GrGeometryProcessor::VertexAttribArray& s = gp->getVertexAttribs();

        int effectIndex = 0;
        for (int index = 0; index < fVACount; index++) {
            if (kGeometryProcessor_GrVertexAttribBinding != fVAPtr[index].fBinding) {
                // we only care about effect bindings
                continue;
            }
            SkASSERT(effectIndex < s.count());
            GrSLType effectSLType = s[effectIndex].getType();
            GrVertexAttribType attribType = fVAPtr[index].fType;
            int slVecCount = GrSLTypeVectorCount(effectSLType);
            int attribVecCount = GrVertexAttribTypeVectorCount(attribType);
            if (slVecCount != attribVecCount ||
                (static_cast<GrSLType>(-1) != slTypes[index] && slTypes[index] != effectSLType)) {
                return false;
            }
            slTypes[index] = effectSLType;
            effectIndex++;
        }
        // Make sure all attributes are consumed and we were able to find everything
        SkASSERT(s.count() == effectIndex);
    }

    return true;
}

bool GrRODrawState::hasSolidCoverage() const {
    // If we're drawing coverage directly then coverage is effectively treated as color.
    if (this->isCoverageDrawing()) {
        return true;
    }

    GrColor coverage;
    uint32_t validComponentFlags;
    // Initialize to an unknown starting coverage if per-vertex coverage is specified.
    if (this->hasCoverageVertexAttribute()) {
        validComponentFlags = 0;
    } else {
        coverage = fCoverage;
        validComponentFlags = kRGBA_GrColorComponentFlags;
    }

    // Run through the coverage stages and see if the coverage will be all ones at the end.
    if (this->hasGeometryProcessor()) {
        const GrGeometryProcessor* gp = fGeometryProcessor->getGeometryProcessor();
        gp->getConstantColorComponents(&coverage, &validComponentFlags);
    }
    for (int s = 0; s < this->numCoverageStages(); ++s) {
        const GrProcessor* processor = this->getCoverageStage(s).getProcessor();
        processor->getConstantColorComponents(&coverage, &validComponentFlags);
    }
    return (kRGBA_GrColorComponentFlags == validComponentFlags) && (0xffffffff == coverage);
}

////////////////////////////////////////////////////////////////////////////////

bool GrRODrawState::willEffectReadDstColor() const {
    if (!this->isColorWriteDisabled()) {
        for (int s = 0; s < this->numColorStages(); ++s) {
            if (this->getColorStage(s).getFragmentProcessor()->willReadDstColor()) {
                return true;
            }
        }
    }
    for (int s = 0; s < this->numCoverageStages(); ++s) {
        if (this->getCoverageStage(s).getFragmentProcessor()->willReadDstColor()) {
            return true;
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

GrRODrawState::BlendOptFlags GrRODrawState::getBlendOpts(bool forceCoverage,
                                                         GrBlendCoeff* srcCoeff,
                                                         GrBlendCoeff* dstCoeff) const {
    GrBlendCoeff bogusSrcCoeff, bogusDstCoeff;
    if (NULL == srcCoeff) {
        srcCoeff = &bogusSrcCoeff;
    }
    if (NULL == dstCoeff) {
        dstCoeff = &bogusDstCoeff;
    }

    *srcCoeff = this->getSrcBlendCoeff();
    *dstCoeff = this->getDstBlendCoeff();

    if (this->isColorWriteDisabled()) {
        *srcCoeff = kZero_GrBlendCoeff;
        *dstCoeff = kOne_GrBlendCoeff;
    }

    bool srcAIsOne = this->srcAlphaWillBeOne();
    bool dstCoeffIsOne = kOne_GrBlendCoeff == *dstCoeff ||
                         (kSA_GrBlendCoeff == *dstCoeff && srcAIsOne);
    bool dstCoeffIsZero = kZero_GrBlendCoeff == *dstCoeff ||
                         (kISA_GrBlendCoeff == *dstCoeff && srcAIsOne);

    // When coeffs are (0,1) there is no reason to draw at all, unless
    // stenciling is enabled. Having color writes disabled is effectively
    // (0,1).
    if ((kZero_GrBlendCoeff == *srcCoeff && dstCoeffIsOne)) {
        if (this->getStencil().doesWrite()) {
            return kEmitCoverage_BlendOptFlag;
        } else {
            *dstCoeff = kOne_GrBlendCoeff;
            return kSkipDraw_BlendOptFlag;
        }
    }

    bool hasCoverage = forceCoverage || !this->hasSolidCoverage();

    // if we don't have coverage we can check whether the dst
    // has to read at all. If not, we'll disable blending.
    if (!hasCoverage) {
        if (dstCoeffIsZero) {
            if (kOne_GrBlendCoeff == *srcCoeff) {
                // if there is no coverage and coeffs are (1,0) then we
                // won't need to read the dst at all, it gets replaced by src
                *dstCoeff = kZero_GrBlendCoeff;
                return kNone_BlendOpt;
            } else if (kZero_GrBlendCoeff == *srcCoeff) {
                // if the op is "clear" then we don't need to emit a color
                // or blend, just write transparent black into the dst.
                *srcCoeff = kOne_GrBlendCoeff;
                *dstCoeff = kZero_GrBlendCoeff;
                return kEmitTransBlack_BlendOptFlag;
            }
        }
    } else if (this->isCoverageDrawing()) {
        // we have coverage but we aren't distinguishing it from alpha by request.
        return kCoverageAsAlpha_BlendOptFlag;
    } else {
        // check whether coverage can be safely rolled into alpha
        // of if we can skip color computation and just emit coverage
        if (this->canTweakAlphaForCoverage()) {
            return kCoverageAsAlpha_BlendOptFlag;
        }
        if (dstCoeffIsZero) {
            if (kZero_GrBlendCoeff == *srcCoeff) {
                // the source color is not included in the blend
                // the dst coeff is effectively zero so blend works out to:
                // (c)(0)D + (1-c)D = (1-c)D.
                *dstCoeff = kISA_GrBlendCoeff;
                return  kEmitCoverage_BlendOptFlag;
            } else if (srcAIsOne) {
                // the dst coeff is effectively zero so blend works out to:
                // cS + (c)(0)D + (1-c)D = cS + (1-c)D.
                // If Sa is 1 then we can replace Sa with c
                // and set dst coeff to 1-Sa.
                *dstCoeff = kISA_GrBlendCoeff;
                return  kCoverageAsAlpha_BlendOptFlag;
            }
        } else if (dstCoeffIsOne) {
            // the dst coeff is effectively one so blend works out to:
            // cS + (c)(1)D + (1-c)D = cS + D.
            *dstCoeff = kOne_GrBlendCoeff;
            return  kCoverageAsAlpha_BlendOptFlag;
        }
    }

    return kNone_BlendOpt;
}

////////////////////////////////////////////////////////////////////////////////

// Some blend modes allow folding a fractional coverage value into the color's alpha channel, while
// others will blend incorrectly.
bool GrRODrawState::canTweakAlphaForCoverage() const {
    /*
     The fractional coverage is f.
     The src and dst coeffs are Cs and Cd.
     The dst and src colors are S and D.
     We want the blend to compute: f*Cs*S + (f*Cd + (1-f))D. By tweaking the source color's alpha
     we're replacing S with S'=fS. It's obvious that that first term will always be ok. The second
     term can be rearranged as [1-(1-Cd)f]D. By substituting in the various possibilities for Cd we
     find that only 1, ISA, and ISC produce the correct destination when applied to S' and D.
     Also, if we're directly rendering coverage (isCoverageDrawing) then coverage is treated as
     color by definition.
     */
    return kOne_GrBlendCoeff == fDstBlend ||
           kISA_GrBlendCoeff == fDstBlend ||
           kISC_GrBlendCoeff == fDstBlend ||
           this->isCoverageDrawing();
}

void GrRODrawState::convertToPendingExec() {
    fRenderTarget.markPendingIO();
    fRenderTarget.removeRef();
    for (int i = 0; i < fColorStages.count(); ++i) {
        fColorStages[i].convertToPendingExec();
    }
    if (fGeometryProcessor) {
        fGeometryProcessor->convertToPendingExec();
    }
    for (int i = 0; i < fCoverageStages.count(); ++i) {
        fCoverageStages[i].convertToPendingExec();
    }
}

bool GrRODrawState::srcAlphaWillBeOne() const {
    uint32_t validComponentFlags;
    GrColor color;
    // Check if per-vertex or constant color may have partial alpha
    if (this->hasColorVertexAttribute()) {
        if (fHints & kVertexColorsAreOpaque_Hint) {
            validComponentFlags = kA_GrColorComponentFlag;
            color = 0xFF << GrColor_SHIFT_A;
        } else {
            validComponentFlags = 0;
            color = 0; // not strictly necessary but we get false alarms from tools about uninit.
        }
    } else {
        validComponentFlags = kRGBA_GrColorComponentFlags;
        color = this->getColor();
    }

    // Run through the color stages
    for (int s = 0; s < this->numColorStages(); ++s) {
        const GrProcessor* processor = this->getColorStage(s).getProcessor();
        processor->getConstantColorComponents(&color, &validComponentFlags);
    }

    // Check whether coverage is treated as color. If so we run through the coverage computation.
    if (this->isCoverageDrawing()) {
        // The shader generated for coverage drawing runs the full coverage computation and then
        // makes the shader output be the multiplication of color and coverage. We mirror that here.
        GrColor coverage;
        uint32_t coverageComponentFlags;
        if (this->hasCoverageVertexAttribute()) {
            coverageComponentFlags = 0;
            coverage = 0; // suppresses any warnings.
        } else {
            coverageComponentFlags = kRGBA_GrColorComponentFlags;
            coverage = this->getCoverageColor();
        }

        // Run through the coverage stages
        for (int s = 0; s < this->numCoverageStages(); ++s) {
            const GrProcessor* processor = this->getCoverageStage(s).getProcessor();
            processor->getConstantColorComponents(&coverage, &coverageComponentFlags);
        }

        // Since the shader will multiply coverage and color, the only way the final A==1 is if
        // coverage and color both have A==1.
        return (kA_GrColorComponentFlag & validComponentFlags & coverageComponentFlags) &&
                0xFF == GrColorUnpackA(color) && 0xFF == GrColorUnpackA(coverage);

    }

    return (kA_GrColorComponentFlag & validComponentFlags) && 0xFF == GrColorUnpackA(color);
}

