/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTrimImpl_DEFINED
#define SkTrimImpl_DEFINED

#include "SkPathEffect.h"

class SkTrimPE : public SkPathEffect {
public:
    SkTrimPE(SkScalar startT, SkScalar stopT);

    bool filterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkTrimPE)

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    const SkScalar fStartT;
    const SkScalar fStopT;

    typedef SkPathEffect INHERITED;
};

#endif
