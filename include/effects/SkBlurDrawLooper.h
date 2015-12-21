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

class SkMaskFilter;
class SkColorFilter;

/** \class SkBlurDrawLooper
    This class draws a shadow of the object (possibly offset), and then draws
    the original object in its original position.
    should there be an option to just draw the shadow/blur layer? webkit?
*/
class SK_API SkBlurDrawLooper : public SkDrawLooper {
public:
    enum BlurFlags {
        kNone_BlurFlag = 0x00,
        /**
            The blur layer's dx/dy/radius aren't affected by the canvas
            transform.
        */
        kIgnoreTransform_BlurFlag   = 0x01,
        kOverrideColor_BlurFlag     = 0x02,
        kHighQuality_BlurFlag       = 0x04,
        /** mask for all blur flags */
        kAll_BlurFlag               = 0x07
    };

    static SkDrawLooper* Create(SkColor color, SkScalar sigma, SkScalar dx, SkScalar dy,
                                uint32_t flags = kNone_BlurFlag) {
        return new SkBlurDrawLooper(color, sigma, dx, dy, flags);
    }

    virtual ~SkBlurDrawLooper();

    SkDrawLooper::Context* createContext(SkCanvas*, void* storage) const override;

    size_t contextSize() const override { return sizeof(BlurDrawLooperContext); }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkBlurDrawLooper)

protected:
    SkBlurDrawLooper(SkColor color, SkScalar sigma, SkScalar dx, SkScalar dy,
                     uint32_t flags);

    void flatten(SkWriteBuffer&) const override;

    bool asABlurShadow(BlurShadowRec*) const override;

private:
    SkMaskFilter*   fBlur;
    SkColorFilter*  fColorFilter;
    SkScalar        fDx, fDy, fSigma;
    SkColor         fBlurColor;
    uint32_t        fBlurFlags;

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

    void init(SkScalar sigma, SkScalar dx, SkScalar dy, SkColor color, uint32_t flags);
    void initEffects();

    typedef SkDrawLooper INHERITED;
};

#endif
