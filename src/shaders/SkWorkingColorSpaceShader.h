/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkWorkingColorSpaceShader_DEFINED
#define SkWorkingColorSpaceShader_DEFINED

#include "include/core/SkColorSpace.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/private/base/SkAssert.h"
#include "src/shaders/SkShaderBase.h"

#include <utility>

class SkReadBuffer;
class SkWriteBuffer;
struct SkStageRec;

class SkWorkingColorSpaceShader final : public SkShaderBase {
public:
    static sk_sp<SkShader> Make(sk_sp<SkShader> shader, sk_sp<SkColorSpace> workingSpace) {
        if (!shader) {
            return nullptr;
        } else if (!workingSpace) {
            return shader;
        } else {
            return sk_sp(new SkWorkingColorSpaceShader(std::move(shader), std::move(workingSpace)));
        }
    }

    ShaderType type() const override { return ShaderType::kWorkingColorSpace; }

    sk_sp<SkShader> shader() const { return fShader; }
    sk_sp<SkColorSpace> workingSpace() const { return fWorkingSpace; }

private:
    SkWorkingColorSpaceShader(sk_sp<SkShader> shader, sk_sp<SkColorSpace> workingSpace)
            : fShader(std::move(shader)), fWorkingSpace(std::move(workingSpace)) {
        SkASSERT(fShader);
        SkASSERT(fWorkingSpace);
    }

    bool appendStages(const SkStageRec& rec, const SkShaders::MatrixRec&) const override;

    friend void ::SkRegisterWorkingColorSpaceShaderFlattenable();
    SK_FLATTENABLE_HOOKS(SkWorkingColorSpaceShader)

    void flatten(SkWriteBuffer& buffer) const override;

    sk_sp<SkShader> fShader;
    sk_sp<SkColorSpace> fWorkingSpace;
};

#endif
