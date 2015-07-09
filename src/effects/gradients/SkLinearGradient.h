/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLinearGradient_DEFINED
#define SkLinearGradient_DEFINED

#include "SkGradientShaderPriv.h"

class SkLinearGradient : public SkGradientShaderBase {
public:
    SkLinearGradient(const SkPoint pts[2], const Descriptor&);

    size_t contextSize() const override;

    class LinearGradientContext : public SkGradientShaderBase::GradientShaderBaseContext {
    public:
        LinearGradientContext(const SkLinearGradient&, const ContextRec&);
        ~LinearGradientContext() {}

        void shadeSpan(int x, int y, SkPMColor dstC[], int count) override;
        void shadeSpan16(int x, int y, uint16_t dstC[], int count) override;

    private:
        typedef SkGradientShaderBase::GradientShaderBaseContext INHERITED;
    };

    BitmapType asABitmap(SkBitmap*, SkMatrix*, TileMode*) const override;
    GradientType asAGradient(GradientInfo* info) const override;
    bool asFragmentProcessor(GrContext*, const SkPaint&, const SkMatrix& viewM,
                             const SkMatrix*, GrColor*, GrProcessorDataManager*,
                             GrFragmentProcessor**) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLinearGradient)

protected:
    SkLinearGradient(SkReadBuffer& buffer);
    void flatten(SkWriteBuffer& buffer) const override;
    Context* onCreateContext(const ContextRec&, void* storage) const override;

private:
    friend class SkGradientShader;
    typedef SkGradientShaderBase INHERITED;
    const SkPoint fStart;
    const SkPoint fEnd;
};

#endif
