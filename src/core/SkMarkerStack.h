/*
* Copyright 2020 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkMarkerStack_DEFINED
#define SkMarkerStack_DEFINED

#include "include/core/SkM44.h"
#include "include/core/SkRefCnt.h"

#include <vector>

class SkMarkerStack : public SkRefCnt {
public:
    SkMarkerStack() {}

    void setMarker(uint32_t id, const SkM44& mx, void* boundary);
    bool findMarker(uint32_t id, SkM44* mx) const;
    bool findMarkerInverse(uint32_t id, SkM44* mx) const;
    void restore(void* boundary);

private:
    struct Rec {
        void*       fBoundary;
        SkM44       fMatrix;
        SkM44       fMatrixInverse;
        uint32_t    fID;
    };
    std::vector<Rec> fStack;
};

#endif
