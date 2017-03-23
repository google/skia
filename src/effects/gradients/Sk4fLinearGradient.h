/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Sk4fLinearGradient_DEFINED
#define Sk4fLinearGradient_DEFINED

#include "Sk4fGradientBase.h"
#include "SkLinearGradient.h"

class SkLinearGradient::
LinearGradient4fContext final : public GradientShaderBase4fContext {
public:
    LinearGradient4fContext(const SkLinearGradient&, const ContextRec&);

    void shadeSpan(int x, int y, SkPMColor dst[], int count) override;
    void shadeSpan4f(int x, int y, SkPM4f dst[], int count) override;

protected:
    void mapTs(int x, int y, SkScalar ts[], int count) const override;

    bool onChooseBlitProcs(const SkImageInfo&, BlitState*) override;

private:
    using INHERITED = GradientShaderBase4fContext;

    template<DstType, ApplyPremul, TileMode>
    class LinearIntervalProcessor;

    template <DstType dstType, ApplyPremul premul>
    void shadePremulSpan(int x, int y, typename DstTraits<dstType, premul>::Type[],
                         int count) const;

    template <DstType dstType, ApplyPremul premul, SkShader::TileMode tileMode>
    void shadeSpanInternal(int x, int y, typename DstTraits<dstType, premul>::Type[],
                           int count) const;

    const Sk4fGradientInterval* findInterval(SkScalar fx) const;

    bool isFast() const { return fDstToPosClass == kLinear_MatrixClass; }

    static void D32_BlitBW(BlitState*, int x, int y, const SkPixmap& dst, int count);
    static void D64_BlitBW(BlitState*, int x, int y, const SkPixmap& dst, int count);

    mutable const Sk4fGradientInterval* fCachedInterval;
};

#endif // Sk4fLinearGradient_DEFINED
