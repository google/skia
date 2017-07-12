/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorFilterShader_DEFINED
#define SkColorFilterShader_DEFINED

#include "SkColorFilter.h"
#include "SkShaderBase.h"

class SkArenaAlloc;

class SkColorFilterShader : public SkShaderBase {
public:
    SkColorFilterShader(sk_sp<SkShader> shader, sk_sp<SkColorFilter> filter);

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(const AsFPArgs&) const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkColorFilterShader)

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkShader> onMakeColorSpace(SkColorSpaceXformer* xformer) const override;
    bool onAppendStages(SkRasterPipeline*, SkColorSpace* dstCS, SkArenaAlloc*,
                        const SkMatrix&, const SkPaint&, const SkMatrix* localM) const override;
    bool onIsRasterPipelineOnly() const override { return true; }

private:
    sk_sp<SkShader>      fShader;
    sk_sp<SkColorFilter> fFilter;

    typedef SkShaderBase INHERITED;
};

#endif
