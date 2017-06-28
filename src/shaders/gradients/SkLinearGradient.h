/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLinearGradient_DEFINED
#define SkLinearGradient_DEFINED

#include "SkGradientShaderPriv.h"
#include "SkNx.h"

struct Sk4fStorage {
    float fArray[4];

    operator Sk4f() const {
        return Sk4f::Load(fArray);
    }

    Sk4fStorage& operator=(const Sk4f& src) {
        src.store(fArray);
        return *this;
    }
};

class SkLinearGradient : public SkGradientShaderBase {
public:
    SkLinearGradient(const SkPoint pts[2], const Descriptor&);

    class LinearGradientContext : public SkGradientShaderBase::GradientShaderBaseContext {
    public:
        LinearGradientContext(const SkLinearGradient&, const ContextRec&);

        void shadeSpan(int x, int y, SkPMColor dstC[], int count) override;

        struct Rec {
            Sk4fStorage fColor;
            float       fPos;
            float       fPosScale;
        };
    private:
        SkTDArray<Rec>  fRecs;
        bool            fApplyAlphaAfterInterp;

        void shade4_clamp(int x, int y, SkPMColor dstC[], int count);
        template <bool, bool> void shade4_dx_clamp(SkPMColor dstC[], int count, float fx, float dx,
                                                   float invDx, const float dither[2]);

        typedef SkGradientShaderBase::GradientShaderBaseContext INHERITED;
    };

    GradientType asAGradient(GradientInfo* info) const override;
#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(const AsFPArgs&) const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLinearGradient)

protected:
    SkLinearGradient(SkReadBuffer& buffer);
    void flatten(SkWriteBuffer& buffer) const override;
    Context* onMakeContext(const ContextRec&, SkArenaAlloc*) const override;
    Context* onMakeBurstPipelineContext(const ContextRec&, SkArenaAlloc*) const override;

    bool adjustMatrixAndAppendStages(SkArenaAlloc* alloc,
                                     SkMatrix* matrix,
                                     SkRasterPipeline* tPipeline,
                                     SkRasterPipeline* postPipeline) const final;


    sk_sp<SkShader> onMakeColorSpace(SkColorSpaceXformer* xformer) const override;

private:
    class LinearGradient4fContext;

    friend class SkGradientShader;
    typedef SkGradientShaderBase INHERITED;
    const SkPoint fStart;
    const SkPoint fEnd;
};

#endif
