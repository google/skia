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
    GrDistanceFieldAdjustTable() { this->buildDistanceAdjustTable(); }
    ~GrDistanceFieldAdjustTable() { delete[] fTable; }

    const SkScalar& operator[] (int i) const {
        return fTable[i];
    }

private:
    void buildDistanceAdjustTable();

    SkScalar* fTable;
};

#endif
