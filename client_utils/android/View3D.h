/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

//  Inspired by Rob Johnson's most excellent QuickDraw GX sample code

#ifndef View3D_DEFINED
#define View3D_DEFINED

#include "include/core/SkM44.h"

class SkCanvas;
class SkMatrix;

namespace android {
namespace skia {

class Patch3D {
public:
    Patch3D();

    void    reset();
    void    transform(const SkM44&, Patch3D* dst = nullptr) const;

    // dot a unit vector with the patch's normal
    SkScalar dotWith(SkScalar dx, SkScalar dy, SkScalar dz) const;
    SkScalar dotWith(const SkV3& v) const {
        return this->dotWith(v.x, v.y, v.z);
    }

    // deprecated, but still here for animator (for now)
    void rotate(SkScalar /*x*/, SkScalar /*y*/, SkScalar /*z*/) {}
    void rotateDegrees(SkScalar /*x*/, SkScalar /*y*/, SkScalar /*z*/) {}

    SkV3  fOrigin;

private:
    SkV3  fU, fV;

    friend class Camera3D;
};

class Camera3D {
public:
    Camera3D();

    void reset();
    void update();
    void patchToMatrix(const Patch3D&, SkMatrix* matrix) const;

    SkV3   fLocation;   // origin of the camera's space
    SkV3   fAxis;       // view direction
    SkV3   fZenith;     // up direction
    SkV3   fObserver;   // eye position (may not be the same as the origin)

private:
    mutable SkMatrix    fOrientation;
    mutable bool        fNeedToUpdate;

    void doUpdate() const;
};

class View3D {
public:
    View3D();
    virtual ~View3D(); // temporarily virtual until Android switches from Sk3DView.

    void save();
    void restore();

    void translate(SkScalar x, SkScalar y, SkScalar z);
    void rotateX(SkScalar deg);
    void rotateY(SkScalar deg);
    void rotateZ(SkScalar deg);

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    void setCameraLocation(SkScalar x, SkScalar y, SkScalar z);
    SkScalar getCameraLocationX() const;
    SkScalar getCameraLocationY() const;
    SkScalar getCameraLocationZ() const;
#endif

    void getMatrix(SkMatrix*) const;
    void applyToCanvas(SkCanvas*) const;

    SkScalar dotWithNormal(SkScalar dx, SkScalar dy, SkScalar dz) const;

private:
    struct Rec {
        Rec*    fNext;
        SkM44   fMatrix;
    };
    Rec*     fRec;
    Rec      fInitialRec;
    Camera3D fCamera;

    View3D(const View3D&) = delete;
    View3D& operator=(const View3D&) = delete;
};
} // namespace skia
} // namespace android
#endif // View3D_DEFINED
