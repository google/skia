/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTrimImpl_DEFINED
#define SkTrimImpl_DEFINED

#include "SkFlattenablePriv.h"
#include "SkPathEffect.h"

#include "SkTrimPathEffect.h"

class SkTrimPE : public SkPathEffect {
public:
    SkTrimPE(SkScalar startT, SkScalar stopT, SkTrimPathEffect::Mode);

    bool filterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const override;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkTrimPE)

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    const SkScalar               fStartT,
                                 fStopT;
    const SkTrimPathEffect::Mode fMode;

    typedef SkPathEffect INHERITED;
};

#endif
