
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSweepGradient_DEFINED
#define SkSweepGradient_DEFINED

#include "SkGradientShaderPriv.h"

class SkSweepGradient : public SkGradientShaderBase {
public:
    SkSweepGradient(SkScalar cx, SkScalar cy, const Descriptor&);

    size_t contextSize() const SK_OVERRIDE;

    class SweepGradientContext : public SkGradientShaderBase::GradientShaderBaseContext {
    public:
        SweepGradientContext(const SkSweepGradient& shader, const ContextRec&);

        void shadeSpan(int x, int y, SkPMColor dstC[], int count) SK_OVERRIDE;
        void shadeSpan16(int x, int y, uint16_t dstC[], int count) SK_OVERRIDE;

    private:
        typedef SkGradientShaderBase::GradientShaderBaseContext INHERITED;
    };

    virtual BitmapType asABitmap(SkBitmap* bitmap,
                                 SkMatrix* matrix,
                                 TileMode* xy) const SK_OVERRIDE;

    GradientType asAGradient(GradientInfo* info) const SK_OVERRIDE;

    virtual bool asFragmentProcessor(GrContext*, const SkPaint&, const SkMatrix& viewM,
                                     const SkMatrix*, GrColor*,
                                     GrFragmentProcessor**) const SK_OVERRIDE;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkSweepGradient)

protected:
    void flatten(SkWriteBuffer& buffer) const SK_OVERRIDE;
    Context* onCreateContext(const ContextRec&, void* storage) const SK_OVERRIDE;

private:
    const SkPoint fCenter;

    friend class SkGradientShader;
    typedef SkGradientShaderBase INHERITED;
};

#endif
