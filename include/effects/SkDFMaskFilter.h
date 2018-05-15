/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDFMaskFilter_DEFINED
#define SkDFMaskFilter_DEFINED

#include "SkMaskFilter.h"

class SK_API SkDFMaskFilter : public SkMaskFilter {
public:
    struct Rec {
        float   fBaseX;
        float   fBaseY;
        int     fOctaves;
        float   fZ;
    };
    static sk_sp<SkMaskFilter> Make(const Rec&);
};

#endif

