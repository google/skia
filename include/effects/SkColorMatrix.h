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

    // DEPRECATED: use postTranslate()
    void postTranslate255(float r, float g, float b, float a = 0) {
        const float scale = 1.0f / 255;
        this->postTranslate(r * scale, g * scale, b * scale, a * scale);
    }

    bool operator==(const SkColorMatrix& other) const {
        return 0 == memcmp(fMat, other.fMat, sizeof(fMat));
    }

    bool operator!=(const SkColorMatrix& other) const { return !((*this) == other); }

private:
    float fMat[20];

    friend class SkColorFilters;
};

#endif
