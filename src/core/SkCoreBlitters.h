/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCoreBlitters_DEFINED
#define SkCoreBlitters_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkCPUTypes.h"
#include "src/core/SkBlitRow.h"
#include "src/core/SkBlitter.h"
#include "src/shaders/SkShaderBase.h"

#include <cstdint>
#include <optional>

class SkArenaAlloc;
class SkMatrix;
class SkRasterPipeline;
class SkShader;
class SkSurfaceProps;
struct SkIRect;
struct SkMask;

class SkRasterBlitter : public SkBlitter {
public:
    SkRasterBlitter(const SkPixmap& device) : fDevice(device) {}

protected:
    const SkPixmap fDevice;
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
    ~SkShaderBlitter() override;

protected:
    sk_sp<SkShader>         fShader;
    SkShaderBase::Context* fShaderContext;
};

///////////////////////////////////////////////////////////////////////////////

class SkARGB32_Blitter : public SkRasterBlitter {
public:
    SkARGB32_Blitter(const SkPixmap& device, const SkPaint& paint);
    void blitH(int x, int y, int width) override;
    void blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[]) override;
    void blitV(int x, int y, int height, SkAlpha alpha) override;
    void blitRect(int x, int y, int width, int height) override;
    void blitMask(const SkMask&, const SkIRect&) override;
    void blitAntiH2(int x, int y, U8CPU a0, U8CPU a1) override;
    void blitAntiV2(int x, int y, U8CPU a0, U8CPU a1) override;

protected:
    SkColor   fColor;
    SkPMColor fPMColor;
    SkAlpha   fSrcA;
};

class SkARGB32_Opaque_Blitter : public SkARGB32_Blitter {
public:
    SkARGB32_Opaque_Blitter(const SkPixmap& device, const SkPaint& paint)
            : SkARGB32_Blitter(device, paint) {
        SkASSERT(paint.getAlpha() == 0xFF);
    }
    void blitMask(const SkMask&, const SkIRect&) override;
    void blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[]) override;
    void blitAntiH2(int x, int y, U8CPU a0, U8CPU a1) override;
    void blitAntiV2(int x, int y, U8CPU a0, U8CPU a1) override;
    std::optional<DirectBlit> canDirectBlit() override;
};

class SkARGB32_Black_Blitter : public SkARGB32_Opaque_Blitter {
public:
    SkARGB32_Black_Blitter(const SkPixmap& device, const SkPaint& paint)
            : SkARGB32_Opaque_Blitter(device, paint) {
        SkASSERT(paint.getColor() == SK_ColorBLACK);
    }
    void blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[]) override;
    void blitAntiH2(int x, int y, U8CPU a0, U8CPU a1) override;
    void blitAntiV2(int x, int y, U8CPU a0, U8CPU a1) override;
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
    SkPMColor*          fBuffer;
    SkBlitRow::Proc32   fProc32;
    SkBlitRow::Proc32   fProc32Blend;
    bool fShadeDirectlyIntoDevice;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

SkBlitter* SkCreateRasterPipelineBlitter(const SkPixmap&,
                                         const SkPaint&,
                                         const SkMatrix& ctm,
                                         SkArenaAlloc*,
                                         sk_sp<SkShader> clipShader,
                                         const SkSurfaceProps& props);
// Use this if you've pre-baked a shader pipeline, including modulating with paint alpha.
SkBlitter* SkCreateRasterPipelineBlitter(const SkPixmap&, const SkPaint&,
                                         const SkRasterPipeline& shaderPipeline,
                                         bool shader_is_opaque,
                                         SkArenaAlloc*, sk_sp<SkShader> clipShader);

#endif
