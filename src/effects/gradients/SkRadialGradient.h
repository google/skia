
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRadialGradient_DEFINED
#define SkRadialGradient_DEFINED

#include "SkGradientShaderPriv.h"

class SkRadialGradient : public SkGradientShaderBase {
public:
    SkRadialGradient(const SkPoint& center, SkScalar radius, const Descriptor&);

    virtual size_t contextSize() const SK_OVERRIDE;

    class RadialGradientContext : public SkGradientShaderBase::GradientShaderBaseContext {
    public:
        RadialGradientContext(const SkRadialGradient&, const ContextRec&);

        virtual void shadeSpan(int x, int y, SkPMColor dstC[], int count) SK_OVERRIDE;
        virtual void shadeSpan16(int x, int y, uint16_t dstC[], int count) SK_OVERRIDE;

    private:
        typedef SkGradientShaderBase::GradientShaderBaseContext INHERITED;
    };

    virtual BitmapType asABitmap(SkBitmap* bitmap,
                                 SkMatrix* matrix,
                                 TileMode* xy) const SK_OVERRIDE;
    virtual GradientType asAGradient(GradientInfo* info) const SK_OVERRIDE;
    virtual bool asFragmentProcessor(GrContext*, const SkPaint&, const SkMatrix*, GrColor*,
                                     GrFragmentProcessor**) const SK_OVERRIDE;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkRadialGradient)

protected:
    SkRadialGradient(SkReadBuffer& buffer);
    virtual void flatten(SkWriteBuffer& buffer) const SK_OVERRIDE;
    virtual Context* onCreateContext(const ContextRec&, void* storage) const SK_OVERRIDE;

private:
    friend class SkGradientShader;
    typedef SkGradientShaderBase INHERITED;
    const SkPoint fCenter;
    const SkScalar fRadius;
};

#endif
