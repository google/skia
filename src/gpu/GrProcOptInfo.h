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
        fAllProcessorsCompatibleWithCoverageAsAlpha = !input.isLCDCoverage();
        fIsOpaque = input.isOpaque();
        GrColor color;
        if (input.isConstant(&color)) {
            fLastKnownOutputColor = GrColor4f::FromGrColor(color);
            fProcessorsVisitedWithKnownOutput = 0;
        }
    }

    void reset(const GrPipelineInput& input) { *this = GrProcOptInfo(input); }

    /**
     * Runs through a series of processors and updates calculated values. This can be called
     * repeatedly for cases when the sequence of processors is not in a contiguous array.
     */
    void analyzeProcessors(const GrFragmentProcessor* const* processors, int cnt);

    bool isOpaque() const { return fIsOpaque; }

    /**
     * Are all the fragment processors compatible with conflating coverage with color prior to the
     * the first fragment processor. This result does not consider processors that should be
     * eliminated as indicated by initialProcessorsToEliminate().
     */
    bool allProcessorsCompatibleWithCoverageAsAlpha() const {
        return fAllProcessorsCompatibleWithCoverageAsAlpha;
    }

    /**
     * Do any of the fragment processors require local coords. This result does not consider
     * processors that should be eliminated as indicated by initialProcessorsToEliminate().
     */
    bool usesLocalCoords() const { return fUsesLocalCoords; }

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
    bool fIsOpaque = false;
    bool fAllProcessorsCompatibleWithCoverageAsAlpha = true;
    bool fUsesLocalCoords = false;
    GrColor4f fLastKnownOutputColor;
};

#endif
