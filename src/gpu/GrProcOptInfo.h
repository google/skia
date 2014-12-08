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

class GrFragmentStage;
class GrFragmentProcessor;
class GrGeometryProcessor;
class GrProcessor;

/**
 * GrProcOptInfo gathers invariant data from a set of processor stages.It is used to recognize
 * optimizations related to eliminating stages and vertex attributes that aren't necessary for a
 * draw.
 */
class GrProcOptInfo {
public:
    GrProcOptInfo()
        : fInOut(0, static_cast<GrColorComponentFlags>(0), false)
        , fFirstEffectStageIndex(0)
        , fInputColorIsUsed(true)
        , fInputColor(0)
        , fRemoveVertexAttrib(false)
        , fReadsDst(false)
        , fReadsFragPosition(false) {}

    void calcWithInitialValues(const GrFragmentStage*, int stageCount, GrColor startColor,
                               GrColorComponentFlags flags, bool areCoverageStages,
                               const GrGeometryProcessor* gp = NULL);

    bool isSolidWhite() const { return fInOut.isSolidWhite(); }
    bool isOpaque() const { return fInOut.isOpaque(); }
    bool isSingleComponent() const { return fInOut.isSingleComponent(); }

    GrColor color() const { return fInOut.color(); }
    uint8_t validFlags() const { return fInOut.validFlags(); }

    /**
     * Returns the index of the first effective color stage. If an intermediate stage doesn't read
     * its input or has a known output, then we can ignore all earlier stages since they will not
     * affect the final output. Thus the first effective stage index is the index to the first stage
     * that will have an effect on the final output.
     *
     * If stages before the firstEffectiveStageIndex are removed, corresponding values from
     * inputColorIsUsed(), inputColorToEffectiveStage(), removeVertexAttribs(), and readsDst() must
     * be used when setting up the draw to ensure correct drawing.
     */
    int firstEffectiveStageIndex() const { return fFirstEffectStageIndex; }

    /**
     * True if the first effective stage reads its input, false otherwise.
     */
    bool inputColorIsUsed() const { return fInputColorIsUsed; }

    /**
     * If input color is used and per-vertex colors are not used, this is the input color to the
     * first effective stage.
     */
    GrColor inputColorToEffectiveStage() const { return fInputColor; }

    /**
     * Given the set of optimizations determined by GrProcOptInfo, should the caller remove the
     * color/coverage vertex attribute that was input to the first stage.
     */
    bool removeVertexAttrib() const { return fRemoveVertexAttrib; }

    /**
     * Returns true if any of the stages preserved by GrProcOptInfo read the dst color.
     */
    bool readsDst() const { return fReadsDst; }

    /**
     * Returns true if any of the stages preserved by GrProcOptInfo read the frag position.
     */
    bool readsFragPosition() const { return fReadsFragPosition; }

private:
    GrInvariantOutput fInOut;
    int fFirstEffectStageIndex;
    bool fInputColorIsUsed;
    GrColor fInputColor;
    bool fRemoveVertexAttrib;
    bool fReadsDst;
    bool fReadsFragPosition;
};

#endif
