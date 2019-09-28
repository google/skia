/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCornerPathEffect_DEFINED
#define SkCornerPathEffect_DEFINED

#include "include/core/SkFlattenable.h"
#include "include/core/SkPathEffect.h"

/** \class SkCornerPathEffect

    SkCornerPathEffect is a subclass of SkPathEffect that can turn sharp corners
    into various treatments (e.g. rounded corners)
*/
class SK_API SkCornerPathEffect : public SkPathEffect {
public:
    /** radius must be > 0 to have an effect. It specifies the distance from each corner
        that should be "rounded".
    */
    static sk_sp<SkPathEffect> Make(SkScalar radius) {
        return radius > 0 ? sk_sp<SkPathEffect>(new SkCornerPathEffect(radius)) : nullptr;
    }

protected:
    ~SkCornerPathEffect() override;

    explicit SkCornerPathEffect(SkScalar radius);
    void flatten(SkWriteBuffer&) const override;
    bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const override;

private:
    SK_FLATTENABLE_HOOKS(SkCornerPathEffect)

    SkScalar    fRadius;

    typedef SkPathEffect INHERITED;
};

#endif
