/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOptDrawState_DEFINED
#define GrOptDrawState_DEFINED

#include "GrRODrawState.h"

class GrDrawState;

/**
 * Subclass of GrRODrawState that holds an optimized version of a GrDrawState. Like it's parent
 * it is meant to be an immutable class, and simply adds a few helpful data members not in the
 * base class.
 */
class GrOptDrawState : public GrRODrawState {
public:
    /**
     * Constructs and optimized drawState out of a GrRODrawState.
     */
    explicit GrOptDrawState(const GrDrawState& drawState);

    bool operator== (const GrOptDrawState& that) const;

private:
    /*
     * Loops through all the color stage effects to check if the stage will ignore color input or
     * always output a constant color. In the ignore color input case we can ignore all previous
     * stages. In the constant color case, we can ignore all previous stages and
     * the current one and set the state color to the constant color. Once we determine the so
     * called first effective stage, we copy all the effective stages into our optimized
     * state.
     */
    void copyEffectiveColorStages(const GrDrawState& ds);

    /*
     * Loops through all the coverage stage effects to check if the stage will ignore color input.
     * If a coverage stage will ignore input, then we can ignore all coverage stages before it. We
     * loop to determine the first effective coverage stage, and then copy all of our effective
     * coverage stages into our optimized state.
     */
    void copyEffectiveCoverageStages(const GrDrawState& ds);

    /*
     * This function takes in a flag and removes the corresponding fixed function vertex attributes.
     * The flags are in the same order as GrVertexAttribBinding array. If bit i of removeVAFlags is
     * set, then vertex attributes with binding (GrVertexAttribute)i will be removed.
     */
    void removeFixedFunctionVertexAttribs(uint8_t removeVAFlags);

    void removeColorVertexAttrib();

    // These flags are needed to protect the code from creating an unused uniform color/coverage
    // which will cause shader compiler errors.
    bool            fInputColorIsUsed;
    bool            fInputCoverageIsUsed;

    SkAutoSTArray<4, GrVertexAttrib> fOptVA;

    typedef GrRODrawState INHERITED;
};

#endif
