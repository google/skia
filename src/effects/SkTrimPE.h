/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTrimImpl_DEFINED
#define SkTrimImpl_DEFINED

#include "include/core/SkPathEffect.h"

#include "include/effects/SkTrimPathEffect.h"

class SkTrimPE : public SkPathEffect {
public:
    SkTrimPE(SkScalar startT, SkScalar stopT, SkTrimPathEffect::Mode);

protected:
    void flatten(SkWriteBuffer&) const override;
    bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const override;

private:
    SK_FLATTENABLE_HOOKS(SkTrimPE)

    const SkScalar               fStartT,
                                 fStopT;
    const SkTrimPathEffect::Mode fMode;

    typedef SkPathEffect INHERITED;
};

#endif
