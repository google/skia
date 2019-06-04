/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrQuadUtils_DEFINED
#define GrQuadUtils_DEFINED

enum class GrQuadAAFlags;
enum class GrAA : bool;
enum class GrAAType : unsigned;
class GrQuad;
struct SkRect;

namespace GrQuadUtils {

    // Resolve disagreements between the overall requested AA type and the per-edge quad AA flags.
    // Both outAAType and outEdgeFlags will be updated.
    void ResolveAAType(GrAAType requestedAAType, GrQuadAAFlags requestedEdgeFlags,
                       const GrQuad& quad, GrAAType* outAAtype, GrQuadAAFlags* outEdgeFlags);

    /**
     * Crops this quad to the provided device-space axis-aligned rectangle. If the intersection of
     * this quad (projected) and clipDevRect results in a quadrilateral, this returns true. If not,
     * this quad will be updated to be the smallest quad of the same type such that its intersection
     * with clipDevRect is visually the same.
     *
     * The provided edge flags are updated to reflect edges clipped by clipDevRect (toggling on or
     * or off based on clipAA policy). The provided local coordinates will be updated to reflect
     * the updated device coordinates of this quad.
     *
     * 'local' may be null, in which case the new local coordinates will not be calculated. This is
     * useful when it's known a paint does not require local coordinates. However, 'edgeFlags'
     * cannot be null.
     */
    bool CropToRect(const SkRect& cropRect, GrAA cropAA, GrQuadAAFlags* edgeAA, GrQuad* quad,
                    GrQuad* local=nullptr);

}; // namespace GrQuadUtils

#endif
