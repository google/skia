/*
 * Copyright 2018 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef FuzzCommon_DEFINED
#define FuzzCommon_DEFINED

#include "Fuzz.h"
#include "SkPath.h"
#include "SkRegion.h"

template <>
inline void Fuzz::next(SkRegion* region) {
    uint8_t N;
    this->nextRange(&N, 0, 10);
    for (uint8_t i = 0; i < N; ++i) {
        SkIRect r;
        uint8_t op;
        this->next(&r);
        r.sort();
        this->nextRange(&op, 0, (uint8_t)SkRegion::kLastOp);
        if (!region->op(r, (SkRegion::Op)op)) {
            return;
        }
    }
}

// allows some float values for path points
void FuzzPath(Fuzz* fuzz, SkPath* path, int maxOps);
// allows all float values for path points
void BuildPath(Fuzz* fuzz, SkPath* path, int last_verb);

#endif
