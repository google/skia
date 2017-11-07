/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPointPriv_DEFINED
#define SkPointPriv_DEFINED

#include "SkPoint.h"

class SkPointPriv {
public:
    // counter-clockwise fan
    static void SetRectFan(SkPoint v[], SkScalar l, SkScalar t, SkScalar r, SkScalar b,
            size_t stride) {
        SkASSERT(stride >= sizeof(SkPoint));

        ((SkPoint*)((intptr_t)v + 0 * stride))->set(l, t);
        ((SkPoint*)((intptr_t)v + 1 * stride))->set(l, b);
        ((SkPoint*)((intptr_t)v + 2 * stride))->set(r, b);
        ((SkPoint*)((intptr_t)v + 3 * stride))->set(r, t);
    }

    // tri strip with two counter-clockwise triangles
    static void SetRectTriStrip(SkPoint v[], SkScalar l, SkScalar t, SkScalar r, SkScalar b,
            size_t stride) {
        SkASSERT(stride >= sizeof(SkPoint));

        ((SkPoint*)((intptr_t)v + 0 * stride))->set(l, t);
        ((SkPoint*)((intptr_t)v + 1 * stride))->set(l, b);
        ((SkPoint*)((intptr_t)v + 2 * stride))->set(r, t);
        ((SkPoint*)((intptr_t)v + 3 * stride))->set(r, b);
    }
};

#endif
