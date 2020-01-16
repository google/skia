/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCanvasMatrix_DEFINED
#define SkCanvasMatrix_DEFINED

#include "include/core/SkMatrix.h"
#include "include/private/SkM44.h"

class SkCanvasMatrix : public SkM44 {
public:
    SkCanvasMatrix& operator=(const SkMatrix& other) {
        this->SkM44::operator=(other);
        return *this;
    }

    void reset() { this->setIdentity(); }

    operator SkMatrix() const { return this->asM33(); }
    // the legacy check was just for the 3x3 portion, so we only check those
    bool isScaleTranslate() const {
        return this->atColMajor(1) == 0 && this->atColMajor(3) == 0 &&
               this->atColMajor(4) == 0 && this->atColMajor(7) == 0 &&
               this->atColMajor(15) == 1;
    }
    bool rectStaysRect() const { return this->asM33().rectStaysRect(); }

    float getScaleX() const { return this->atColMajor(0); }
    float getScaleY() const { return this->atColMajor(5); }
    float getTranslateX() const { return this->atColMajor(12); }
    float getTranslateY() const { return this->atColMajor(13); }

    bool mapRect(SkRect* dst, const SkRect& src) { return this->asM33().mapRect(dst, src); }
};

#endif
