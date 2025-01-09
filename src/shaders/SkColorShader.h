/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorShader_DEFINED
#define SkColorShader_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkFlattenable.h"
#include "src/shaders/SkShaderBase.h"

class SkReadBuffer;
class SkWriteBuffer;
struct SkStageRec;

/** \class SkColorShader
    A Shader that represents a single color. In general, this effect can be accomplished by just
    using the color field on the paint, but if an actual shader object is needed, this provides that
    feature.  Note: like all shaders, at draw time the paint's alpha will be respected, and is
    applied to the specified color.
*/
class SkColorShader : public SkShaderBase {
public:
    /** Create a ColorShader wrapping the given sRGB color.
    */
    explicit SkColorShader(const SkColor4f& c) : fColor(c) {}

    bool isOpaque() const override { return fColor.isOpaque(); }
    bool isConstant() const override { return true; }

    ShaderType type() const override { return ShaderType::kColor; }

    const SkColor4f& color() const { return fColor; }

private:
    friend void ::SkRegisterColorShaderFlattenable();
    SK_FLATTENABLE_HOOKS(SkColorShader)

    void flatten(SkWriteBuffer&) const override;

    bool onAsLuminanceColor(SkColor4f* lum) const override {
        *lum = fColor;
        return true;
    }

    bool appendStages(const SkStageRec&, const SkShaders::MatrixRec&) const override;

    // The color is stored in extended sRGB, regardless of the original color space that was
    // passed into SkShaders::Color().
    const SkColor4f fColor;
};

#endif
