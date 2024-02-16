/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/shaders/SkColorShader.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/private/base/SkTPin.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkShaderBase.h"

#include <utility>

SkColorShader::SkColorShader(SkColor c) : fColor(c) {}

bool SkColorShader::isOpaque() const {
    return SkColorGetA(fColor) == 255;
}

sk_sp<SkFlattenable> SkColorShader::CreateProc(SkReadBuffer& buffer) {
    return sk_make_sp<SkColorShader>(buffer.readColor());
}

void SkColorShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeColor(fColor);
}

SkColor4Shader::SkColor4Shader(const SkColor4f& color, sk_sp<SkColorSpace> space)
    : fColorSpace(std::move(space))
    , fColor({color.fR, color.fG, color.fB, SkTPin(color.fA, 0.0f, 1.0f)})
{}

sk_sp<SkFlattenable> SkColor4Shader::CreateProc(SkReadBuffer& buffer) {
    SkColor4f color;
    sk_sp<SkColorSpace> colorSpace;
    buffer.readColor4f(&color);
    if (buffer.readBool()) {
        sk_sp<SkData> data = buffer.readByteArrayAsData();
        colorSpace = data ? SkColorSpace::Deserialize(data->data(), data->size()) : nullptr;
    }
    return SkShaders::Color(color, std::move(colorSpace));
}

void SkColor4Shader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeColor4f(fColor);
    sk_sp<SkData> colorSpaceData = fColorSpace ? fColorSpace->serialize() : nullptr;
    if (colorSpaceData) {
        buffer.writeBool(true);
        buffer.writeDataAsByteArray(colorSpaceData.get());
    } else {
        buffer.writeBool(false);
    }
}

bool SkColorShader::appendStages(const SkStageRec& rec, const SkShaders::MatrixRec&) const {
    SkColor4f color = SkColor4f::FromColor(fColor);
    SkColorSpaceXformSteps(sk_srgb_singleton(), kUnpremul_SkAlphaType,
                           rec.fDstCS,          kUnpremul_SkAlphaType).apply(color.vec());
    rec.fPipeline->appendConstantColor(rec.fAlloc, color.premul().vec());
    return true;
}

bool SkColor4Shader::appendStages(const SkStageRec& rec, const SkShaders::MatrixRec&) const {
    SkColor4f color = fColor;
    SkColorSpaceXformSteps(fColorSpace.get(), kUnpremul_SkAlphaType,
                           rec.fDstCS,        kUnpremul_SkAlphaType).apply(color.vec());
    rec.fPipeline->appendConstantColor(rec.fAlloc, color.premul().vec());
    return true;
}

bool SkColor4Shader::onAsLuminanceColor(SkColor4f* lum) const {
    SkColor4f color = fColor;
    SkColorSpaceXformSteps(fColorSpace.get(),   kUnpremul_SkAlphaType,
                           sk_srgb_singleton(), kUnpremul_SkAlphaType).apply(color.vec());
    *lum = color;
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void SkRegisterColor4ShaderFlattenable() {
    SK_REGISTER_FLATTENABLE(SkColor4Shader);
}

void SkRegisterColorShaderFlattenable() {
    SK_REGISTER_FLATTENABLE(SkColorShader);
}

namespace SkShaders {
sk_sp<SkShader> Color(SkColor color) { return sk_make_sp<SkColorShader>(color); }

sk_sp<SkShader> Color(const SkColor4f& color, sk_sp<SkColorSpace> space) {
    if (!SkScalarsAreFinite(color.vec(), 4)) {
        return nullptr;
    }
    return sk_make_sp<SkColor4Shader>(color, std::move(space));
}
}  // namespace SkShaders
