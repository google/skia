/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRODrawState.h"
#include "GrDrawTargetCaps.h"

////////////////////////////////////////////////////////////////////////////////

bool GrRODrawState::isEqual(const GrRODrawState& that) const {
    bool usingVertexColors = this->hasColorVertexAttribute();
    if (!usingVertexColors && this->fColor != that.fColor) {
        return false;
    }

    if (this->fRenderTarget.get() != that.fRenderTarget.get() ||
        this->fColorStages.count() != that.fColorStages.count() ||
        this->fCoverageStages.count() != that.fCoverageStages.count() ||
        !this->fViewMatrix.cheapEqualTo(that.fViewMatrix) ||
        this->fSrcBlend != that.fSrcBlend ||
        this->fDstBlend != that.fDstBlend ||
        this->fBlendConstant != that.fBlendConstant ||
        this->fFlagBits != that.fFlagBits ||
        this->fVACount != that.fVACount ||
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
    for (int i = 0; i < this->numColorStages(); i++) {
        if (!GrEffectStage::AreCompatible(this->getColorStage(i), that.getColorStage(i),
                                          explicitLocalCoords)) {
            return false;
        }
    }
    for (int i = 0; i < this->numCoverageStages(); i++) {
        if (!GrEffectStage::AreCompatible(this->getCoverageStage(i), that.getCoverageStage(i),
                                          explicitLocalCoords)) {
            return false;
        }
    }

    SkASSERT(this->fVertexSize == that.fVertexSize);
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
    int totalStages = this->numTotalStages();
    for (int s = 0; s < totalStages; ++s) {
        int covIdx = s - this->numColorStages();
        const GrEffectStage& stage = covIdx < 0 ? this->getColorStage(s) :
                                                  this->getCoverageStage(covIdx);
        const GrEffect* effect = stage.getEffect();
        SkASSERT(NULL != effect);
        // make sure that any attribute indices have the correct binding type, that the attrib
        // type and effect's shader lang type are compatible, and that attributes shared by
        // multiple effects use the same shader lang type.
        const int* attributeIndices = stage.getVertexAttribIndices();
        int numAttributes = stage.getVertexAttribIndexCount();
        for (int i = 0; i < numAttributes; ++i) {
            int attribIndex = attributeIndices[i];
            if (attribIndex >= fVACount ||
                kEffect_GrVertexAttribBinding != fVAPtr[attribIndex].fBinding) {
                return false;
            }

            GrSLType effectSLType = effect->vertexAttribType(i);
            GrVertexAttribType attribType = fVAPtr[attribIndex].fType;
            int slVecCount = GrSLTypeVectorCount(effectSLType);
            int attribVecCount = GrVertexAttribTypeVectorCount(attribType);
            if (slVecCount != attribVecCount ||
                (static_cast<GrSLType>(-1) != slTypes[attribIndex] &&
                    slTypes[attribIndex] != effectSLType)) {
                return false;
            }
            slTypes[attribIndex] = effectSLType;
        }
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
    for (int s = 0; s < this->numCoverageStages(); ++s) {
        const GrEffect* effect = this->getCoverageStage(s).getEffect();
        effect->getConstantColorComponents(&coverage, &validComponentFlags);
    }
    return (kRGBA_GrColorComponentFlags == validComponentFlags) && (0xffffffff == coverage);
}

////////////////////////////////////////////////////////////////////////////////

bool GrRODrawState::willEffectReadDstColor() const {
    if (!this->isColorWriteDisabled()) {
        for (int s = 0; s < this->numColorStages(); ++s) {
            if (this->getColorStage(s).getEffect()->willReadDstColor()) {
                return true;
            }
        }
    }
    for (int s = 0; s < this->numCoverageStages(); ++s) {
        if (this->getCoverageStage(s).getEffect()->willReadDstColor()) {
            return true;
        }
    }
    return false;
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

