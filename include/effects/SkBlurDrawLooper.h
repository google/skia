/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkBlurDrawLooper_DEFINED
#define SkBlurDrawLooper_DEFINED

#include "SkDrawLooper.h"
#include "SkColor.h"

class SkArenaAlloc;
class SkMaskFilter;
class SkColorFilter;

/** \class SkBlurDrawLooper
    This class draws a shadow of the object (possibly offset), and then draws
    the original object in its original position.
    should there be an option to just draw the shadow/blur layer? webkit?
*/
class SK_API SkBlurDrawLooper : public SkDrawLooper {
public:
    static sk_sp<SkDrawLooper> Make(SkColor color, SkScalar sigma, SkScalar dx, SkScalar dy) {
        return sk_sp<SkDrawLooper>(new SkBlurDrawLooper(color, sigma, dx, dy));
    }

    SkDrawLooper::Context* makeContext(SkCanvas*, SkArenaAlloc*) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkBlurDrawLooper)

protected:
    SkBlurDrawLooper(SkColor color, SkScalar sigma, SkScalar dx, SkScalar dy);

    void flatten(SkWriteBuffer&) const override;

    bool asABlurShadow(BlurShadowRec*) const override;

private:
    sk_sp<SkMaskFilter>  fBlur;
    SkScalar        fDx, fDy, fSigma;
    SkColor         fBlurColor;

    enum State {
        kBeforeEdge,
        kAfterEdge,
        kDone
    };

    class BlurDrawLooperContext : public SkDrawLooper::Context {
    public:
        explicit BlurDrawLooperContext(const SkBlurDrawLooper* looper);

        bool next(SkCanvas* canvas, SkPaint* paint) override;

    private:
        const SkBlurDrawLooper* fLooper;
        State fState;
    };

    void init(SkScalar sigma, SkScalar dx, SkScalar dy, SkColor color);
    void initEffects();

    typedef SkDrawLooper INHERITED;
};

#endif
