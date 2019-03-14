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
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&) const override;
#endif

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkShader> onMakeColorSpace(SkColorSpaceXformer* xformer) const override;
    bool onAppendStages(const SkStageRec&) const override;

private:
    SK_FLATTENABLE_HOOKS(SkColorFilterShader)

    sk_sp<SkShader>      fShader;
    sk_sp<SkColorFilter> fFilter;

    typedef SkShaderBase INHERITED;
};

#endif
