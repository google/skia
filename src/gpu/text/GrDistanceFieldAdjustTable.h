/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDistanceFieldAdjustTable_DEFINED
#define GrDistanceFieldAdjustTable_DEFINED

#include "SkRefCnt.h"
#include "SkScalar.h"

// Distance field text needs this table to compute a value for use in the fragment shader.
// Because the GrAtlasTextContext can go out of scope before the final flush, this needs to be
// refcnted and malloced
struct GrDistanceFieldAdjustTable : public SkNVRefCnt<GrDistanceFieldAdjustTable> {
    GrDistanceFieldAdjustTable() { this->buildDistanceAdjustTables(); }
    ~GrDistanceFieldAdjustTable() {
        delete[] fTable;
        delete[] fGammaCorrectTable;
    }

    const SkScalar& getAdjustment(int i, bool useGammaCorrectTable) const {
        return useGammaCorrectTable ? fGammaCorrectTable[i] : fTable[i];
    }

private:
    void buildDistanceAdjustTables();

    SkScalar* fTable;
    SkScalar* fGammaCorrectTable;
};

#endif
