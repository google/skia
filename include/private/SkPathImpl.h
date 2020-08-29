/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathImpl_DEFINED
#define SkPathImpl_DEFINED

#include "include/core/SkTypes.h"

enum SkPathConvexDir : uint8_t {
    kUnknown,
    kConcave,
    kConvex_CW,
    kConvex_CCW,
};

static inline bool SkPathConvexDir_isConvex(SkPathConvexDir cd) {
    return cd == SkPathConvexDir::kConvex_CW || cd == SkPathConvexDir::kConvex_CCW;
}

static inline SkPathConvexDir SkPathConvexDir_reverse_dir(SkPathConvexDir cd) {
    SkASSERT(SkPathConvexDir_isConvex(cd));
    if (cd == SkPathConvexDir::kConvex_CW) {
        cd = SkPathConvexDir::kConvex_CCW;
    } else if (cd == SkPathConvexDir::kConvex_CCW) {
        cd = SkPathConvexDir::kConvex_CW;
    }
    return cd;
}

#endif
