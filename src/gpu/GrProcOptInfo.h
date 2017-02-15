/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProcOptInfo_DEFINED
#define GrProcOptInfo_DEFINED

#include "GrColor.h"
#include "GrPipelineInput.h"

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
    GrProcOptInfo() = default;

    GrProcOptInfo(const GrPipelineInput& input) : GrProcOptInfo() {
        fIsLCDCoverage = input.isLCDCoverage();
        fIsOpaque = input.isOpaque();
        GrColor color;
        if (input.isConstant(&color)) {
            fLastKnownOutputColor = GrColor4f::FromGrColor(color);
            fProcessorsVisitedWithKnownOutput = 0;
        }
    }

    void resetToLCDCoverage() {
        *this = GrProcOptInfo();
        fIsLCDCoverage = true;
    }

    void reset(const GrPipelineInput& input) { *this = GrProcOptInfo(input); }

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
    bool allProcessorsCompatibleWithCoverageAsAlpha() const {
        return fAllProcessorsCompatibleWithCoverageAsAlpha;
    }
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
    int fTotalProcessorsVisited = 0;
    // negative one means even the color is unknown before adding the first processor.
    int fProcessorsVisitedWithKnownOutput = -1;
    bool fIsLCDCoverage = false;
    bool fIsOpaque = false;
    bool fAllProcessorsCompatibleWithCoverageAsAlpha = true;
    GrColor4f fLastKnownOutputColor;
};

#endif
