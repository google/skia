/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOptDrawState_DEFINED
#define GrOptDrawState_DEFINED

#include "GrDrawState.h"
#include "GrRODrawState.h"

/**
 * Subclass of GrRODrawState that holds an optimized version of a GrDrawState. Like it's parent
 * it is meant to be an immutable class, and simply adds a few helpful data members not in the
 * base class.
 */
class GrOptDrawState : public GrRODrawState {
public:
    bool operator== (const GrOptDrawState& that) const;

    bool inputColorIsUsed() const { return fInputColorIsUsed; }
    bool inputCoverageIsUsed() const { return fInputCoverageIsUsed; }

    bool readsDst() const { return fReadsDst; }
    bool readsFragPosition() const { return fReadsFragPosition; }
    bool requiresLocalCoordAttrib() const { return fRequiresLocalCoordAttrib; }

    ///////////////////////////////////////////////////////////////////////////
    /// @name Stage Output Types
    ////

    enum PrimaryOutputType {
        // Modulate color and coverage, write result as the color output.
        kModulate_PrimaryOutputType,
        // Combines the coverage, dst, and color as coverage * color + (1 - coverage) * dst. This
        // can only be set if fDstReadKey is non-zero.
        kCombineWithDst_PrimaryOutputType,

        kPrimaryOutputTypeCnt,
    };

    enum SecondaryOutputType {
        // There is no secondary output
        kNone_SecondaryOutputType,
        // Writes coverage as the secondary output. Only set if dual source blending is supported
        // and primary output is kModulate.
        kCoverage_SecondaryOutputType,
        // Writes coverage * (1 - colorA) as the secondary output. Only set if dual source blending
        // is supported and primary output is kModulate.
        kCoverageISA_SecondaryOutputType,
        // Writes coverage * (1 - colorRGBA) as the secondary output. Only set if dual source
        // blending is supported and primary output is kModulate.
        kCoverageISC_SecondaryOutputType,

        kSecondaryOutputTypeCnt,
    };

    PrimaryOutputType getPrimaryOutputType() const { return fPrimaryOutputType; }
    SecondaryOutputType getSecondaryOutputType() const { return fSecondaryOutputType; }

    /// @}

private:
    /**
     * Constructs and optimized drawState out of a GrRODrawState.
     */
    GrOptDrawState(const GrDrawState& drawState, BlendOptFlags blendOptFlags,
                   GrBlendCoeff optSrcCoeff, GrBlendCoeff optDstCoeff,
                   const GrDrawTargetCaps& caps);

    /**
     * Loops through all the color stage effects to check if the stage will ignore color input or
     * always output a constant color. In the ignore color input case we can ignore all previous
     * stages. In the constant color case, we can ignore all previous stages and
     * the current one and set the state color to the constant color. Once we determine the so
     * called first effective stage, we copy all the effective stages into our optimized
     * state.
     */
    void copyEffectiveColorStages(const GrDrawState& ds);

    /**
     * Loops through all the coverage stage effects to check if the stage will ignore color input.
     * If a coverage stage will ignore input, then we can ignore all coverage stages before it. We
     * loop to determine the first effective coverage stage, and then copy all of our effective
     * coverage stages into our optimized state.
     */
    void copyEffectiveCoverageStages(const GrDrawState& ds);

    /**
     * This function takes in a flag and removes the corresponding fixed function vertex attributes.
     * The flags are in the same order as GrVertexAttribBinding array. If bit i of removeVAFlags is
     * set, then vertex attributes with binding (GrVertexAttribute)i will be removed.
     */
    void removeFixedFunctionVertexAttribs(uint8_t removeVAFlags);

    /**
     * Alter the OptDrawState (adjusting stages, vertex attribs, flags, etc.) based on the
     * BlendOptFlags.
     */
    void adjustFromBlendOpts();

    /**
     * Loop over the effect stages to determine various info like what data they will read and what
     * shaders they require.
     */
    void getStageStats();

    /**
     * Calculates the primary and secondary output types of the shader. For certain output types
     * the function may adjust the blend coefficients. After this function is called the src and dst
     * blend coeffs will represent those used by backend API.
     */
    void setOutputStateInfo(const GrDrawTargetCaps&);

    // These flags are needed to protect the code from creating an unused uniform color/coverage
    // which will cause shader compiler errors.
    bool            fInputColorIsUsed;
    bool            fInputCoverageIsUsed;

    // These flags give aggregated info on the effect stages that are used when building programs.
    bool            fReadsDst;
    bool            fReadsFragPosition;
    bool            fRequiresLocalCoordAttrib;

    SkAutoSTArray<4, GrVertexAttrib> fOptVA;

    BlendOptFlags   fBlendOptFlags;

    // Fragment shader color outputs
    PrimaryOutputType  fPrimaryOutputType : 8;
    SecondaryOutputType  fSecondaryOutputType : 8;

    friend GrOptDrawState* GrDrawState::createOptState(const GrDrawTargetCaps&) const;
    typedef GrRODrawState INHERITED;
};

#endif
