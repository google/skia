/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRSXform_DEFINED
#define SkRSXform_DEFINED

#include "SkScalar.h"

/**
 *  A compressed form of a rotation+scale matrix.
 *
 *  [ fSCos     -fSSin    fTx ]
 *  [ fSSin      fSCos    fTy ]
 *  [     0          0      1 ]
 */
struct SkRSXform {
    SkScalar    fSCos;
    SkScalar    fSSin;
    SkScalar    fTx;
    SkScalar    fTy;

    bool rectStaysRect() const {
        return 0 == fSCos || 0 == fSSin;
    }
    
    void setIdentity() {
        fSCos = 1;
        fSSin = fTx = fTy = 0;
    }

    void set(SkScalar scos, SkScalar ssin, SkScalar tx, SkScalar ty) {
        fSCos = scos;
        fSSin = ssin;
        fTx = tx;
        fTy = ty;
    }

    void toQuad(SkScalar width, SkScalar height, SkPoint quad[4]) const;
    void toQuad(const SkSize& size, SkPoint quad[4]) const {
        this->toQuad(size.width(), size.height(), quad);
    }
};

#endif

