/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlurMaskFilterImpl_DEFINED
#define SkBlurMaskFilterImpl_DEFINED

#include "include/core/SkFlattenable.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskFilterBase.h"

class SkImageFilter;
class SkMatrix;
class SkRRect;
class SkReadBuffer;
class SkWriteBuffer;
enum SkBlurStyle : int;
struct SkIPoint;
struct SkIRect;
struct SkRect;
template<typename T> class SkTLazy;

class SkBlurMaskFilterImpl : public SkMaskFilterBase {
public:
    SkBlurMaskFilterImpl(SkScalar sigma, SkBlurStyle, bool respectCTM);

    // From SkMaskFilterBase.h
    SkMask::Format getFormat() const override;
    bool filterMask(SkMaskBuilder* dst, const SkMask& src, const SkMatrix&,
                    SkIPoint* margin) const override;
    SkMaskFilterBase::Type type() const override { return SkMaskFilterBase::Type::kBlur; }

    void computeFastBounds(const SkRect&, SkRect*) const override;
    bool asABlur(BlurRec*) const override;
    sk_sp<SkImageFilter> asImageFilter(const SkMatrix& ctm) const override;


    SkScalar computeXformedSigma(const SkMatrix& ctm) const;
    SkBlurStyle blurStyle() const {return fBlurStyle;}
    SkScalar sigma() const {return fSigma;}
    bool ignoreXform() const { return !fRespectCTM; }

private:
    FilterReturn filterRectsToNine(const SkRect[], int count, const SkMatrix&,
                                   const SkIRect& clipBounds,
                                   SkTLazy<NinePatch>*) const override;

    FilterReturn filterRRectToNine(const SkRRect&, const SkMatrix&,
                                   const SkIRect& clipBounds,
                                   SkTLazy<NinePatch>*) const override;

    bool filterRectMask(SkMaskBuilder* dstM, const SkRect& r, const SkMatrix& matrix,
                        SkIPoint* margin, SkMaskBuilder::CreateMode createMode) const;
    bool filterRRectMask(SkMaskBuilder* dstM, const SkRRect& r, const SkMatrix& matrix,
                        SkIPoint* margin, SkMaskBuilder::CreateMode createMode) const;

    SK_FLATTENABLE_HOOKS(SkBlurMaskFilterImpl)

    SkScalar    fSigma;
    SkBlurStyle fBlurStyle;
    bool        fRespectCTM;

    SkBlurMaskFilterImpl(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;

    friend class SkBlurMaskFilter;

    friend void sk_register_blur_maskfilter_createproc();
};

#endif
