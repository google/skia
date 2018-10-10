/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDescriptor_DEFINED
#define SkDescriptor_DEFINED

#include "SkScalerContext.h"
#include "SkTypes.h"

class SkMaskFilter;
class SkPathEffect;

class SkDescriptor {
public:
    SkDescriptor(const SkScalerContextRec&,
                 const SkPathEffect* = nullptr,
                 const SkMaskFilter* = nullptr);

    SkDescriptor(const SkDescriptor&)            = default;
    SkDescriptor(SkDescriptor&&)                 = default;
    SkDescriptor& operator=(const SkDescriptor&) = default;
    SkDescriptor& operator=(SkDescriptor&&)      = default;

    bool operator==(const SkDescriptor& other) const;
    bool operator!=(const SkDescriptor& other) const { return !(*this == other); }

    uint32_t                  hash() const { return fHash; }
    const SkScalerContextRec&  rec() const { return  fRec; }

private:
    uint32_t           fHash;
    SkScalerContextRec fRec;
};

#endif
