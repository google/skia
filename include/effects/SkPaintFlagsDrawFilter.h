
/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPaintFlagsDrawFilter_DEFINED
#define SkPaintFlagsDrawFilter_DEFINED

#include "SkDrawFilter.h"

class SkPaintFlagsDrawFilter : public SkDrawFilter {
public:
    SkPaintFlagsDrawFilter(uint32_t clearFlags, uint32_t setFlags);
    
    // overrides
    virtual void filter(SkPaint*, Type);
    
private:
    uint32_t    fPrevFlags;     // local cache for filter/restore
    uint16_t    fClearFlags;    // user specified
    uint16_t    fSetFlags;      // user specified
};

#endif

