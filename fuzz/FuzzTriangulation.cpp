/*
 * Copyright 2021 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"
#include "fuzz/FuzzCommon.h"
#include "include/core/SkPath.h"
#include "src/gpu/GrEagerVertexAllocator.h"
#include "src/gpu/geometry/GrPathUtils.h"
#include "src/gpu/geometry/GrTriangulator.h"

DEF_FUZZ(Triangulation, fuzz) {

    SkPath path;
    FuzzEvilPath(fuzz, &path, SkPath::Verb::kDone_Verb);

    SkScalar tol = GrPathUtils::scaleToleranceToSrc(GrPathUtils::kDefaultTolerance,
                                                    SkMatrix::I(), path.getBounds());


    // TODO(robertphillips): messing w/ the clipBounds might be another axis to fuzz.
    // afaict it only affects inverse filled paths.
    SkRect clipBounds = path.getBounds();

    GrCpuVertexAllocator allocator;
    bool isLinear;

    int count = GrTriangulator::PathToTriangles(path, tol, clipBounds, &allocator, &isLinear);
    if (count > 0) {
        allocator.detachVertexData(); // normally handled by the triangulating path renderer.
    }
}
