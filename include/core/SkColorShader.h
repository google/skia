/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorShader_DEFINED
#define SkColorShader_DEFINED

#include "SkShader.h"

/** \class SkColorShader
    A Shader that represents a single color. In general, this effect can be
    accomplished by just using the color field on the paint, but if an
    actual shader object is needed, this provides that feature.
*/
class SK_API SkColorShader : public SkShader {
public:
    /** Create a ColorShader that ignores the color in the paint, and uses the
        specified color. Note: like all shaders, at draw time the paint's alpha
        will be respected, and is applied to the specified color.
    */
    explicit SkColorShader(SkColor c);

    virtual bool isOpaque() const SK_OVERRIDE;

    virtual size_t contextSize() const SK_OVERRIDE {
        return sizeof(ColorShaderContext);
    }

    class ColorShaderContext : public SkShader::Context {
    public:
        ColorShaderContext(const SkColorShader& shader, const ContextRec&);

        virtual uint32_t getFlags() const SK_OVERRIDE;
        virtual uint8_t getSpan16Alpha() const SK_OVERRIDE;
        virtual void shadeSpan(int x, int y, SkPMColor span[], int count) SK_OVERRIDE;
        virtual void shadeSpan16(int x, int y, uint16_t span[], int count) SK_OVERRIDE;
        virtual void shadeSpanAlpha(int x, int y, uint8_t alpha[], int count) SK_OVERRIDE;

    private:
        SkPMColor   fPMColor;
        uint32_t    fFlags;
        uint16_t    fColor16;

        typedef SkShader::Context INHERITED;
    };

    // we return false for this, use asAGradient
    virtual BitmapType asABitmap(SkBitmap* outTexture,
                                 SkMatrix* outMatrix,
                                 TileMode xy[2]) const SK_OVERRIDE;

    virtual GradientType asAGradient(GradientInfo* info) const SK_OVERRIDE;

    virtual bool asFragmentProcessor(GrContext*, const SkPaint&, const SkMatrix*, GrColor*,
                                     GrFragmentProcessor**) const SK_OVERRIDE;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkColorShader)

protected:
    SkColorShader(SkReadBuffer&);
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;
    virtual Context* onCreateContext(const ContextRec&, void* storage) const SK_OVERRIDE;
    virtual bool onAsLuminanceColor(SkColor* lum) const SK_OVERRIDE {
        *lum = fColor;
        return true;
    }

private:
    SkColor fColor;

    typedef SkShader INHERITED;
};

#endif
