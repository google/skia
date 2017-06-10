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
#include "SkRefCnt.h"

class GrFragmentProcessor;
class GrTexture;
class GrXPFactory;
class SkRasterPipeline;
class SkString;

class SkXfermode : public SkRefCnt {
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

protected:
    SkXfermode() {}
    virtual ~SkXfermode() {}

private:
};

#endif
