/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorFilterShader_DEFINED
#define SkColorFilterShader_DEFINED

#include "include/core/SkFlattenable.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"
#include "src/shaders/SkShaderBase.h"

class SkColorFilter;
class SkReadBuffer;
class SkWriteBuffer;
struct SkStageRec;

class SkColorFilterShader : public SkShaderBase {
public:
    static sk_sp<SkShader> Make(sk_sp<SkShader> shader, float alpha, sk_sp<SkColorFilter> filter);

    ShaderType type() const override { return ShaderType::kColorFilter; }

    sk_sp<SkShader> shader() const { return fShader; }
    sk_sp<SkColorFilterBase> filter() const { return fFilter; }
    float alpha() const { return fAlpha; }

private:
    SkColorFilterShader(sk_sp<SkShader> shader, float alpha, sk_sp<SkColorFilter> filter);

    bool isOpaque() const override;
    void flatten(SkWriteBuffer&) const override;
    bool appendStages(const SkStageRec&, const SkShaders::MatrixRec&) const override;

    SK_FLATTENABLE_HOOKS(SkColorFilterShader)

    sk_sp<SkShader>          fShader;
    sk_sp<SkColorFilterBase> fFilter;
    float fAlpha;
};

#endif
