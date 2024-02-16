/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorShader_DEFINED
#define SkColorShader_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkRefCnt.h"
#include "src/shaders/SkShaderBase.h"

class SkReadBuffer;
class SkWriteBuffer;
struct SkStageRec;

/** \class SkColorShader
    A Shader that represents a single color. In general, this effect can be
    accomplished by just using the color field on the paint, but if an
    actual shader object is needed, this provides that feature.
*/
class SkColorShader : public SkShaderBase {
public:
    /** Create a ColorShader that ignores the color in the paint, and uses the
        specified color. Note: like all shaders, at draw time the paint's alpha
        will be respected, and is applied to the specified color.
    */
    explicit SkColorShader(SkColor c);

    bool isOpaque() const override;
    bool isConstant() const override { return true; }

    ShaderType type() const override { return ShaderType::kColor; }

    SkColor color() const { return fColor; }

private:
    friend void ::SkRegisterColorShaderFlattenable();
    SK_FLATTENABLE_HOOKS(SkColorShader)

    void flatten(SkWriteBuffer&) const override;

    bool onAsLuminanceColor(SkColor4f* lum) const override {
        *lum = SkColor4f::FromColor(fColor);
        return true;
    }

    bool appendStages(const SkStageRec&, const SkShaders::MatrixRec&) const override;

    SkColor fColor;
};

class SkColor4Shader : public SkShaderBase {
public:
    SkColor4Shader(const SkColor4f&, sk_sp<SkColorSpace>);

    bool isOpaque() const override { return fColor.isOpaque(); }
    bool isConstant() const override { return true; }

    ShaderType type() const override { return ShaderType::kColor4; }

    sk_sp<SkColorSpace> colorSpace() const { return fColorSpace; }
    SkColor4f color() const { return fColor; }

private:
    friend void ::SkRegisterColor4ShaderFlattenable();
    SK_FLATTENABLE_HOOKS(SkColor4Shader)

    void flatten(SkWriteBuffer&) const override;
    bool onAsLuminanceColor(SkColor4f* lum) const override;
    bool appendStages(const SkStageRec&, const SkShaders::MatrixRec&) const override;

    sk_sp<SkColorSpace> fColorSpace;
    const SkColor4f fColor;
};

#endif
