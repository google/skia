/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkCamera.h"

static SkScalar SkScalarDotDiv(int count, const SkScalar a[], int step_a,
                               const SkScalar b[], int step_b,
                               SkScalar denom) {
    SkScalar prod = 0;
    for (int i = 0; i < count; i++) {
        prod += a[0] * b[0];
        a += step_a;
        b += step_b;
    }
    return prod / denom;
}

///////////////////////////////////////////////////////////////////////////////

SkPatch3D::SkPatch3D() {
    this->reset();
}

void SkPatch3D::reset() {
    fOrigin = {0, 0, 0};
    fU = {SK_Scalar1, 0, 0};
    fV = {0, -SK_Scalar1, 0};
}

void SkPatch3D::transform(const SkM44& m, SkPatch3D* dst) const {
    if (dst == nullptr) {
        dst = const_cast<SkPatch3D*>(this);
    }
    dst->fU = m * fU;
    dst->fV = m * fV;
    auto [x,y,z,_] = m.map(fOrigin.x, fOrigin.y, fOrigin.z, 1);
    dst->fOrigin = {x, y, z};
}

SkScalar SkPatch3D::dotWith(SkScalar dx, SkScalar dy, SkScalar dz) const {
    SkScalar cx = fU.y * fV.z - fU.z * fV.y;
    SkScalar cy = fU.z * fV.x - fU.x * fV.y;
    SkScalar cz = fU.x * fV.y - fU.y * fV.x;

    return cx * dx + cy * dy + cz * dz;
}

///////////////////////////////////////////////////////////////////////////////

SkCamera3D::SkCamera3D() {
    this->reset();
}

void SkCamera3D::reset() {
    fLocation = {0, 0, -SkIntToScalar(576)};   // 8 inches backward
    fAxis = {0, 0, SK_Scalar1};                // forward
    fZenith = {0, -SK_Scalar1, 0};             // up

    fObserver = {0, 0, fLocation.z};

    fNeedToUpdate = true;
}

void SkCamera3D::update() {
    fNeedToUpdate = true;
}

void SkCamera3D::doUpdate() const {
    SkV3    axis, zenith, cross;

    // construct a orthonormal basis of cross (x), zenith (y), and axis (z)
    axis = fAxis.normalize();

    zenith = fZenith - (axis * fZenith) * axis;
    zenith = zenith.normalize();

    cross = axis.cross(zenith);

    {
        SkMatrix* orien = &fOrientation;
        auto [x, y, z] = fObserver;

        // Looking along the view axis we have:
        //
        //   /|\ zenith
        //    |
        //    |
        //    |  * observer (projected on XY plane)
        //    |
        //    |____________\ cross
        //                 /
        //
        // So this does a z-shear along the view axis based on the observer's x and y values,
        // and scales in x and y relative to the negative of the observer's z value
        // (the observer is in the negative z direction).

        orien->set(SkMatrix::kMScaleX, x * axis.x - z * cross.x);
        orien->set(SkMatrix::kMSkewX,  x * axis.y - z * cross.y);
        orien->set(SkMatrix::kMTransX, x * axis.z - z * cross.z);
        orien->set(SkMatrix::kMSkewY,  y * axis.x - z * zenith.x);
        orien->set(SkMatrix::kMScaleY, y * axis.y - z * zenith.y);
        orien->set(SkMatrix::kMTransY, y * axis.z - z * zenith.z);
        orien->set(SkMatrix::kMPersp0, axis.x);
        orien->set(SkMatrix::kMPersp1, axis.y);
        orien->set(SkMatrix::kMPersp2, axis.z);
    }
}

void SkCamera3D::patchToMatrix(const SkPatch3D& quilt, SkMatrix* matrix) const {
    if (fNeedToUpdate) {
        this->doUpdate();
        fNeedToUpdate = false;
    }

    const SkScalar* mapPtr = (const SkScalar*)(const void*)&fOrientation;
    const SkScalar* patchPtr;

    SkV3 diff = quilt.fOrigin - fLocation;
    SkScalar dot = diff.dot({mapPtr[6], mapPtr[7], mapPtr[8]});

    // This multiplies fOrientation by the matrix [quilt.fU quilt.fV diff] -- U, V, and diff are
    // column vectors in the matrix -- then divides by the length of the projection of diff onto
    // the view axis (which is 'dot'). This transforms the patch (which transforms from local path
    // space to world space) into view space (since fOrientation transforms from world space to
    // view space).
    //
    // The divide by 'dot' isn't strictly necessary as the homogeneous divide would do much the
    // same thing (it's just scaling the entire matrix by 1/dot). It looks like it's normalizing
    // the matrix into some canonical space.
    patchPtr = (const SkScalar*)&quilt;
    matrix->set(SkMatrix::kMScaleX, SkScalarDotDiv(3, patchPtr, 1, mapPtr, 1, dot));
    matrix->set(SkMatrix::kMSkewY,  SkScalarDotDiv(3, patchPtr, 1, mapPtr+3, 1, dot));
    matrix->set(SkMatrix::kMPersp0, SkScalarDotDiv(3, patchPtr, 1, mapPtr+6, 1, dot));

    patchPtr += 3;
    matrix->set(SkMatrix::kMSkewX,  SkScalarDotDiv(3, patchPtr, 1, mapPtr, 1, dot));
    matrix->set(SkMatrix::kMScaleY, SkScalarDotDiv(3, patchPtr, 1, mapPtr+3, 1, dot));
    matrix->set(SkMatrix::kMPersp1, SkScalarDotDiv(3, patchPtr, 1, mapPtr+6, 1, dot));

    patchPtr = (const SkScalar*)(const void*)&diff;
    matrix->set(SkMatrix::kMTransX, SkScalarDotDiv(3, patchPtr, 1, mapPtr, 1, dot));
    matrix->set(SkMatrix::kMTransY, SkScalarDotDiv(3, patchPtr, 1, mapPtr+3, 1, dot));
    matrix->set(SkMatrix::kMPersp2, SK_Scalar1);
}

///////////////////////////////////////////////////////////////////////////////

Sk3DView::Sk3DView() {
    fRec = &fInitialRec;
}

Sk3DView::~Sk3DView() {
    Rec* rec = fRec;
    while (rec != &fInitialRec) {
        Rec* next = rec->fNext;
        delete rec;
        rec = next;
    }
}

void Sk3DView::save() {
    Rec* rec = new Rec;
    rec->fNext = fRec;
    rec->fMatrix = fRec->fMatrix;
    fRec = rec;
}

void Sk3DView::restore() {
    SkASSERT(fRec != &fInitialRec);
    Rec* next = fRec->fNext;
    delete fRec;
    fRec = next;
}

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
void Sk3DView::setCameraLocation(SkScalar x, SkScalar y, SkScalar z) {
    // the camera location is passed in inches, set in pt
    SkScalar lz = z * 72.0f;
    fCamera.fLocation = {x * 72.0f, y * 72.0f, lz};
    fCamera.fObserver = {0, 0, lz};
    fCamera.update();

}

SkScalar Sk3DView::getCameraLocationX() const {
    return fCamera.fLocation.x / 72.0f;
}

SkScalar Sk3DView::getCameraLocationY() const {
    return fCamera.fLocation.y / 72.0f;
}

SkScalar Sk3DView::getCameraLocationZ() const {
    return fCamera.fLocation.z / 72.0f;
}
#endif

void Sk3DView::translate(SkScalar x, SkScalar y, SkScalar z) {
    fRec->fMatrix.preTranslate(x, y, z);
}

void Sk3DView::rotateX(SkScalar deg) {
    fRec->fMatrix.preConcat(SkM44::Rotate({1, 0, 0}, deg * SK_ScalarPI / 180));
}

void Sk3DView::rotateY(SkScalar deg) {
    fRec->fMatrix.preConcat(SkM44::Rotate({0,-1, 0}, deg * SK_ScalarPI / 180));
}

void Sk3DView::rotateZ(SkScalar deg) {
    fRec->fMatrix.preConcat(SkM44::Rotate({0, 0, 1}, deg * SK_ScalarPI / 180));
}

SkScalar Sk3DView::dotWithNormal(SkScalar x, SkScalar y, SkScalar z) const {
    SkPatch3D   patch;
    patch.transform(fRec->fMatrix);
    return patch.dotWith(x, y, z);
}

void Sk3DView::getMatrix(SkMatrix* matrix) const {
    if (matrix != nullptr) {
        SkPatch3D   patch;
        patch.transform(fRec->fMatrix);
        fCamera.patchToMatrix(patch, matrix);
    }
}

#include "include/core/SkCanvas.h"

void Sk3DView::applyToCanvas(SkCanvas* canvas) const {
    SkMatrix    matrix;

    this->getMatrix(&matrix);
    canvas->concat(matrix);
}
