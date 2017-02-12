/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPipelineInput_DEFINED
#define GrPipelineInput_DEFINED

#include "GrColor.h"

/**
 * This describes the color or coverage input that will be seen by the first color or coverage stage
 * of a GrPipeline. This is also the GrPrimitiveProcessor color or coverage *output*.
 */
struct GrPipelineInput {
    GrPipelineInput()
            : fValidFlags(kNone_GrColorComponentFlags), fColor(0), fIsLCDCoverage(false) {}

    void setKnownFourComponents(GrColor color) {
        fColor = color;
        fValidFlags = kRGBA_GrColorComponentFlags;
    }

    void setUnknownFourComponents() { fValidFlags = kNone_GrColorComponentFlags; }

    void setUnknownOpaqueFourComponents() {
        fColor = 0xffU << GrColor_SHIFT_A;
        fValidFlags = kA_GrColorComponentFlag;
    }

    void setKnownSingleComponent(uint8_t alpha) {
        fColor = GrColorPackRGBA(alpha, alpha, alpha, alpha);
        fValidFlags = kRGBA_GrColorComponentFlags;
    }

    void setUnknownSingleComponent() { fValidFlags = kNone_GrColorComponentFlags; }

    void setUsingLCDCoverage() { fIsLCDCoverage = true; }

    GrColorComponentFlags fValidFlags;
    GrColor fColor;
    bool fIsLCDCoverage;
};

#endif
