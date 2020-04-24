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

#include <string>
#include <vector>

class SkMarkerStack : public SkRefCnt {
public:
    SkMarkerStack() {}

    void setMarker(const char* name, const SkM44& mx, void* boundary);
    bool findMarker(const char* name, SkM44* mx) const;
    bool findMarkerInverse(const char* name, SkM44* mx) const;
    void restore(void* boundary);

private:
    struct Rec {
        void*       fBoundary;
        SkM44       fMatrix;
        SkM44       fMatrixInverse;
        std::string fName;
    };
    std::vector<Rec> fStack;
};

#endif
