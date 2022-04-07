/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDistanceFieldAdjustTable_DEFINED
#define GrDistanceFieldAdjustTable_DEFINED

#include "include/core/SkScalar.h"

// Distance field text needs this table to compute a value for use in the fragment shader.
class GrDistanceFieldAdjustTable {
public:
    static const GrDistanceFieldAdjustTable* Get();

    ~GrDistanceFieldAdjustTable() {
        delete[] fTable;
        delete[] fGammaCorrectTable;
    }

    SkScalar getAdjustment(int i, bool useGammaCorrectTable) const {
        return useGammaCorrectTable ? fGammaCorrectTable[i] : fTable[i];
    }

private:
    GrDistanceFieldAdjustTable();

    SkScalar* fTable;
    SkScalar* fGammaCorrectTable;
};

#endif
