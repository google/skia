/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDiscretePathEffect_DEFINED
#define SkDiscretePathEffect_DEFINED

#include "SkPathEffect.h"

/** \class SkDiscretePathEffect

    This path effect chops a path into discrete segments, and randomly displaces them.
*/
class SK_API SkDiscretePathEffect : public SkPathEffect {
public:
    /** Break the path into segments of segLength length, and randomly move the endpoints
        away from the original path by a maximum of deviation.
        Note: works on filled or framed paths

        @param seedAssist This is a caller-supplied seedAssist that modifies
                          the seed value that is used to randomize the path
                          segments' endpoints. If not supplied it defaults to 0,
                          in which case filtering a path multiple times will
                          result in the same set of segments (this is useful for
                          testing). If a caller does not want this behaviour
                          they can pass in a different seedAssist to get a
                          different set of path segments.
    */
    static sk_sp<SkPathEffect> Make(SkScalar segLength, SkScalar dev, uint32_t seedAssist = 0);

    virtual bool filterPath(SkPath* dst, const SkPath& src,
                            SkStrokeRec*, const SkRect*) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDiscretePathEffect)

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    bool exposedInAndroidJavaAPI() const override { return true; }
#endif

protected:
    SkDiscretePathEffect(SkScalar segLength,
                         SkScalar deviation,
                         uint32_t seedAssist);
    void flatten(SkWriteBuffer&) const override;

private:
    SkScalar fSegLength, fPerterb;

    /* Caller-supplied 32 bit seed assist */
    uint32_t fSeedAssist;

    typedef SkPathEffect INHERITED;
};

#endif
