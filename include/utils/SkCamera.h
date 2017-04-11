/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

//  Inspired by Rob Johnson's most excellent QuickDraw GX sample code

#ifndef SkCamera_DEFINED
#define SkCamera_DEFINED

#include "SkMatrix.h"

class SkCanvas;

struct SkUnit3D {
    SkScalar fX, fY, fZ;

    void set(SkScalar x, SkScalar y, SkScalar z) {
        fX = x; fY = y; fZ = z;
    }
    static SkScalar Dot(const SkUnit3D&, const SkUnit3D&);
    static void Cross(const SkUnit3D&, const SkUnit3D&, SkUnit3D* cross);
};

struct SkPoint3D {
    SkScalar    fX, fY, fZ;

    void set(SkScalar x, SkScalar y, SkScalar z) {
        fX = x; fY = y; fZ = z;
    }
    SkScalar    normalize(SkUnit3D*) const;
};
typedef SkPoint3D SkVector3D;

struct SkMatrix3D {
    SkScalar    fMat[3][4];

    void reset();

    void setRow(int row, SkScalar a, SkScalar b, SkScalar c, SkScalar d = 0) {
        SkASSERT((unsigned)row < 3);
        fMat[row][0] = a;
        fMat[row][1] = b;
        fMat[row][2] = c;
        fMat[row][3] = d;
    }

    void setRotateX(SkScalar deg);
    void setRotateY(SkScalar deg);
    void setRotateZ(SkScalar deg);
    void setTranslate(SkScalar x, SkScalar y, SkScalar z);

    void preRotateX(SkScalar deg);
    void preRotateY(SkScalar deg);
    void preRotateZ(SkScalar deg);
    void preTranslate(SkScalar x, SkScalar y, SkScalar z);

    void setConcat(const SkMatrix3D& a, const SkMatrix3D& b);
    void mapPoint(const SkPoint3D& src, SkPoint3D* dst) const;
    void mapVector(const SkVector3D& src, SkVector3D* dst) const;

    void mapPoint(SkPoint3D* v) const {
        this->mapPoint(*v, v);
    }

    void mapVector(SkVector3D* v) const {
        this->mapVector(*v, v);
    }
};

class SkPatch3D {
public:
    SkPatch3D();

    void    reset();
    void    transform(const SkMatrix3D&, SkPatch3D* dst = NULL) const;

    // dot a unit vector with the patch's normal
    SkScalar dotWith(SkScalar dx, SkScalar dy, SkScalar dz) const;
    SkScalar dotWith(const SkVector3D& v) const {
        return this->dotWith(v.fX, v.fY, v.fZ);
    }

    // deprecated, but still here for animator (for now)
    void rotate(SkScalar /*x*/, SkScalar /*y*/, SkScalar /*z*/) {}
    void rotateDegrees(SkScalar /*x*/, SkScalar /*y*/, SkScalar /*z*/) {}

private:
public: // make public for SkDraw3D for now
    SkVector3D  fU, fV;
    SkPoint3D   fOrigin;

    friend class SkCamera3D;
};

class SkCamera3D {
public:
    SkCamera3D();

    void reset();
    void update();
    void patchToMatrix(const SkPatch3D&, SkMatrix* matrix) const;

    SkPoint3D   fLocation;   // origin of the camera's space
    SkPoint3D   fAxis;       // view direction
    SkPoint3D   fZenith;     // up direction
    SkPoint3D   fObserver;   // eye position (may not be the same as the origin)

private:
    mutable SkMatrix    fOrientation;
    mutable bool        fNeedToUpdate;

    void doUpdate() const;
};

class Sk3DView : SkNoncopyable {
public:
    Sk3DView();
    ~Sk3DView();

    void save();
    void restore();

    void translate(SkScalar x, SkScalar y, SkScalar z);
    void rotateX(SkScalar deg);
    void rotateY(SkScalar deg);
    void rotateZ(SkScalar deg);

#ifdef SK_BUILD_FOR_ANDROID
    void setCameraLocation(SkScalar x, SkScalar y, SkScalar z);
    SkScalar getCameraLocationX();
    SkScalar getCameraLocationY();
    SkScalar getCameraLocationZ();
#endif

    void getMatrix(SkMatrix*) const;
    void applyToCanvas(SkCanvas*) const;

    SkScalar dotWithNormal(SkScalar dx, SkScalar dy, SkScalar dz) const;

private:
    struct Rec {
        Rec*        fNext;
        SkMatrix3D  fMatrix;
    };
    Rec*        fRec;
    Rec         fInitialRec;
    SkCamera3D  fCamera;
};

#endif
