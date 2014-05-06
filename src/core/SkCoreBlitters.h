/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCoreBlitters_DEFINED
#define SkCoreBlitters_DEFINED

#include "SkBitmapProcShader.h"
#include "SkBlitter.h"
#include "SkBlitRow.h"
#include "SkShader.h"
#include "SkSmallAllocator.h"

class SkRasterBlitter : public SkBlitter {
public:
    SkRasterBlitter(const SkBitmap& device) : fDevice(device) {}

protected:
    const SkBitmap& fDevice;

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
    SkShaderBlitter(const SkBitmap& device, const SkPaint& paint,
                    SkShader::Context* shaderContext);
    virtual ~SkShaderBlitter();

    /**
      *  Create a new shader context and uses it instead of the old one if successful.
      *  Will create the context at the same location as the old one (this is safe
      *  because the shader itself is unchanged).
      */
    virtual bool resetShaderContext(const SkShader::ContextRec&) SK_OVERRIDE;

    virtual SkShader::Context* getShaderContext() const SK_OVERRIDE { return fShaderContext; }

protected:
    uint32_t            fShaderFlags;
    const SkShader*     fShader;
    SkShader::Context*  fShaderContext;

private:
    // illegal
    SkShaderBlitter& operator=(const SkShaderBlitter&);

    typedef SkRasterBlitter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class SkA8_Coverage_Blitter : public SkRasterBlitter {
public:
    SkA8_Coverage_Blitter(const SkBitmap& device, const SkPaint& paint);
    virtual void blitH(int x, int y, int width) SK_OVERRIDE;
    virtual void blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[]) SK_OVERRIDE;
    virtual void blitV(int x, int y, int height, SkAlpha alpha) SK_OVERRIDE;
    virtual void blitRect(int x, int y, int width, int height) SK_OVERRIDE;
    virtual void blitMask(const SkMask&, const SkIRect&) SK_OVERRIDE;
    virtual const SkBitmap* justAnOpaqueColor(uint32_t*) SK_OVERRIDE;
};

class SkA8_Blitter : public SkRasterBlitter {
public:
    SkA8_Blitter(const SkBitmap& device, const SkPaint& paint);
    virtual void blitH(int x, int y, int width);
    virtual void blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[]);
    virtual void blitV(int x, int y, int height, SkAlpha alpha);
    virtual void blitRect(int x, int y, int width, int height);
    virtual void blitMask(const SkMask&, const SkIRect&);
    virtual const SkBitmap* justAnOpaqueColor(uint32_t*);

private:
    unsigned fSrcA;

    // illegal
    SkA8_Blitter& operator=(const SkA8_Blitter&);

    typedef SkRasterBlitter INHERITED;
};

class SkA8_Shader_Blitter : public SkShaderBlitter {
public:
    SkA8_Shader_Blitter(const SkBitmap& device, const SkPaint& paint,
                        SkShader::Context* shaderContext);
    virtual ~SkA8_Shader_Blitter();
    virtual void blitH(int x, int y, int width);
    virtual void blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[]);
    virtual void blitMask(const SkMask&, const SkIRect&);

private:
    SkXfermode* fXfermode;
    SkPMColor*  fBuffer;
    uint8_t*    fAAExpand;

    // illegal
    SkA8_Shader_Blitter& operator=(const SkA8_Shader_Blitter&);

    typedef SkShaderBlitter INHERITED;
};

////////////////////////////////////////////////////////////////

class SkARGB32_Blitter : public SkRasterBlitter {
public:
    SkARGB32_Blitter(const SkBitmap& device, const SkPaint& paint);
    virtual void blitH(int x, int y, int width);
    virtual void blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[]);
    virtual void blitV(int x, int y, int height, SkAlpha alpha);
    virtual void blitRect(int x, int y, int width, int height);
    virtual void blitMask(const SkMask&, const SkIRect&);
    virtual const SkBitmap* justAnOpaqueColor(uint32_t*);

protected:
    SkColor                fColor;
    SkPMColor              fPMColor;
    SkBlitRow::ColorProc   fColor32Proc;
    SkBlitRow::ColorRectProc fColorRect32Proc;

private:
    unsigned fSrcA, fSrcR, fSrcG, fSrcB;

    // illegal
    SkARGB32_Blitter& operator=(const SkARGB32_Blitter&);

    typedef SkRasterBlitter INHERITED;
};

class SkARGB32_Opaque_Blitter : public SkARGB32_Blitter {
public:
    SkARGB32_Opaque_Blitter(const SkBitmap& device, const SkPaint& paint)
        : INHERITED(device, paint) { SkASSERT(paint.getAlpha() == 0xFF); }
    virtual void blitMask(const SkMask&, const SkIRect&);

private:
    typedef SkARGB32_Blitter INHERITED;
};

class SkARGB32_Black_Blitter : public SkARGB32_Opaque_Blitter {
public:
    SkARGB32_Black_Blitter(const SkBitmap& device, const SkPaint& paint)
        : INHERITED(device, paint) {}
    virtual void blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[]);

private:
    typedef SkARGB32_Opaque_Blitter INHERITED;
};

class SkARGB32_Shader_Blitter : public SkShaderBlitter {
public:
    SkARGB32_Shader_Blitter(const SkBitmap& device, const SkPaint& paint,
                            SkShader::Context* shaderContext);
    virtual ~SkARGB32_Shader_Blitter();
    virtual void blitH(int x, int y, int width) SK_OVERRIDE;
    virtual void blitV(int x, int y, int height, SkAlpha alpha) SK_OVERRIDE;
    virtual void blitRect(int x, int y, int width, int height) SK_OVERRIDE;
    virtual void blitAntiH(int x, int y, const SkAlpha[], const int16_t[]) SK_OVERRIDE;
    virtual void blitMask(const SkMask&, const SkIRect&) SK_OVERRIDE;

private:
    SkXfermode*         fXfermode;
    SkPMColor*          fBuffer;
    SkBlitRow::Proc32   fProc32;
    SkBlitRow::Proc32   fProc32Blend;
    bool                fShadeDirectlyIntoDevice;
    bool                fConstInY;

    // illegal
    SkARGB32_Shader_Blitter& operator=(const SkARGB32_Shader_Blitter&);

    typedef SkShaderBlitter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

/*  These return the correct subclass of blitter for their device config.

    Currently, they make the following assumptions about the state of the
    paint:

    1. If there is an xfermode, there will also be a shader
    2. If there is a colorfilter, there will be a shader that itself handles
       calling the filter, so the blitter can always ignore the colorfilter obj

    These pre-conditions must be handled by the caller, in our case
    SkBlitter::Choose(...)
 */

SkBlitter* SkBlitter_ChooseD565(const SkBitmap& device, const SkPaint& paint,
                                SkShader::Context* shaderContext,
                                SkTBlitterAllocator* allocator);

#endif
