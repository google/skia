
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkXfermode_DEFINED
#define SkXfermode_DEFINED

#include "SkFlattenable.h"
#include "SkColor.h"

class GrContext;
class GrEffectRef;
class GrTexture;
class SkString;

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
    SK_DECLARE_INST_COUNT(SkXfermode)

    SkXfermode() {}

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

    /** If the xfermode can be expressed as an equation using the coefficients
        in Coeff, then asCoeff() returns true, and sets (if not null) src and
        dst accordingly.

            result = src_coeff * src_color + dst_coeff * dst_color;

        As examples, here are some of the porterduff coefficients

        MODE        SRC_COEFF       DST_COEFF
        clear       zero            zero
        src         one             zero
        dst         zero            one
        srcover     one             isa
        dstover     ida             one
     */
    virtual bool asCoeff(Coeff* src, Coeff* dst) const;

    /**
     *  The same as calling xfermode->asCoeff(..), except that this also checks
     *  if the xfermode is NULL, and if so, treats it as kSrcOver_Mode.
     */
    static bool AsCoeff(const SkXfermode*, Coeff* src, Coeff* dst);

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
        kSrcOver_Mode,  //!< [Sa + Da - Sa*Da, Rc = Sc + (1 - Sa)*Dc]
        kDstOver_Mode,  //!< [Sa + Da - Sa*Da, Rc = Dc + (1 - Da)*Sc]
        kSrcIn_Mode,    //!< [Sa * Da, Sc * Da]
        kDstIn_Mode,    //!< [Sa * Da, Sa * Dc]
        kSrcOut_Mode,   //!< [Sa * (1 - Da), Sc * (1 - Da)]
        kDstOut_Mode,   //!< [Da * (1 - Sa), Dc * (1 - Sa)]
        kSrcATop_Mode,  //!< [Da, Sc * Da + (1 - Sa) * Dc]
        kDstATop_Mode,  //!< [Sa, Sa * Dc + Sc * (1 - Da)]
        kXor_Mode,      //!< [Sa + Da - 2 * Sa * Da, Sc * (1 - Da) + (1 - Sa) * Dc]
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

    /** Return an SkXfermode object for the specified mode.
     */
    static SkXfermode* Create(Mode mode);

    /** Return a function pointer to a routine that applies the specified
        porter-duff transfer mode.
     */
    static SkXfermodeProc GetProc(Mode mode);

    /** Return a function pointer to a routine that applies the specified
        porter-duff transfer mode and srcColor to a 16bit device color. Note,
        if the mode+srcColor might return a non-opaque color, then there is not
        16bit proc, and this will return NULL.
      */
    static SkXfermodeProc16 GetProc16(Mode mode, SkColor srcColor);

    /**
     *  If the specified mode can be represented by a pair of Coeff, then return
     *  true and set (if not NULL) the corresponding coeffs. If the mode is
     *  not representable as a pair of Coeffs, return false and ignore the
     *  src and dst parameters.
     */
    static bool ModeAsCoeff(Mode mode, Coeff* src, Coeff* dst);

    // DEPRECATED: call AsMode(...)
    static bool IsMode(const SkXfermode* xfer, Mode* mode) {
        return AsMode(xfer, mode);
    }

    /** A subclass may implement this factory function to work with the GPU backend. It is legal
        to call this with all but the context param NULL to simply test the return value. effect,
        src, and dst must all be NULL or all non-NULL. If effect is non-NULL then the xfermode may
        optionally allocate an effect to return and the caller as *effect. The caller will install
        it and own a ref to it. Since the xfermode may or may not assign *effect, the caller should
        set *effect to NULL beforehand. If the function returns true and *effect is NULL then the
        src and dst coeffs will be applied to the draw. When *effect is non-NULL the coeffs are
        ignored. background specifies the texture to use as the background for compositing, and
        should be accessed in the effect's fragment shader. If NULL, the effect should request
        access to destination color (setWillReadDstColor()), and use that in the fragment shader
        (builder->dstColor()).
     */
    virtual bool asNewEffectOrCoeff(GrContext*,
                                    GrEffectRef** effect,
                                    Coeff* src,
                                    Coeff* dst,
                                    GrTexture* background = NULL) const;

    /**
     *  The same as calling xfermode->asNewEffect(...), except that this also checks if the xfermode
     *  is NULL, and if so, treats it as kSrcOver_Mode.
     */
    static bool AsNewEffectOrCoeff(SkXfermode*,
                                   GrContext*,
                                   GrEffectRef** effect,
                                   Coeff* src,
                                   Coeff* dst,
                                   GrTexture* background = NULL);

    SkDEVCODE(virtual void toString(SkString* str) const = 0;)
    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()
protected:
    SkXfermode(SkFlattenableReadBuffer& rb) : SkFlattenable(rb) {}

    /** The default implementation of xfer32/xfer16/xferA8 in turn call this
        method, 1 color at a time (upscaled to a SkPMColor). The default
        implmentation of this method just returns dst. If performance is
        important, your subclass should override xfer32/xfer16/xferA8 directly.

        This method will not be called directly by the client, so it need not
        be implemented if your subclass has overridden xfer32/xfer16/xferA8
    */
    virtual SkPMColor xferColor(SkPMColor src, SkPMColor dst) const;

private:
    enum {
        kModeCount = kLastMode + 1
    };

    friend class SkGraphics;
    static void Term();

    typedef SkFlattenable INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

/** \class SkProcXfermode

    SkProcXfermode is a xfermode that applies the specified proc to its colors.
    This class is not exported to java.
*/
class SkProcXfermode : public SkXfermode {
public:
    SkProcXfermode(SkXfermodeProc proc) : fProc(proc) {}

    // overrides from SkXfermode
    virtual void xfer32(SkPMColor dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]) const SK_OVERRIDE;
    virtual void xfer16(uint16_t dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]) const SK_OVERRIDE;
    virtual void xferA8(SkAlpha dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]) const SK_OVERRIDE;

    SK_DEVELOPER_TO_STRING()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkProcXfermode)

protected:
    SkProcXfermode(SkFlattenableReadBuffer&);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

    // allow subclasses to update this after we unflatten
    void setProc(SkXfermodeProc proc) {
        fProc = proc;
    }

    SkXfermodeProc getProc() const {
        return fProc;
    }

private:
    SkXfermodeProc  fProc;

    typedef SkXfermode INHERITED;
};

#endif
