/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlurMaskFilterImpl_DEFINED
#define SkBlurMaskFilterImpl_DEFINED

#include "include/core/SkFlattenable.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskFilterBase.h"

#include <optional>
#include <utility>

class SkImageFilter;
class SkMatrix;
class SkPaint;
class SkRRect;
class SkReadBuffer;
class SkResourceCache;
class SkWriteBuffer;
enum SkBlurStyle : int;
struct SkIPoint;
struct SkIRect;
struct SkRect;
template <typename T> class sk_sp;

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
    std::pair<sk_sp<SkImageFilter>, bool> asImageFilter(const SkMatrix& ctm,
                                                        const SkPaint&) const override;

    SkScalar computeXformedSigma(const SkMatrix& ctm) const;
    SkBlurStyle blurStyle() const {return fBlurStyle;}
    SkScalar sigma() const {return fSigma;}
    bool ignoreXform() const { return !fRespectCTM; }

private:
    FilterReturn filterRectsToNine(SkSpan<const SkRect>,
                                   const SkMatrix&,
                                   const SkIRect& clipBounds,
                                   std::optional<NinePatch>*,
                                   SkResourceCache*) const override;

    std::optional<NinePatch> filterRRectToNine(const SkRRect&,
                                               const SkMatrix&,
                                               const SkIRect& clipBounds,
                                               SkResourceCache*) const override;

    bool filterRectMask(SkMaskBuilder* dstM,
                        const SkRect& r,
                        const SkMatrix& matrix,
                        SkIPoint* margin,
                        SkMaskBuilder::CreateMode createMode) const;

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
