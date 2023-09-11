/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sktext_gpu_DistanceFieldAdjustTable_DEFINED
#define sktext_gpu_DistanceFieldAdjustTable_DEFINED

#include "include/core/SkScalar.h"

template <typename T> class SkNoDestructor;

namespace sktext::gpu {

// Distance field text needs this table to compute a value for use in the fragment shader.
class DistanceFieldAdjustTable {
public:
    static const DistanceFieldAdjustTable* Get();

    ~DistanceFieldAdjustTable() {
        delete[] fTable;
        delete[] fGammaCorrectTable;
    }

    SkScalar getAdjustment(int i, bool useGammaCorrectTable) const {
        return useGammaCorrectTable ? fGammaCorrectTable[i] : fTable[i];
    }

private:
    DistanceFieldAdjustTable();

    SkScalar* fTable;
    SkScalar* fGammaCorrectTable;

    friend class SkNoDestructor<DistanceFieldAdjustTable>;
};

}  // namespace sktext::gpu

#endif
