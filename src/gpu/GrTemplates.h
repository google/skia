/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTemplates_DEFINED
#define GrTemplates_DEFINED

#include "SkTypes.h"

/**
 *  Use to cast a ptr to a different type, and maintain strict-aliasing
 */
template <typename Dst, typename Src> Dst GrTCast(Src src) {
    union {
        Src src;
        Dst dst;
    } data;
    data.src = src;
    return data.dst;
}

#endif
