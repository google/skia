/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPatchGrid_DEFINED
#define SkPatchGrid_DEFINED

#include "SkCanvas.h"
#include "SkPatchUtils.h"
#include "SkXfermode.h"

/**
 * Class that represents a grid of patches. Adjacent patches share their corners and a color is
 * specified at each one of them. The colors are bilinearly interpolated across the patch.
 *
 * This implementation defines a bidimensional array of patches. There are 3 arrays to store the
 * control points of the patches to avoid storing repeated data since there are several points
 * shared between adjacent patches.
 *
 * The array fCornerPts stores the corner control points of the patches.
 * The array fHrzPts holds the intermidiate control points of the top and bottom curves of a patch.
 * The array fVrtPts holds the intermidiate control points of the left and right curves of a patch.
 * The array fCornerColors holds the corner colors in the same format as fCornerPts.
 * The array fTexCoords holds the texture coordinates in the same format as fCornerpts.
 *
 *               fCornerPts               fHrzPts                  fVrtPts
 *             --------------       -------------------         --------------
 *            | C0 | C1 | C2 |     | H0 | H1 | H2 | H3 |       | V0 | V1 | V2 |
 *             --------------       ------------------         ---------------
 *            | C3 | C4 | C5 |     | H4 | H5 | H6 | H7 |       | V4 | V5 | V6 |
 *             --------------       -------------------         --------------
 *            | C6 | C7 | C8 |     | H8 | H9 | H10| H11|       | V6 | V7 | V8 |
 *             --------------       -------------------         --------------
 *                                                             | V9 | V10| V11|
 *                                                              --------------
 *
 * With the above configuration we would have a 2x2 grid of patches:
 *               H0     H1 H2   H3
 *              /        \/      \
 *              C0-------C1-------C2
 *             /|        |        |\
 *           v0 |        v1       | v2
 *           v3 |        V4       | v5
 *             \|        |        |/
 *              C3-H4-H5-C4-H6-H7-C5
 *             /|        |        |\
 *           v6 |        v7       | v8
 *           v9 |        v10      | v11
 *             \|        |        |/
 *              C6-------C7-------C8
 *               \      / \      /
 *                H8   H9  H10  H11
 *
 * When trying to get a patch at a certain position it justs builds it with the corresponding
 * points.
 * When adding a patch it tries to add the points at their corresponding position trying to comply
 * with the adjacent points or overwriting them.
 *
 * Based the idea on the SVG2 spec for mesh gradients in which a grid of patches is build as in the
 * the following example:
 * <meshGradient x="100" y="100">
 *      <meshRow>
 *          <meshPatch>
 *              <stop .../>
 *              Up to four stops in first patch. See details below.
 *          </meshPatch>
 *          <meshPatch>
 *              Any number of meshPatches in row.
 *          </meshPatch>
 *      </meshRow>
 *      <meshRow>
 *          Any number of meshRows, each with the same number of meshPatches as in the first row.
 *      </meshRow>
 * </meshGradient>
 */
class SkPatchGrid {

public:

    enum VertexType {
        kNone_VertexType = 0X00,
        kColors_VertexType = 0x01,
        kTexs_VertexType = 0x02,
        kColorsAndTexs_VertexType = 0x03
    };

    SkPatchGrid(int rows = 0, int cols = 0, VertexType flags = kNone_VertexType,
                SkXfermode* xfer = nullptr);

    ~SkPatchGrid();

    /**
     * Add a patch at location (x,y) overwriting the previous patch and shared points so they
     * mantain C0 connectivity.
     * The control points must be passed in a clockwise order starting at the top left corner.
     * The colors and texCoords are the values at the corners of the patch which will be bilerp
     * across it, they must also be in counterclockwise order starting at the top left corner.
     */
    bool setPatch(int x, int y, const SkPoint cubics[12], const SkColor colors[4],
                  const SkPoint texCoords[4]);

    /**
     * Get patch at location (x,y). If cubics, colors or texCoords is not nullptr it sets patch's
     * array with its corresponding values.
     * The function returns false if the cubics parameter is nullptr or if the (x,y) coordinates are
     * not within the range of the grid.
     */
    bool getPatch(int x, int y, SkPoint cubics[12], SkColor colors[4], SkPoint texCoords[4]) const;

    /**
     * Resets the grid of patches to contain rows and cols of patches.
     */
    void reset(int rows, int cols, VertexType flags, SkXfermode* xMode);

    /**
     * Draws the grid of patches. The patches are drawn starting at patch (0,0) drawing columns, so
     * for a 2x2 grid the order would be (0,0)->(0,1)->(1,0)->(1,1). The order follows the order
     * of the parametric coordinates of the coons patch.
     */
    void draw(SkCanvas* canvas, SkPaint& paint);

    /**
     * Get the dimensions of the grid of patches.
     */
    SkISize getDimensions() const {
        return SkISize::Make(fCols, fRows);
    }

private:
    int fRows, fCols;
    VertexType fModeFlags;
    SkPoint* fCornerPts;
    SkColor* fCornerColors;
    SkPoint* fTexCoords;
    SkPoint* fHrzCtrlPts;
    SkPoint* fVrtCtrlPts;
    SkXfermode* fXferMode;
};


#endif
