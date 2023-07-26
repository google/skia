/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlendShader_DEFINED
#define SkBlendShader_DEFINED

#include "include/core/SkFlattenable.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "src/shaders/SkShaderBase.h"

#include <utility>

class SkReadBuffer;
class SkWriteBuffer;
enum class SkBlendMode;
struct SkStageRec;

class SkBlendShader final : public SkShaderBase {
public:
    SkBlendShader(SkBlendMode mode, sk_sp<SkShader> dst, sk_sp<SkShader> src)
            : fDst(std::move(dst)), fSrc(std::move(src)), fMode(mode) {}

    ShaderType type() const override { return ShaderType::kBlend; }

    sk_sp<SkShader> dst() const { return fDst; }
    sk_sp<SkShader> src() const { return fSrc; }
    SkBlendMode mode() const { return fMode; }

protected:
    SkBlendShader(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;
    bool appendStages(const SkStageRec&, const SkShaders::MatrixRec&) const override;

private:
    friend void ::SkRegisterBlendShaderFlattenable();
    SK_FLATTENABLE_HOOKS(SkBlendShader)

    sk_sp<SkShader> fDst;
    sk_sp<SkShader> fSrc;
    SkBlendMode fMode;
};

#endif
