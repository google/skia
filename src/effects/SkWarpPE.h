/*
 * Copyright 2018 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkWarpPE_DEFINED
#define SkWarpPE_DEFINED

#include "SkPathEffect.h"

class SkWarpPE : public SkPathEffect {
    SkRect  fSrc;
    SkPoint fDst[9];

public:
    SkWarpPE(const SkRect& src, const SkPoint dst[4]) : fSrc(src) {
        memcpy(fDst, dst, 9 * sizeof(SkPoint));
    }

    bool filterPath(SkPath* dst, const SkPath& src, SkStrokeRec* rec, const SkRect*) const override;

    Factory getFactory() const override { return CreateProc; }

    void flatten(SkWriteBuffer& buffer) const override;
    static sk_sp<SkFlattenable> CreateProc(SkReadBuffer& buffer);

    typedef SkPathEffect INHERITED;
};

#endif

