/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkXfermodePriv_DEFINED
#define SkXfermodePriv_DEFINED

#include "SkBlendMode.h"
#include "SkColor.h"
#include "SkFlattenable.h"

class GrFragmentProcessor;
class GrTexture;
class GrXPFactory;
class SkRasterPipeline;
class SkString;

struct SkArithmeticParams;

struct SkPM4f;
typedef SkPM4f (*SkXfermodeProc4f)(const SkPM4f& src, const SkPM4f& dst);

/** \class SkXfermode
 *
 *  SkXfermode is the base class for objects that are called to implement custom
 *  "transfer-modes" in the drawing pipeline. The static function Create(Modes)
 *  can be called to return an instance of any of the predefined subclasses as
 *  specified in the Modes enum. When an SkXfermode is assigned to an SkPaint,
 *  then objects drawn with that paint have the xfermode applied.
 *
 *  All subclasses are required to be reentrant-safe : it must be legal to share
 *  the same instance between several threads.
 */
class SK_API SkXfermode : public SkFlattenable {
public:
    virtual void xfer32(SkPMColor dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]) const = 0;

    /**
     *  If the xfermode is one of the modes in the Mode enum, then asMode()
     *  returns true and sets (if not null) mode accordingly. Otherwise it
     *  returns false and ignores the mode parameter.
     */
    virtual bool asMode(SkBlendMode* mode) const;

    /** Return an SkXfermode object for the specified mode.
     */
    static sk_sp<SkXfermode> Make(SkBlendMode);

    /**
     *  Skia maintains global xfermode objects corresponding to each BlendMode. This returns a
     *  ptr to that global xfermode (or null if the mode is srcover). Thus the caller may use
     *  the returned ptr, but it should leave its refcnt untouched.
     */
    static SkXfermode* Peek(SkBlendMode mode) {
        sk_sp<SkXfermode> xfer = Make(mode);
        if (!xfer) {
            SkASSERT(SkBlendMode::kSrcOver == mode);
            return nullptr;
        }
        SkASSERT(!xfer->unique());
        return xfer.get();
    }

    SkBlendMode blend() const {
        SkBlendMode mode;
        SkAssertResult(this->asMode(&mode));
        return mode;
    }

    static SkXfermodeProc GetProc(SkBlendMode);
    static SkXfermodeProc4f GetProc4f(SkBlendMode);

    enum SrcColorOpacity {
        // The src color is known to be opaque (alpha == 255)
        kOpaque_SrcColorOpacity = 0,
        // The src color is known to be fully transparent (color == 0)
        kTransparentBlack_SrcColorOpacity = 1,
        // The src alpha is known to be fully transparent (alpha == 0)
        kTransparentAlpha_SrcColorOpacity = 2,
        // The src color opacity is unknown
        kUnknown_SrcColorOpacity = 3
    };

    static bool IsOpaque(SkBlendMode, SrcColorOpacity);

#if SK_SUPPORT_GPU
    /** Used by the SkXfermodeImageFilter to blend two colors via a GrFragmentProcessor.
        The input to the returned FP is the src color. The dst color is
        provided by the dst param which becomes a child FP of the returned FP.
        It is legal for the function to return a null output. This indicates that
        the output of the blend is simply the src color.
     */
    virtual sk_sp<GrFragmentProcessor> makeFragmentProcessorForImageFilter(
                                                            sk_sp<GrFragmentProcessor> dst) const;

    /** A subclass must implement this factory function to work with the GPU backend.
        The xfermode will return a factory for which the caller will get a ref. It is up
        to the caller to install it. XferProcessors cannot use a background texture.
      */
    virtual const GrXPFactory* asXPFactory() const;
#endif

    SK_TO_STRING_PUREVIRT()
    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()
    SK_DEFINE_FLATTENABLE_TYPE(SkXfermode)

    enum D32Flags {
        kSrcIsOpaque_D32Flag  = 1 << 0,
        kSrcIsSingle_D32Flag  = 1 << 1,
        kDstIsSRGB_D32Flag    = 1 << 2,
    };
    typedef void (*D32Proc)(SkBlendMode, uint32_t dst[], const SkPM4f src[],
                            int count, const SkAlpha coverage[]);
    static D32Proc GetD32Proc(SkBlendMode, uint32_t flags);

    enum F16Flags {
        kSrcIsOpaque_F16Flag  = 1 << 0,
        kSrcIsSingle_F16Flag  = 1 << 1,
    };
    typedef void (*F16Proc)(SkBlendMode, uint64_t dst[], const SkPM4f src[], int count,
                            const SkAlpha coverage[]);
    static F16Proc GetF16Proc(SkBlendMode, uint32_t flags);

    enum LCDFlags {
        kSrcIsOpaque_LCDFlag    = 1 << 0,   // else src(s) may have alpha < 1
        kSrcIsSingle_LCDFlag    = 1 << 1,   // else src[count]
        kDstIsSRGB_LCDFlag      = 1 << 2,   // else l32 or f16
    };
    typedef void (*LCD32Proc)(uint32_t* dst, const SkPM4f* src, int count, const uint16_t lcd[]);
    typedef void (*LCDF16Proc)(uint64_t* dst, const SkPM4f* src, int count, const uint16_t lcd[]);
    static LCD32Proc GetLCD32Proc(uint32_t flags);
    static LCDF16Proc GetLCDF16Proc(uint32_t) { return nullptr; }

    virtual bool isArithmetic(SkArithmeticParams*) const { return false; }

protected:
    SkXfermode() {}

private:

    typedef SkFlattenable INHERITED;
};

#endif
