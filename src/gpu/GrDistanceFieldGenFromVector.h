/*
 * Copyright 2017 ARM Ltd.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDistanceFieldGenFromVector_DEFINED
#define GrDistanceFieldGenFromVector_DEFINED

#include "include/core/SkPath.h"

class SkMatrix;

/** Given a vector path, generate the associated distance field.

 *  @param distanceField     The distance field to be generated. Should already be allocated
 *                           by the client with the padding defined in "SkDistanceFieldGen.h".
 *  @param path              The path we're using to generate the distance field.
 *  @param matrix            Transformation matrix for path.
 *  @param width             Width of the distance field.
 *  @param height            Height of the distance field.
 *  @param rowBytes          Size of each row in the distance field, in bytes.
 */
bool GrGenerateDistanceFieldFromPath(unsigned char* distanceField,
                                     const SkPath& path, const SkMatrix& viewMatrix,
                                     int width, int height, size_t rowBytes);

inline bool IsDistanceFieldSupportedFillType(SkPathFillType fFillType)
{
    return (SkPathFillType::kEvenOdd == fFillType ||
            SkPathFillType::kInverseEvenOdd == fFillType);
}

#endif
