/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "src/gpu/GrQuad.h"

// what scenarios are important to test


// identity, rot90, rot-90, rot180, flipx-1, flipy-1 for the draw quad
// x
// draw inside clip fully (0 edges)
// draw contains clip fully (4 edges)
// draw does not intersect clip at all (0 edges and drop)
// clip 1 edge (clip bigger than draw, intersect one edge)
// clip 3 edges (clip smaller than draw, intersect one edge)
// clip 2 edges (intersects over a corner)
// x
// clip has AA
// clip has no AA
// x
// has local coords

// Test all scenarios of an axis-aligned GrQuad (I, R90, R180, R270, Sx-1, Sy-1)
// against a clip rect, with or without local coordinates, and testing that the
// AA edges are marked appropriately when cropped.
TEST(CropAxisAligned) {
    // Make the base rect contain the origin and unique edge values so that each
    // transform produces a different axis-aligned rectangle.
    SkRect drawRect = SkRect::MakeLTRB(-5.f, -6.f, 10.f, 11.f);
    SkRect localRect = SkRect::MakeWH(1.f, 1.f);

    SkMatrix drawMatrices[6];
    drawMatrices[0].setIdentity();
    drawMatrices[1].setRotate(90.f);
    drawMatrices[2].setRotate(180.f);
    drawMatrices[3].setRotate(270.f);
    drawMatrices[4].setScale(-1.f, 1.f);
    drawMatrices[5].setScale(1.f, -1.f);

    SkRect clipRects[5];
    clipRects[0] = SkRect::MakeLTRB(-4.f, -4.f, 4.f, 4.f); // Always inside draw
    clipRects[1] = SkRect::MakeLTRB(-12.f, -12.f, 12.f, 12.f); // Always outside draw
    clipRects[2] = SkRect::MakeLTRB(-7.f, -7.f, 7.f, 7.f); // Intersects two axes somehow
    clipRects[3] = SkRect::MakeLTRB(-7.f, -12.f, 7.f, 12.f); // Intersects one axis
    clipRects[4] = SkRect::MakeLTRB(-12.f, -7.f, 12.f, 7.f); // Intersects other axis

    for (int i = 0; i < SK_ARRAY_COUNT(drawMatrices); ++i) {
        for (int j = 0; j < SK_ARRAY_COUNT(clipRects); ++j) {
            for (int k = 0; k < 2; ++k) {
                const GrAA clipAA = k == 0 ? GrAA::kYes : GrAA::kNo;
                for (int l = 0; l < 2; ++l) {
                    // Reset everything that can be modified by a crop() call
                    GrQuad drawQuad = GrQuad::MakeFromRect(drawRect, drawMatrices[i]);
                    GrQuad localQuad(localRect);
                    GrQuad* localQuadPtr = l == 0 ? &localQuad : nullptr;
                    GrQuadAAFlags edgeFlags = k == 0 ? GrQuadAAFlags::kNone : GrQuadAAFlags::kAll;

                    drawQuad.crop(clipRects[j], &edgeFlags, localQuadPtr);

                    // And analyze
                    // FIXME we know the bounds of drawQuad should exactly match intersection of
                    // drawRect mapped then intersected with clipRect
                    // And if we apply inverse draw matrix to final draw quad we should get the
                    // local coordinates (right order to directly compare, unsure?)
                    // Then I should check the edges that have changed, but that gets trickier
                    //  -> could map clipRect into local space and then check edges that way, probs the best
                }
            }
        }
        GrQuad drawQuad = GrQuad::MakeFromRect(drawRect, drawMatrices[i]);
        SkASSERT(drawQuad.quadType() == GrQuad::Type::kRect);

        for (int j = 0; j < 2; ++j) {
            // Initial flags are opposite of clipAA so we can see what is turned on/off easily
            GrQuadAAFlags edgeFlags = j == 0 ? GrQuadAAFlags::kNone : GrQuadAAFlags::kAll;

            for (int k = 0; k < 2; ++k) {

                for (int l = 0; l < )

            }
        }
    }
}


//
// then we also have rotated, skewed, and perspective draws
// x
// draw inside clip fully (e.g. 0 edge move)
// draw contains clip fully (e.g. 4 edge move)
// clip intersection, move 1 edge
// clip intersection, move 2 edge
// clip intersection, move 3 edge
// x
// has local coords

// no need to test clip AA since edge flags won't be modified

// FIXME but for now just make sure that clip inside with no local coords does the right thing
