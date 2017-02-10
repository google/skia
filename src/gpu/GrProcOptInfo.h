/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProcOptInfo_DEFINED
#define GrProcOptInfo_DEFINED

#include "GrColor.h"
#include "GrInvariantOutput.h"

class GrDrawOp;
class GrFragmentProcessor;
class GrPrimitiveProcessor;

/**
 * GrProcOptInfo gathers invariant data from a set of processor stages.It is used to recognize
 * optimizations related to eliminating stages and vertex attributes that aren't necessary for a
 * draw.
 */
class GrProcOptInfo {
public:
    GrProcOptInfo() { this->reset(0, kNone_GrColorComponentFlags); }

    GrProcOptInfo(GrColor color, GrColorComponentFlags colorFlags) {
        this->reset(color, colorFlags);
    }

    void resetToLCDCoverage(GrColor color, GrColorComponentFlags colorFlags) {
        this->internalReset(color, colorFlags, true);
    }

    void reset(GrColor color, GrColorComponentFlags colorFlags) {
        this->internalReset(color, colorFlags, false);
    }

    void reset(const GrPipelineInput& input) {
        this->internalReset(input.fColor, input.fValidFlags, input.fIsLCDCoverage);
    }

    /**
     * Runs through a series of processors and updates calculated values. This can be called
     * repeatedly for cases when the sequence of processors is not in a contiguous array.
     */
    void analyzeProcessors(const GrFragmentProcessor* const* processors, int cnt);

    bool isSolidWhite() const {
        return fProcessorsVisitedWithKnownOutput == fTotalProcessorsVisited &&
               fLastKnownOutputColor == GrColor4f::OpaqueWhite();
    }
    bool isOpaque() const { return fIsOpaque; }
    bool allProcessorsModulateByPremul() const { return fAllProcessorsModulatePremul; }
    bool isLCDCoverage() const { return fIsLCDCoverage; }

    /**
     * If we detected that the result after the first N processors is a known color then we
     * eliminate those N processors and replace the GrDrawOp's color input to the GrPipeline with
     * the known output of the Nth processor, so that the Nth+1 fragment processor (or the XP if
     * there are only N processors) sees its expected input. If this returns 0 then there are no
     * processors to eliminate.
     */
    int initialProcessorsToEliminate(GrColor* newPipelineInputColor) const {
        if (fProcessorsVisitedWithKnownOutput > 0) {
            *newPipelineInputColor = fLastKnownOutputColor.toGrColor();
        }
        return SkTMax(0, fProcessorsVisitedWithKnownOutput);
    }
    int initialProcessorsToEliminate(GrColor4f* newPipelineInputColor) const {
        if (fProcessorsVisitedWithKnownOutput > 0) {
            *newPipelineInputColor = fLastKnownOutputColor;
        }
        return SkTMax(0, fProcessorsVisitedWithKnownOutput);
    }

    bool hasKnownOutputColor(GrColor* knownOutputColor = nullptr) const {
        if (fProcessorsVisitedWithKnownOutput != fTotalProcessorsVisited) {
            return false;
        }
        if (knownOutputColor) {
            *knownOutputColor = fLastKnownOutputColor.toGrColor();
        }
        return true;
    }

private:
    void internalReset(GrColor color, GrColorComponentFlags colorFlags, bool isLCDCoverage) {
        fTotalProcessorsVisited = 0;
        fIsLCDCoverage = isLCDCoverage;
        fIsOpaque = (kA_GrColorComponentFlag & colorFlags) && GrColorIsOpaque(color);
        fAllProcessorsModulatePremul = true;
        if (kRGBA_GrColorComponentFlags == colorFlags) {
            fProcessorsVisitedWithKnownOutput = 0;
            fLastKnownOutputColor = GrColor4f::FromGrColor(color);
        } else {
            // -1 so that we know that even without adding processors that the color is not known.
            fProcessorsVisitedWithKnownOutput = -1;
        }
    }

    int fTotalProcessorsVisited;
    int fProcessorsVisitedWithKnownOutput;
    bool fIsLCDCoverage;
    bool fIsOpaque;
    bool fAllProcessorsModulatePremul;
    GrColor4f fLastKnownOutputColor;
};

#endif
