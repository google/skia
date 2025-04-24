/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShaderMaskFilterImpl_DEFINED
#define SkShaderMaskFilterImpl_DEFINED

#include "include/core/SkFlattenable.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskFilterBase.h"

#include <utility>

class SkImageFilter;
class SkMatrix;
class SkPaint;
class SkReadBuffer;
class SkWriteBuffer;
struct SkIPoint;

class SkShaderMaskFilterImpl : public SkMaskFilterBase {
public:
    SkShaderMaskFilterImpl(sk_sp<SkShader> shader) : fShader(std::move(shader)) {}

    SkMask::Format getFormat() const override { return SkMask::kA8_Format; }
    SkMaskFilterBase::Type type() const override { return SkMaskFilterBase::Type::kShader; }

    bool filterMask(SkMaskBuilder* dst, const SkMask& src, const SkMatrix&,
                    SkIPoint* margin) const override;
    std::pair<sk_sp<SkImageFilter>, bool> asImageFilter(const SkMatrix&,
                                                        const SkPaint&) const override;

    void computeFastBounds(const SkRect& src, SkRect* dst) const override {
        *dst = src;
    }

    bool asABlur(BlurRec*) const override { return false; }
    sk_sp<SkShader> shader() const { return fShader; }

private:
    SK_FLATTENABLE_HOOKS(SkShaderMaskFilterImpl)

    sk_sp<SkShader> fShader;

    SkShaderMaskFilterImpl(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;

    friend class SkShaderMaskFilter;
};

#endif
