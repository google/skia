/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorMatrix_DEFINED
#define SkColorMatrix_DEFINED

#include "SkScalar.h"

class SK_API SkColorMatrix {
#ifdef SK_SUPPORT_LEGACY_COLORMATRIX_PUBLIC
public:
#endif
    enum {
        kCount = 20
    };
    SkScalar    fMat[kCount];

    enum Elem {
        kR_Scale    = 0,
        kG_Scale    = 6,
        kB_Scale    = 12,
        kA_Scale    = 18,

        kR_Trans    = 4,
        kG_Trans    = 9,
        kB_Trans    = 14,
        kA_Trans    = 19,
    };
    void postTranslate(SkScalar rTrans, SkScalar gTrans, SkScalar bTrans,
                       SkScalar aTrans = 0);
    static bool NeedsClamping(const SkScalar[20]);
    static void SetConcat(SkScalar result[20], const SkScalar outer[20], const SkScalar inner[20]);

public:
    void setIdentity();
    void setScale(SkScalar rScale, SkScalar gScale, SkScalar bScale,
                  SkScalar aScale = SK_Scalar1);

    enum Axis {
        kR_Axis = 0,
        kG_Axis = 1,
        kB_Axis = 2
    };
    void setRotate(Axis, SkScalar degrees);
    void setSinCos(Axis, SkScalar sine, SkScalar cosine);
    void preRotate(Axis, SkScalar degrees);
    void postRotate(Axis, SkScalar degrees);

    void setConcat(const SkColorMatrix& a, const SkColorMatrix& b);
    void preConcat(const SkColorMatrix& mat) { this->setConcat(*this, mat); }
    void postConcat(const SkColorMatrix& mat) { this->setConcat(mat, *this); }

    void setSaturation(SkScalar sat);
    void setRGB2YUV();
    void setYUV2RGB();

    void postTranslate255(SkScalar r, SkScalar g, SkScalar b, SkScalar a = 0) {
        this->postTranslate(r, g, b, a);
    }

    bool operator==(const SkColorMatrix& other) const {
        return 0 == memcmp(fMat, other.fMat, sizeof(fMat));
    }

    bool operator!=(const SkColorMatrix& other) const { return !((*this) == other); }

private:
    friend class SkColorFilters;
};

#endif
