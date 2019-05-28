/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorMatrix_DEFINED
#define SkColorMatrix_DEFINED

#include "include/core/SkTypes.h"
#include <memory.h>

class SK_API SkColorMatrix {
public:
    void setIdentity();
    void setScale(float rScale, float gScale, float bScale, float aScale = 1.0f);

    void setRowMajor(const float src[20]) {
        memcpy(fMat, src, sizeof(fMat));
    }

    void getRowMajor(float dst[20]) const {
        memcpy(dst, fMat, sizeof(fMat));
    }

    enum Axis {
        kR_Axis = 0,
        kG_Axis = 1,
        kB_Axis = 2
    };
    void setRotate(Axis, float degrees);
    void setSinCos(Axis, float sine, float cosine);
    void preRotate(Axis, float degrees);
    void postRotate(Axis, float degrees);
    void postTranslate(float dr, float dg, float db, float da);

    void setConcat(const SkColorMatrix& a, const SkColorMatrix& b);
    void preConcat(const SkColorMatrix& mat) { this->setConcat(*this, mat); }
    void postConcat(const SkColorMatrix& mat) { this->setConcat(mat, *this); }

    void setSaturation(float sat);
    void setRGB2YUV();
    void setYUV2RGB();

    bool operator==(const SkColorMatrix& other) const {
        return 0 == memcmp(fMat, other.fMat, sizeof(fMat));
    }

    bool operator!=(const SkColorMatrix& other) const { return !((*this) == other); }

    float* get20(float m[20]) const {
        memcpy(m, fMat, sizeof(fMat));
        return m;
    }
    void set20(const float m[20]) {
        memcpy(fMat, m, sizeof(fMat));
    }

private:
    float fMat[20];

    friend class SkColorFilters;
};

#endif
