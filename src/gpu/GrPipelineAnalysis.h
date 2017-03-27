/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPipelineAnalysis_DEFINED
#define GrPipelineAnalysis_DEFINED

#include "GrColor.h"

class GrDrawOp;
class GrFragmentProcessor;
class GrPrimitiveProcessor;

class GrPipelineAnalysisColor {
public:
    enum class Opaque {
        kNo,
        kYes,
    };

    GrPipelineAnalysisColor(Opaque opaque = Opaque::kNo)
            : fFlags(opaque == Opaque::kYes ? kIsOpaque_Flag : 0) {}

    GrPipelineAnalysisColor(GrColor color) { this->setToConstant(color); }

    void setToConstant(GrColor color) {
        fColor = color;
        if (GrColorIsOpaque(color)) {
            fFlags = kColorIsKnown_Flag | kIsOpaque_Flag;
        } else {
            fFlags = kColorIsKnown_Flag;
        }
    }

    void setToUnknown() { fFlags = 0; }

    void setToUnknownOpaque() { fFlags = kIsOpaque_Flag; }

    bool isOpaque() const { return SkToBool(kIsOpaque_Flag & fFlags); }

    bool isConstant(GrColor* color) const {
        if (kColorIsKnown_Flag & fFlags) {
            *color = fColor;
            return true;
        }
        return false;
    }

private:
    enum Flags {
        kColorIsKnown_Flag = 0x1,
        kIsOpaque_Flag = 0x2,
    };
    uint32_t fFlags;
    GrColor fColor;
};

enum class GrPipelineAnalysisCoverage { kNone, kSingleChannel, kLCD };

/**
 * GrColorFragmentProcessorAnalysis gathers invariant data from a set of color fragment processor.
 * It is used to recognize optimizations that can simplify the generated shader or make blending
 * more effecient.
 */
class GrColorFragmentProcessorAnalysis {
public:
    GrColorFragmentProcessorAnalysis() = default;

    GrColorFragmentProcessorAnalysis(const GrPipelineAnalysisColor& input)
            : GrColorFragmentProcessorAnalysis() {
        fAllProcessorsCompatibleWithCoverageAsAlpha = true;
        fIsOpaque = input.isOpaque();
        GrColor color;
        if (input.isConstant(&color)) {
            fLastKnownOutputColor = GrColor4f::FromGrColor(color);
            fProcessorsVisitedWithKnownOutput = 0;
        }
    }

    void reset(const GrPipelineAnalysisColor& input) {
        *this = GrColorFragmentProcessorAnalysis(input);
    }

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

    GrPipelineAnalysisColor outputColor() const {
        if (fProcessorsVisitedWithKnownOutput != fTotalProcessorsVisited) {
            return GrPipelineAnalysisColor(fIsOpaque ? GrPipelineAnalysisColor::Opaque::kYes
                                                     : GrPipelineAnalysisColor::Opaque::kNo);
        }
        return GrPipelineAnalysisColor(fLastKnownOutputColor.toGrColor());
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
