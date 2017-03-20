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
                        const SkAlpha aa[]) const;
    virtual void xfer16(uint16_t dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]) const;
    virtual void xferA8(SkAlpha dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]) const;

    /** Enum of possible coefficients to describe some xfermodes
     */
    enum Coeff {
        kZero_Coeff,    /** 0 */
        kOne_Coeff,     /** 1 */
        kSC_Coeff,      /** src color */
        kISC_Coeff,     /** inverse src color (i.e. 1 - sc) */
        kDC_Coeff,      /** dst color */
        kIDC_Coeff,     /** inverse dst color (i.e. 1 - dc) */
        kSA_Coeff,      /** src alpha */
        kISA_Coeff,     /** inverse src alpha (i.e. 1 - sa) */
        kDA_Coeff,      /** dst alpha */
        kIDA_Coeff,     /** inverse dst alpha (i.e. 1 - da) */

        kCoeffCount
    };

    /** List of predefined xfermodes.
        The algebra for the modes uses the following symbols:
        Sa, Sc  - source alpha and color
        Da, Dc - destination alpha and color (before compositing)
        [a, c] - Resulting (alpha, color) values
        For these equations, the colors are in premultiplied state.
        If no xfermode is specified, kSrcOver is assumed.
        The modes are ordered by those that can be expressed as a pair of Coeffs, followed by those
        that aren't Coeffs but have separable r,g,b computations, and finally
        those that are not separable.
     */
    enum Mode {
        kClear_Mode,    //!< [0, 0]
        kSrc_Mode,      //!< [Sa, Sc]
        kDst_Mode,      //!< [Da, Dc]
        kSrcOver_Mode,  //!< [Sa + Da * (1 - Sa), Sc + Dc * (1 - Sa)]
        kDstOver_Mode,  //!< [Da + Sa * (1 - Da), Dc + Sc * (1 - Da)]
        kSrcIn_Mode,    //!< [Sa * Da, Sc * Da]
        kDstIn_Mode,    //!< [Da * Sa, Dc * Sa]
        kSrcOut_Mode,   //!< [Sa * (1 - Da), Sc * (1 - Da)]
        kDstOut_Mode,   //!< [Da * (1 - Sa), Dc * (1 - Sa)]
        kSrcATop_Mode,  //!< [Da, Sc * Da + Dc * (1 - Sa)]
        kDstATop_Mode,  //!< [Sa, Dc * Sa + Sc * (1 - Da)]
        kXor_Mode,      //!< [Sa + Da - 2 * Sa * Da, Sc * (1 - Da) + Dc * (1 - Sa)]
        kPlus_Mode,     //!< [Sa + Da, Sc + Dc]
        kModulate_Mode, // multiplies all components (= alpha and color)

        // Following blend modes are defined in the CSS Compositing standard:
        // https://dvcs.w3.org/hg/FXTF/rawfile/tip/compositing/index.html#blending
        kScreen_Mode,
        kLastCoeffMode = kScreen_Mode,

        kOverlay_Mode,
        kDarken_Mode,
        kLighten_Mode,
        kColorDodge_Mode,
        kColorBurn_Mode,
        kHardLight_Mode,
        kSoftLight_Mode,
        kDifference_Mode,
        kExclusion_Mode,
        kMultiply_Mode,
        kLastSeparableMode = kMultiply_Mode,

        kHue_Mode,
        kSaturation_Mode,
        kColor_Mode,
        kLuminosity_Mode,
        kLastMode = kLuminosity_Mode
    };

    /**
     * Gets the name of the Mode as a string.
     */
    static const char* ModeName(Mode);
    static const char* ModeName(SkBlendMode mode) {
        return ModeName(Mode(mode));
    }

    /**
     *  If the xfermode is one of the modes in the Mode enum, then asMode()
     *  returns true and sets (if not null) mode accordingly. Otherwise it
     *  returns false and ignores the mode parameter.
     */
    virtual bool asMode(Mode* mode) const;

    /**
     *  The same as calling xfermode->asMode(mode), except that this also checks
     *  if the xfermode is NULL, and if so, treats it as kSrcOver_Mode.
     */
    static bool AsMode(const SkXfermode*, Mode* mode);
    static bool AsMode(const sk_sp<SkXfermode>& xfer, Mode* mode) {
        return AsMode(xfer.get(), mode);
    }

    /**
     *  Returns true if the xfermode claims to be the specified Mode. This works
     *  correctly even if the xfermode is NULL (which equates to kSrcOver.) Thus
     *  you can say this without checking for a null...
     *
     *  If (SkXfermode::IsMode(paint.getXfermode(),
     *                         SkXfermode::kDstOver_Mode)) {
     *      ...
     *  }
     */
    static bool IsMode(const SkXfermode* xfer, Mode mode);
    static bool IsMode(const sk_sp<SkXfermode>& xfer, Mode mode) {
        return IsMode(xfer.get(), mode);
    }

    /** Return an SkXfermode object for the specified mode.
     */
    static sk_sp<SkXfermode> Make(SkBlendMode);
    static sk_sp<SkXfermode> Make(Mode m) { return Make((SkBlendMode)m); }

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
        Mode mode;
        SkAssertResult(this->asMode(&mode));
        return (SkBlendMode)mode;
    }

    static SkXfermodeProc GetProc(SkBlendMode);
    static SkXfermodeProc4f GetProc4f(SkBlendMode);

    /**
     *  If the specified mode can be represented by a pair of Coeff, then return
     *  true and set (if not NULL) the corresponding coeffs. If the mode is
     *  not representable as a pair of Coeffs, return false and ignore the
     *  src and dst parameters.
     */
    static bool ModeAsCoeff(Mode mode, Coeff* src, Coeff* dst);
    static bool ModeAsCoeff(SkBlendMode mode, Coeff* src, Coeff* dst) {
        return ModeAsCoeff((Mode)mode, src, dst);
    }

    /**
     * Returns whether or not the xfer mode can support treating coverage as alpha
     */
    virtual bool supportsCoverageAsAlpha() const;

    /**
     *  The same as calling xfermode->supportsCoverageAsAlpha(), except that this also checks if
     *  the xfermode is NULL, and if so, treats it as kSrcOver_Mode.
     */
    static bool SupportsCoverageAsAlpha(const SkXfermode* xfer);
    static bool SupportsCoverageAsAlpha(const sk_sp<SkXfermode>& xfer) {
        return SupportsCoverageAsAlpha(xfer.get());
    }

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

    /**
     * Returns whether or not the result of the draw with the xfer mode will be opaque or not. The
     * input to this call is an enum describing known information about the opacity of the src color
     * that will be given to the xfer mode.
     */
    virtual bool isOpaque(SrcColorOpacity opacityType) const;

    /**
     *  The same as calling xfermode->isOpaque(...), except that this also checks if
     *  the xfermode is NULL, and if so, treats it as kSrcOver_Mode.
     */
    static bool IsOpaque(const SkXfermode* xfer, SrcColorOpacity opacityType);
    static bool IsOpaque(const sk_sp<SkXfermode>& xfer, SrcColorOpacity opacityType) {
        return IsOpaque(xfer.get(), opacityType);
    }
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
    /** The default implementation of xfer32/xfer16/xferA8 in turn call this
        method, 1 color at a time (upscaled to a SkPMColor). The default
        implementation of this method just returns dst. If performance is
        important, your subclass should override xfer32/xfer16/xferA8 directly.

        This method will not be called directly by the client, so it need not
        be implemented if your subclass has overridden xfer32/xfer16/xferA8
    */
    virtual SkPMColor xferColor(SkPMColor src, SkPMColor dst) const;

private:
    enum {
        kModeCount = kLastMode + 1
    };

    typedef SkFlattenable INHERITED;
};

#endif
