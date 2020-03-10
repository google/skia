/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCoreBlitters_DEFINED
#define SkCoreBlitters_DEFINED

#include "include/core/SkPaint.h"
#include "src/core/SkBlitRow.h"
#include "src/core/SkBlitter.h"
#include "src/core/SkXfermodePriv.h"
#include "src/shaders/SkBitmapProcShader.h"
#include "src/shaders/SkShaderBase.h"

class SkRasterBlitter : public SkBlitter {
public:
    SkRasterBlitter(const SkPixmap& device) : fDevice(device) {}

protected:
    const SkPixmap fDevice;

private:
    typedef SkBlitter INHERITED;
};

class SkShaderBlitter : public SkRasterBlitter {
public:
    /**
      *  The storage for shaderContext is owned by the caller, but the object itself is not.
      *  The blitter only ensures that the storage always holds a live object, but it may
      *  exchange that object.
      */
    SkShaderBlitter(const SkPixmap& device, const SkPaint& paint,
                    SkShaderBase::Context* shaderContext);
    virtual ~SkShaderBlitter();

protected:
    uint32_t                fShaderFlags;
    const SkShader*         fShader;
    SkShaderBase::Context*  fShaderContext;
    bool                    fConstInY;

private:
    // illegal
    SkShaderBlitter& operator=(const SkShaderBlitter&);

    typedef SkRasterBlitter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class SkA8_Coverage_Blitter : public SkRasterBlitter {
public:
    SkA8_Coverage_Blitter(const SkPixmap& device, const SkPaint& paint);
    void blitH(int x, int y, int width) override;
    void blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[]) override;
    void blitV(int x, int y, int height, SkAlpha alpha) override;
    void blitRect(int x, int y, int width, int height) override;
    void blitMask(const SkMask&, const SkIRect&) override;
    const SkPixmap* justAnOpaqueColor(uint32_t*) override;

private:
    typedef SkRasterBlitter INHERITED;
};

////////////////////////////////////////////////////////////////

class SkARGB32_Blitter : public SkRasterBlitter {
public:
    SkARGB32_Blitter(const SkPixmap& device, const SkPaint& paint);
    void blitH(int x, int y, int width) override;
    void blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[]) override;
    void blitV(int x, int y, int height, SkAlpha alpha) override;
    void blitRect(int x, int y, int width, int height) override;
    void blitMask(const SkMask&, const SkIRect&) override;
    const SkPixmap* justAnOpaqueColor(uint32_t*) override;
    void blitAntiH2(int x, int y, U8CPU a0, U8CPU a1) override;
    void blitAntiV2(int x, int y, U8CPU a0, U8CPU a1) override;

protected:
    SkColor                fColor;
    SkPMColor              fPMColor;

private:
    unsigned fSrcA, fSrcR, fSrcG, fSrcB;

    // illegal
    SkARGB32_Blitter& operator=(const SkARGB32_Blitter&);

    typedef SkRasterBlitter INHERITED;
};

class SkARGB32_Opaque_Blitter : public SkARGB32_Blitter {
public:
    SkARGB32_Opaque_Blitter(const SkPixmap& device, const SkPaint& paint)
        : INHERITED(device, paint) { SkASSERT(paint.getAlpha() == 0xFF); }
    void blitMask(const SkMask&, const SkIRect&) override;
    void blitAntiH2(int x, int y, U8CPU a0, U8CPU a1) override;
    void blitAntiV2(int x, int y, U8CPU a0, U8CPU a1) override;

private:
    typedef SkARGB32_Blitter INHERITED;
};

class SkARGB32_Black_Blitter : public SkARGB32_Opaque_Blitter {
public:
    SkARGB32_Black_Blitter(const SkPixmap& device, const SkPaint& paint)
        : INHERITED(device, paint) {}
    void blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[]) override;
    void blitAntiH2(int x, int y, U8CPU a0, U8CPU a1) override;
    void blitAntiV2(int x, int y, U8CPU a0, U8CPU a1) override;

private:
    typedef SkARGB32_Opaque_Blitter INHERITED;
};

class SkARGB32_Shader_Blitter : public SkShaderBlitter {
public:
    SkARGB32_Shader_Blitter(const SkPixmap& device, const SkPaint& paint,
                            SkShaderBase::Context* shaderContext);
    ~SkARGB32_Shader_Blitter() override;
    void blitH(int x, int y, int width) override;
    void blitV(int x, int y, int height, SkAlpha alpha) override;
    void blitRect(int x, int y, int width, int height) override;
    void blitAntiH(int x, int y, const SkAlpha[], const int16_t[]) override;
    void blitMask(const SkMask&, const SkIRect&) override;

private:
    SkXfermode*         fXfermode;
    SkPMColor*          fBuffer;
    SkBlitRow::Proc32   fProc32;
    SkBlitRow::Proc32   fProc32Blend;
    bool                fShadeDirectlyIntoDevice;

    // illegal
    SkARGB32_Shader_Blitter& operator=(const SkARGB32_Shader_Blitter&);

    typedef SkShaderBlitter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

typedef void (*SkS32D16BlendProc)(uint16_t*, const SkPMColor*, int, uint8_t);

class SkRGB565_Shader_Blitter : public SkShaderBlitter {
public:
    SkRGB565_Shader_Blitter(const SkPixmap& device, const SkPaint&, SkShaderBase::Context*);
    ~SkRGB565_Shader_Blitter() override;
    void blitH(int x, int y, int width) override;
    void blitAntiH(int x, int y, const SkAlpha[], const int16_t[]) override;

    static bool Supports(const SkPixmap& device, const SkPaint&);

private:
    SkPMColor*          fBuffer;
    SkS32D16BlendProc   fBlend;
    SkS32D16BlendProc   fBlendCoverage;

    typedef SkShaderBlitter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

// Neither of these ever returns nullptr, but this first factory may return a SkNullBlitter.
SkBlitter* SkCreateRasterPipelineBlitter(const SkPixmap&, const SkPaint&, const SkMatrix& ctm,
                                         SkArenaAlloc*, sk_sp<SkShader> clipShader);
// Use this if you've pre-baked a shader pipeline, including modulating with paint alpha.
// This factory never returns an SkNullBlitter.
SkBlitter* SkCreateRasterPipelineBlitter(const SkPixmap&, const SkPaint&,
                                         const SkRasterPipeline& shaderPipeline,
                                         bool shader_is_opaque,
                                         SkArenaAlloc*, sk_sp<SkShader> clipShader);

SkBlitter* SkCreateSkVMBlitter(const SkPixmap&,
                               const SkPaint&,
                               const SkMatrix& ctm,
                               SkArenaAlloc*,
                               sk_sp<SkShader> clipShader);

#endif
