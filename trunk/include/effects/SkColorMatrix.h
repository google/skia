
/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkColorMatrix_DEFINED
#define SkColorMatrix_DEFINED

#include "SkScalar.h"

class SkColorMatrix {
public:
    SkScalar    fMat[20];

    void setIdentity();
    void setScale(SkScalar rScale, SkScalar gScale, SkScalar bScale,
                  SkScalar aScale = SK_Scalar1);
    void preScale(SkScalar rScale, SkScalar gScale, SkScalar bScale,
                  SkScalar aScale = SK_Scalar1);
    void postScale(SkScalar rScale, SkScalar gScale, SkScalar bScale,
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
};

#endif
