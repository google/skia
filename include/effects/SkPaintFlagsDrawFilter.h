/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPaintFlagsDrawFilter_DEFINED
#define SkPaintFlagsDrawFilter_DEFINED

#include "SkDrawFilter.h"

class SK_API SkPaintFlagsDrawFilter : public SkDrawFilter {
public:
    SkPaintFlagsDrawFilter(uint32_t clearFlags, uint32_t setFlags);

    virtual bool filter(SkPaint*, Type) SK_OVERRIDE;

private:
    uint16_t    fClearFlags;    // user specified
    uint16_t    fSetFlags;      // user specified
};

#endif
