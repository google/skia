/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorShader_DEFINED
#define SkColorShader_DEFINED

#include "SkColorSpaceXformer.h"
#include "SkShaderBase.h"
#include "SkPM4f.h"

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

    class ColorShaderContext : public Context {
    public:
        ColorShaderContext(const SkColorShader& shader, const ContextRec&);

        uint32_t getFlags() const override;
        void shadeSpan(int x, int y, SkPMColor span[], int count) override;
        void shadeSpanAlpha(int x, int y, uint8_t alpha[], int count) override;
        void shadeSpan4f(int x, int y, SkPM4f[], int count) override;

    private:
        SkPM4f      fPM4f;
        SkPMColor   fPMColor;
        uint32_t    fFlags;

        typedef Context INHERITED;
    };

    GradientType asAGradient(GradientInfo* info) const override;

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(const AsFPArgs&) const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkColorShader)

protected:
    SkColorShader(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;
    Context* onMakeContext(const ContextRec&, SkArenaAlloc* storage) const override;

    bool onAsLuminanceColor(SkColor* lum) const override {
        *lum = fColor;
        return true;
    }

    bool onAppendStages(SkRasterPipeline*, SkColorSpace*, SkArenaAlloc*,
                        const SkMatrix& ctm, const SkPaint&, const SkMatrix*) const override;

    sk_sp<SkShader> onMakeColorSpace(SkColorSpaceXformer* xformer) const override {
        return SkShader::MakeColorShader(xformer->apply(fColor));
    }

private:
    SkColor fColor;

    typedef SkShaderBase INHERITED;
};

class SkColor4Shader : public SkShaderBase {
public:
    SkColor4Shader(const SkColor4f&, sk_sp<SkColorSpace>);

    bool isOpaque() const override {
        return SkColorGetA(fCachedByteColor) == 255;
    }
    bool isConstant() const override { return true; }

    class Color4Context : public Context {
    public:
        Color4Context(const SkColor4Shader& shader, const ContextRec&);

        uint32_t getFlags() const override;
        void shadeSpan(int x, int y, SkPMColor span[], int count) override;
        void shadeSpanAlpha(int x, int y, uint8_t alpha[], int count) override;
        void shadeSpan4f(int x, int y, SkPM4f[], int count) override;

    private:
        SkPM4f      fPM4f;
        SkPMColor   fPMColor;
        uint32_t    fFlags;

        typedef Context INHERITED;
    };

    GradientType asAGradient(GradientInfo* info) const override;

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(const AsFPArgs&) const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkColorShader)

protected:
    SkColor4Shader(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;
    Context* onMakeContext(const ContextRec&, SkArenaAlloc*) const override;
    bool onAsLuminanceColor(SkColor* lum) const override {
        *lum = fCachedByteColor;
        return true;
    }
    bool onAppendStages(SkRasterPipeline*, SkColorSpace*, SkArenaAlloc*,
                        const SkMatrix& ctm, const SkPaint&, const SkMatrix*) const override;

    sk_sp<SkShader> onMakeColorSpace(SkColorSpaceXformer* xformer) const override;

private:
    sk_sp<SkColorSpace> fColorSpace;
    const SkColor4f     fColor4;
    const SkColor       fCachedByteColor;

    typedef SkShaderBase INHERITED;
};

#endif
