/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRRectEffect_DEFINED
#define GrRRectEffect_DEFINED

#include "GrTypes.h"

class GrEffectRef;
class SkRRect;

namespace GrRRectEffect {
    enum EdgeType {
        kFillAA_EdgeType,
        kInverseFillAA_EdgeType,
        
        kLastEdgeType = kInverseFillAA_EdgeType,
    };
    
    static const int kEdgeTypeCnt = kLastEdgeType + 1;

    /**
     * Creates an effect that performs anti-aliased clipping against a SkRRect. It doesn't support
     * all varieties of SkRRect so the caller must check for a NULL return.
     */
    GrEffectRef* Create(EdgeType, const SkRRect&);
};

#endif
