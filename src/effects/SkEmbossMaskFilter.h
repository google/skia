/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkEmbossMaskFilter_DEFINED
#define SkEmbossMaskFilter_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskFilterBase.h"

#include <cstdint>
#include <utility>

class SkImageFilter;
class SkMatrix;
class SkPaint;
class SkReadBuffer;
class SkWriteBuffer;
struct SkIPoint;
struct SkPoint3;

/** \class SkEmbossMaskFilter

    This mask filter creates a 3D emboss look, by specifying a light and blur amount.
*/
class SkEmbossMaskFilter : public SkMaskFilterBase {
public:
    struct Light {
        SkScalar    fDirection[3];  // x,y,z
        uint16_t    fPad;
        uint8_t     fAmbient;
        uint8_t     fSpecular;      // exponent, 4.4 right now
    };

    static sk_sp<SkMaskFilter> Make(SkScalar blurSigma, const Light& light);

    // overrides from SkMaskFilter
    //  This method is not exported to java.
    SkMask::Format getFormat() const override;
    //  This method is not exported to java.
    bool filterMask(SkMaskBuilder* dst, const SkMask& src, const SkMatrix&,
                    SkIPoint* margin) const override;
    SkMaskFilterBase::Type type() const override { return SkMaskFilterBase::Type::kEmboss; }
    std::pair<sk_sp<SkImageFilter>, bool> asImageFilter(const SkMatrix& ctm,
                                                        const SkPaint& paint) const override;

protected:
    SkEmbossMaskFilter(SkScalar blurSigma, const Light& light);
    void flatten(SkWriteBuffer&) const override;

private:
    SK_FLATTENABLE_HOOKS(SkEmbossMaskFilter)

   /**
     *  Create a filter that calculates the specular illumination from a distant light source,
     *  interpreting the alpha channel of the input as the height profile of the surface (to
     *  approximate normal vectors). This is based on the legacy raster implementation of the
     *  emboss mask filter for clients that still use it.
     *  @param direction    The direction to the distance light.
     *  @param lightColor   The color of the specular light source.
     *  @param surfaceScale Scale factor to transform from alpha values to physical height.
     *  @param ks           Specular reflectance coefficient.
     *  @param shininess    The specular exponent determining how shiny the surface is.
     *  @param input        The input filter that defines surface normals (as alpha), or uses the
     *                      source bitmap when null.
     *  @param cropRect     Optional rectangle that crops the input and output.
     *
     * Defined in SkLightingImageFilter.cpp because it overlaps heavily with
     * SkImageFilters::DistantLitSpecular and that family of functions.
     */
    static sk_sp<SkImageFilter> LegacySpecular(const SkPoint3& direction, SkColor lightColor,
                                               SkScalar surfaceScale, SkScalar ks,
                                               SkScalar shininess, sk_sp<SkImageFilter> input);

    Light fLight;
    SkScalar    fBlurSigma;

    using INHERITED = SkMaskFilter;
};

#endif
