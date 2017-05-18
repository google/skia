/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPrimitiveProcessor.h"

#include "GrCoordTransform.h"

/**
 * The key for an individual coord transform is made up of a matrix type, and a bit that indicates
 * the source of the input coords.
 */
enum {
    kMatrixTypeKeyBits   = 1,
    kPositionCoords_Flag = 1 << kMatrixTypeKeyBits,
    kTransformKeyBits    = kMatrixTypeKeyBits + 1,
};

/**
 * We specialize the vertex code for each of these matrix types.
 */
enum MatrixType {
    kNoPersp_MatrixType  = 0,
    kGeneral_MatrixType  = 1,
};

uint32_t
GrPrimitiveProcessor::getTransformKey(const SkTArray<const GrCoordTransform*, true>& coords,
                                      int numCoords) const {
    uint32_t totalKey = 0;
    for (int t = 0; t < numCoords; ++t) {
        uint32_t key = 0;
        const GrCoordTransform* coordTransform = coords[t];
        if (coordTransform->getMatrix().hasPerspective()) {
            key |= kGeneral_MatrixType;
        } else {
            key |= kNoPersp_MatrixType;
        }

        if (!this->hasExplicitLocalCoords()) {
            key |= kPositionCoords_Flag;
        }

        key <<= kTransformKeyBits * t;

        SkASSERT(0 == (totalKey & key)); // keys for each transform ought not to overlap
        totalKey |= key;
    }
    return totalKey;
}
