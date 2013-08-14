
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTypes.h"
#include "GrColor.h"

#ifndef GrBlend_DEFINED
#define GrBlend_DEFINED

static inline bool GrBlendCoeffRefsSrc(GrBlendCoeff coeff) {
    switch (coeff) {
        case kSC_GrBlendCoeff:
        case kISC_GrBlendCoeff:
        case kSA_GrBlendCoeff:
        case kISA_GrBlendCoeff:
            return true;
        default:
            return false;
    }
}

static inline bool GrBlendCoeffRefsDst(GrBlendCoeff coeff) {
    switch (coeff) {
        case kDC_GrBlendCoeff:
        case kIDC_GrBlendCoeff:
        case kDA_GrBlendCoeff:
        case kIDA_GrBlendCoeff:
            return true;
        default:
            return false;
    }
}

GrColor GrSimplifyBlend(GrBlendCoeff* srcCoeff,
                        GrBlendCoeff* dstCoeff,
                        GrColor srcColor, uint32_t srcCompFlags,
                        GrColor dstColor, uint32_t dstCompFlags,
                        GrColor constantColor);

#endif
