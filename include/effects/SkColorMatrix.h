/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorMatrix_DEFINED
#define SkColorMatrix_DEFINED

#include "SkTypes.h"
#include <memory.h>

class SK_API SkColorMatrix {
#ifdef SK_SUPPORT_LEGACY_COLORMATRIX_PUBLIC
public:
#endif
    enum {
        kCount = 20
    };
    float fMat[kCount];

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
    void postTranslate(float rTrans, float gTrans, float bTrans, float aTrans = 0);
    static bool NeedsClamping(const float[20]);
    static void SetConcat(float result[20], const float outer[20], const float inner[20]);

public:
    void setIdentity();
    void setScale(float rScale, float gScale, float bScale, float aScale = 1.0f);

    enum Axis {
        kR_Axis = 0,
        kG_Axis = 1,
        kB_Axis = 2
    };
    void setRotate(Axis, float degrees);
    void setSinCos(Axis, float sine, float cosine);
    void preRotate(Axis, float degrees);
    void postRotate(Axis, float degrees);

    void setConcat(const SkColorMatrix& a, const SkColorMatrix& b);
    void preConcat(const SkColorMatrix& mat) { this->setConcat(*this, mat); }
    void postConcat(const SkColorMatrix& mat) { this->setConcat(mat, *this); }

    void setSaturation(float sat);
    void setRGB2YUV();
    void setYUV2RGB();

    void postTranslate255(float r, float g, float b, float a = 0) {
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
