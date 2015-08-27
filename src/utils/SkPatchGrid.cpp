/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPatchGrid.h"
#include "SkPatchUtils.h"

SkPatchGrid::SkPatchGrid(int rows, int cols, VertexType flags, SkXfermode* xfer)
    : fRows(0)
    , fCols(0)
    , fModeFlags(kNone_VertexType)
    , fCornerPts(nullptr)
    , fCornerColors(nullptr)
    , fTexCoords(nullptr)
    , fHrzCtrlPts(nullptr)
    , fVrtCtrlPts(nullptr)
    , fXferMode(nullptr) {
        this->reset(rows, cols, flags, xfer);
}

SkPatchGrid::~SkPatchGrid() {
    delete[] fCornerPts;
    delete[] fCornerColors;
    delete[] fTexCoords;
    delete[] fHrzCtrlPts;
    delete[] fVrtCtrlPts;
}

bool SkPatchGrid::setPatch(int x, int y, const SkPoint cubics[12], const SkColor colors[4],
                           const SkPoint texCoords[4]) {
    // Check for the passed paramaters to be within the range of the grid dimensions and a valid
    // pointer for the cubics' control points.
    if (x < 0 || y < 0 || x > fCols - 1 || y > fRows - 1 || nullptr == cubics) {
        return false;
    }
    
    // setup corners and colors
    int cornerPos = y * (fCols + 1) + x;
    fCornerPts[cornerPos] = cubics[SkPatchUtils::kTopP0_CubicCtrlPts];
    fCornerPts[cornerPos + 1] = cubics[SkPatchUtils::kTopP3_CubicCtrlPts];
    fCornerPts[cornerPos + (fCols + 1)] = cubics[SkPatchUtils::kBottomP0_CubicCtrlPts];
    fCornerPts[cornerPos + (fCols + 1) + 1] = cubics[SkPatchUtils::kBottomP3_CubicCtrlPts];
    
    // set horizontal control points
    int hrzPos = y * (fCols * 2) + (x * 2);
    fHrzCtrlPts[hrzPos] = cubics[SkPatchUtils::kTopP1_CubicCtrlPts];
    fHrzCtrlPts[hrzPos + 1] = cubics[SkPatchUtils::kTopP2_CubicCtrlPts];
    fHrzCtrlPts[hrzPos + (fCols * 2)] = cubics[SkPatchUtils::kBottomP1_CubicCtrlPts];
    fHrzCtrlPts[hrzPos + (fCols * 2) + 1] = cubics[SkPatchUtils::kBottomP2_CubicCtrlPts];
    
    // set vertical control points
    int vrtPos = (y*2) * (fCols + 1) + x;
    fVrtCtrlPts[vrtPos] = cubics[SkPatchUtils::kLeftP1_CubicCtrlPts];
    fVrtCtrlPts[vrtPos + 1] = cubics[SkPatchUtils::kRightP1_CubicCtrlPts];
    fVrtCtrlPts[vrtPos + (fCols + 1)] = cubics[SkPatchUtils::kLeftP2_CubicCtrlPts];
    fVrtCtrlPts[vrtPos + (fCols + 1) + 1] = cubics[SkPatchUtils::kRightP2_CubicCtrlPts];
    
    // set optional values (colors and texture coordinates)
    if ((fModeFlags & kColors_VertexType)  && colors) {
        fCornerColors[cornerPos] = colors[0];
        fCornerColors[cornerPos + 1] = colors[1];
        fCornerColors[cornerPos + (fCols + 1)] = colors[3];
        fCornerColors[cornerPos + (fCols + 1) + 1] = colors[2];
    }
    
    if ((fModeFlags & kTexs_VertexType) && texCoords) {
        fTexCoords[cornerPos] = texCoords[0];
        fTexCoords[cornerPos + 1] = texCoords[1];
        fTexCoords[cornerPos + (fCols + 1)] = texCoords[3];
        fTexCoords[cornerPos + (fCols + 1) + 1] = texCoords[2];
    }
    
    return true;
}

bool SkPatchGrid::getPatch(int x, int y, SkPoint cubics[12], SkColor colors[4],
                           SkPoint texCoords[4]) const {
    
    if (x < 0 || y < 0 || x > fCols - 1 || y > fRows - 1 || nullptr == cubics) {
        return false;
    }
    
    // set the patch by building the array of points and colors with the corresponding values.
    int cornerPos = y * (fCols + 1) + x;
    cubics[SkPatchUtils::kTopP0_CubicCtrlPts] = fCornerPts[cornerPos];
    cubics[SkPatchUtils::kTopP3_CubicCtrlPts] = fCornerPts[cornerPos + 1];
    cubics[SkPatchUtils::kBottomP0_CubicCtrlPts] = fCornerPts[cornerPos + (fCols + 1)];
    cubics[SkPatchUtils::kBottomP3_CubicCtrlPts] = fCornerPts[cornerPos + (fCols + 1) + 1];
    
    int hrzPos = y * (fCols * 2) + (x * 2);
    cubics[SkPatchUtils::kTopP1_CubicCtrlPts] = fHrzCtrlPts[hrzPos];
    cubics[SkPatchUtils::kTopP2_CubicCtrlPts] = fHrzCtrlPts[hrzPos + 1];
    cubics[SkPatchUtils::kBottomP1_CubicCtrlPts] = fHrzCtrlPts[hrzPos + (fCols * 2)];
    cubics[SkPatchUtils::kBottomP2_CubicCtrlPts] = fHrzCtrlPts[hrzPos + (fCols * 2) + 1];
    
    int vrtPos = (y*2) * (fCols + 1) + x;
    cubics[SkPatchUtils::kLeftP1_CubicCtrlPts] = fVrtCtrlPts[vrtPos];
    cubics[SkPatchUtils::kRightP1_CubicCtrlPts] = fVrtCtrlPts[vrtPos + 1];
    cubics[SkPatchUtils::kLeftP2_CubicCtrlPts] = fVrtCtrlPts[vrtPos + (fCols + 1)];
    cubics[SkPatchUtils::kRightP2_CubicCtrlPts] = fVrtCtrlPts[vrtPos + (fCols + 1) + 1];
    
    if ((fModeFlags & kColors_VertexType)  && colors) {
        colors[0] = fCornerColors[cornerPos];
        colors[1] = fCornerColors[cornerPos + 1];
        colors[3] = fCornerColors[cornerPos + (fCols + 1)];
        colors[2] = fCornerColors[cornerPos + (fCols + 1) + 1];
    }
    
    if ((fModeFlags & kTexs_VertexType)  && texCoords) {
        texCoords[0] = fTexCoords[cornerPos];
        texCoords[1] = fTexCoords[cornerPos + 1];
        texCoords[3] = fTexCoords[cornerPos + (fCols + 1)];
        texCoords[2] = fTexCoords[cornerPos + (fCols + 1) + 1];
    }
    
    return true;
}

void SkPatchGrid::reset(int rows, int cols, VertexType flags, SkXfermode* xMode) {
    delete[] fCornerPts;
    delete[] fCornerColors;
    delete[] fTexCoords;
    delete[] fHrzCtrlPts;
    delete[] fVrtCtrlPts;

    fCols = cols;
    fRows = rows;
    fModeFlags = flags;
    fXferMode = xMode;

    fCornerPts = new SkPoint[(fRows + 1) * (fCols + 1)];
    fHrzCtrlPts = new SkPoint[(fRows + 1) * fCols * 2];
    fVrtCtrlPts = new SkPoint[fRows * 2 * (fCols + 1)];
    memset(fCornerPts, 0, (fRows + 1) * (fCols + 1) * sizeof(SkPoint));
    memset(fHrzCtrlPts, 0, (fRows + 1) * fCols * 2 * sizeof(SkPoint));
    memset(fVrtCtrlPts, 0, fRows * 2 * (fCols + 1) * sizeof(SkPoint));
    
    if (fModeFlags & kColors_VertexType) {
        fCornerColors = new SkColor[(fRows + 1) * (fCols + 1)];
        memset(fCornerColors, 0, (fRows + 1) * (fCols + 1) * sizeof(SkColor));
    }
    
    if (fModeFlags & kTexs_VertexType) {
        fTexCoords = new SkPoint[(fRows + 1) * (fCols + 1)];
        memset(fTexCoords, 0, (fRows + 1) * (fCols + 1) * sizeof(SkPoint));
    }
}

void SkPatchGrid::draw(SkCanvas* canvas, SkPaint& paint) {
    int* maxCols = new int[fCols];
    int* maxRows = new int[fRows];
    memset(maxCols, 0, fCols * sizeof(int));
    memset(maxRows, 0, fRows * sizeof(int));
    
    // Get the maximum level of detail per axis for each row and column
    for (int y = 0; y < fRows; y++) {
        for (int x = 0; x < fCols; x++) {
            SkPoint cubics[12];
            this->getPatch(x, y, cubics, nullptr, nullptr);
            SkMatrix matrix = canvas->getTotalMatrix();
            SkISize lod = SkPatchUtils::GetLevelOfDetail(cubics, &matrix);
            maxCols[x] = SkMax32(maxCols[x], lod.width());
            maxRows[y] = SkMax32(maxRows[y], lod.height());
        }
    }
    // Draw the patches by generating their geometry with the maximum level of detail per axis.
    for (int x = 0; x < fCols; x++) {
        for (int y = 0; y < fRows; y++) {
            SkPoint cubics[12];
            SkPoint texCoords[4];
            SkColor colors[4];
            this->getPatch(x, y, cubics, colors, texCoords);
            SkPatchUtils::VertexData data;
            if (SkPatchUtils::getVertexData(&data, cubics,
                                            fModeFlags & kColors_VertexType ? colors : nullptr,
                                            fModeFlags & kTexs_VertexType ? texCoords : nullptr,
                                            maxCols[x], maxRows[y])) {
                canvas->drawVertices(SkCanvas::kTriangles_VertexMode, data.fVertexCount,
                                     data.fPoints, data.fTexCoords, data.fColors, fXferMode,
                                     data.fIndices, data.fIndexCount, paint);
            }
        }
    }
    delete[] maxCols;
    delete[] maxRows;
}
