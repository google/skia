/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrQuadUtils_DEFINED
#define GrQuadUtils_DEFINED

enum class GrQuadAAFlags;
enum class GrAAType : unsigned;
class GrQuad;

namespace GrQuadUtils {

    // Resolve disagreements between the overall requested AA type and the per-edge quad AA flags.
    // Both outAAType and outEdgeFlags will be updated.
    void ResolveAAType(GrAAType requestedAAType, GrQuadAAFlags requestedEdgeFlags,
                       const GrQuad& quad, GrAAType* outAAtype, GrQuadAAFlags* outEdgeFlags);

}; // namespace GrQuadUtils

#endif
