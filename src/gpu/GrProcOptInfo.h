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
    GrProcOptInfo() : fInOut(0, static_cast<GrColorComponentFlags>(0)) {}

    GrProcOptInfo(GrColor color, GrColorComponentFlags colorFlags)
            : fInOut(color, colorFlags), fInputColor(color) {}

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
    void addProcessors(const GrFragmentProcessor* const* processors, int cnt);

    bool isSolidWhite() const { return fInOut.isSolidWhite(); }
    bool isOpaque() const { return fInOut.isOpaque(); }
    bool allStagesMultiplyInput() const { return fInOut.allStagesMulInput(); }
    bool isLCDCoverage() const { return fIsLCDCoverage; }
    GrColor color() const { return fInOut.color(); }
    GrColorComponentFlags validFlags() const { return fInOut.validFlags(); }

    /**
     * Returns the index of the first effective color processor. If an intermediate processor
     * doesn't read its input or has a known output, then we can ignore all earlier processors
     * since they will not affect the final output. Thus the first effective processors index is
     * the index to the first processor that will have an effect on the final output.
     *
     * If processors before the firstEffectiveProcessorIndex() are removed, corresponding values
     * from inputColorIsUsed(), inputColorToEffectiveProcessor(), removeVertexAttribs(), and
     * readsDst() must be used when setting up the draw to ensure correct drawing.
     */
    int firstEffectiveProcessorIndex() const { return fFirstEffectiveProcessorIndex; }

    /**
     * True if the first effective processor reads its input, false otherwise.
     */
    bool inputColorIsUsed() const { return fInputColorIsUsed; }

    /**
     * If input color is used and per-vertex colors are not used, this is the input color to the
     * first effective processor.
     */
    GrColor inputColorToFirstEffectiveProccesor() const { return fInputColor; }

private:
    void internalReset(GrColor color, GrColorComponentFlags colorFlags, bool isLCDCoverage) {
        fInOut.reset(color, colorFlags);
        fFirstEffectiveProcessorIndex = 0;
        fInputColorIsUsed = true;
        fInputColor = color;
        fIsLCDCoverage = isLCDCoverage;
    }

    void internalCalc(const GrFragmentProcessor* const[], int cnt);

    GrInvariantOutput fInOut;
    int fFirstEffectiveProcessorIndex = 0;
    bool fInputColorIsUsed = true;
    bool fIsLCDCoverage = false;
    GrColor fInputColor = 0;
};

#endif
