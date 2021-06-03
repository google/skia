/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProcessorAnalysis_DEFINED
#define GrProcessorAnalysis_DEFINED

#include "include/private/SkColorData.h"

class GrDrawOp;
class GrFragmentProcessor;

class GrProcessorAnalysisColor {
public:
    enum class Opaque {
        kNo,
        kYes,
    };

    constexpr GrProcessorAnalysisColor(Opaque opaque = Opaque::kNo)
            : fFlags(opaque == Opaque::kYes ? kIsOpaque_Flag : 0)
            , fColor(SK_PMColor4fTRANSPARENT) {}

    GrProcessorAnalysisColor(const SkPMColor4f& color) { this->setToConstant(color); }

    void setToConstant(const SkPMColor4f& color) {
        fColor = color;
        if (color.isOpaque()) {
            fFlags = kColorIsKnown_Flag | kIsOpaque_Flag;
        } else {
            fFlags = kColorIsKnown_Flag;
        }
    }

    void setToUnknown() { fFlags = 0; }

    void setToUnknownOpaque() { fFlags = kIsOpaque_Flag; }

    bool isUnknown() const { return SkToBool(fFlags == 0); }

    bool isOpaque() const { return SkToBool(kIsOpaque_Flag & fFlags); }

    bool isConstant(SkPMColor4f* color = nullptr) const {
        if (kColorIsKnown_Flag & fFlags) {
            if (color) {
                *color = fColor;
            }
            return true;
        }
        return false;
    }

    bool operator==(const GrProcessorAnalysisColor& that) const {
        if (fFlags != that.fFlags) {
            return false;
        }
        return (kColorIsKnown_Flag & fFlags) ? fColor == that.fColor : true;
    }

    /** The returned value reflects the common properties of the two inputs. */
    static GrProcessorAnalysisColor Combine(const GrProcessorAnalysisColor& a,
                                            const GrProcessorAnalysisColor& b) {
        GrProcessorAnalysisColor result;
        uint32_t commonFlags = a.fFlags & b.fFlags;
        if ((kColorIsKnown_Flag & commonFlags) && a.fColor == b.fColor) {
            result.fColor = a.fColor;
            result.fFlags = a.fFlags;
        } else if (kIsOpaque_Flag & commonFlags) {
            result.fFlags = kIsOpaque_Flag;
        }
        return result;
    }

private:
    enum Flags {
        kColorIsKnown_Flag = 0x1,
        kIsOpaque_Flag = 0x2,
    };
    uint32_t fFlags;
    SkPMColor4f fColor;
};

enum class GrProcessorAnalysisCoverage { kNone, kSingleChannel, kLCD };

/**
 * GrColorFragmentProcessorAnalysis gathers invariant data from a set of color fragment processors.
 * It is used to recognize optimizations that can simplify the generated shader or make blending
 * more effecient.
 */
class GrColorFragmentProcessorAnalysis {
public:
    GrColorFragmentProcessorAnalysis() = delete;

    GrColorFragmentProcessorAnalysis(const GrProcessorAnalysisColor& input,
                                     std::unique_ptr<GrFragmentProcessor> const fps[],
                                     int count);

    bool isOpaque() const { return fIsOpaque; }

    /**
     * Are all the fragment processors compatible with conflating coverage with color prior to the
     * the first fragment processor. This result assumes that processors that should be eliminated
     * as indicated by initialProcessorsToEliminate() are in fact eliminated.
     */
    bool allProcessorsCompatibleWithCoverageAsAlpha() const {
        return fCompatibleWithCoverageAsAlpha;
    }

    /**
     * Do any of the fragment processors require local coords. This result assumes that
     * processors that should be eliminated as indicated by initialProcessorsToEliminate() are in
     * fact eliminated.
     */
    bool usesLocalCoords() const { return fUsesLocalCoords; }

    /**
     * Do any of the fragment processors read back the destination color?
     */
    bool willReadDstColor() const { return fWillReadDstColor; }

    /**
     * Will we require a destination-surface texture?
     */
    bool requiresDstTexture(const GrCaps& caps) const;

    /**
     * If we detected that the result after the first N processors is a known color then we
     * eliminate those N processors and replace the GrDrawOp's color input to the GrPipeline with
     * the known output of the Nth processor, so that the Nth+1 fragment processor (or the XP if
     * there are only N processors) sees its expected input. If this returns 0 then there are no
     * processors to eliminate.
     */
    int initialProcessorsToEliminate(SkPMColor4f* newPipelineInputColor) const {
        if (fProcessorsToEliminate > 0) {
            *newPipelineInputColor = fLastKnownOutputColor;
        }
        return fProcessorsToEliminate;
    }

    /**
     * Provides known information about the last processor's output color.
     */
    GrProcessorAnalysisColor outputColor() const {
        if (fOutputColorKnown) {
            return fLastKnownOutputColor;
        }
        return fIsOpaque ? GrProcessorAnalysisColor::Opaque::kYes
                         : GrProcessorAnalysisColor::Opaque::kNo;
    }

private:
    bool fIsOpaque;
    bool fCompatibleWithCoverageAsAlpha;
    bool fUsesLocalCoords;
    bool fWillReadDstColor;
    bool fOutputColorKnown;
    int fProcessorsToEliminate;
    SkPMColor4f fLastKnownOutputColor;
};

#endif
