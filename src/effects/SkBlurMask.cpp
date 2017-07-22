/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlurMask.h"
#include "SkMaskBlurFilter.h"
#include "SkMath.h"
#include "SkTemplates.h"
#include "SkEndian.h"

// This constant approximates the scaling done in the software path's
// "high quality" mode, in SkBlurMask::Blur() (1 / sqrt(3)).
// IMHO, it actually should be 1:  we blur "less" than we should do
// according to the CSS and canvas specs, simply because Safari does the same.
// Firefox used to do the same too, until 4.0 where they fixed it.  So at some
// point we should probably get rid of these scaling constants and rebaseline
// all the blur tests.
static const SkScalar kBLUR_SIGMA_SCALE = 0.57735f;

SkScalar SkBlurMask::ConvertRadiusToSigma(SkScalar radius) {
    return radius > 0 ? kBLUR_SIGMA_SCALE * radius + 0.5f : 0.0f;
}

SkScalar SkBlurMask::ConvertSigmaToRadius(SkScalar sigma) {
    return sigma > 0.5f ? (sigma - 0.5f) / kBLUR_SIGMA_SCALE : 0.0f;
}

#include "SkColorPriv.h"

static void merge_src_with_blur(uint8_t dst[], int dstRB,
                                const uint8_t src[], int srcRB,
                                const uint8_t blur[], int blurRB,
                                int sw, int sh) {
    dstRB -= sw;
    srcRB -= sw;
    blurRB -= sw;
    while (--sh >= 0) {
        for (int x = sw - 1; x >= 0; --x) {
            *dst = SkToU8(SkAlphaMul(*blur, SkAlpha255To256(*src)));
            dst += 1;
            src += 1;
            blur += 1;
        }
        dst += dstRB;
        src += srcRB;
        blur += blurRB;
    }
}

static void clamp_with_orig(uint8_t dst[], int dstRowBytes,
                            const uint8_t src[], int srcRowBytes,
                            int sw, int sh,
                            SkBlurStyle style) {
    int x;
    while (--sh >= 0) {
        switch (style) {
        case kSolid_SkBlurStyle:
            for (x = sw - 1; x >= 0; --x) {
                int s = *src;
                int d = *dst;
                *dst = SkToU8(s + d - SkMulDiv255Round(s, d));
                dst += 1;
                src += 1;
            }
            break;
        case kOuter_SkBlurStyle:
            for (x = sw - 1; x >= 0; --x) {
                if (*src) {
                    *dst = SkToU8(SkAlphaMul(*dst, SkAlpha255To256(255 - *src)));
                }
                dst += 1;
                src += 1;
            }
            break;
        default:
            SkDEBUGFAIL("Unexpected blur style here");
            break;
        }
        dst += dstRowBytes - sw;
        src += srcRowBytes - sw;
    }
}

///////////////////////////////////////////////////////////////////////////////

// we use a local function to wrap the class static method to work around
// a bug in gcc98
void SkMask_FreeImage(uint8_t* image);
void SkMask_FreeImage(uint8_t* image) {
    SkMask::FreeImage(image);
}

bool SkBlurMask::BoxBlur(SkMask* dst, const SkMask& src,
                         SkScalar sigma, SkBlurStyle style, SkBlurQuality quality,
                         SkIPoint* margin, bool force_quality) {

    if (src.fFormat != SkMask::kA8_Format) {
        return false;
    }

    SkMaskBlurFilter blurFilter{sigma, sigma};
    if (blurFilter.hasNoBlur()) {
        return false;
    }
    SkIPoint border = blurFilter.blur(src, dst);

    if (src.fImage != nullptr) {
        // if need be, alloc the "real" dst (same size as src) and copy/merge
        // the blur into it (applying the src)
        if (style == kInner_SkBlurStyle) {
            // now we allocate the "real" dst, mirror the size of src
            size_t srcSize = src.computeImageSize();
            if (0 == srcSize) {
                return false;   // too big to allocate, abort
            }
            auto blur = dst->fImage;
            dst->fImage = SkMask::AllocImage(srcSize);
            auto blurStart = &blur[border.x() + border.y() * dst->fRowBytes];
            merge_src_with_blur(dst->fImage, src.fRowBytes,
                                src.fImage, src.fRowBytes,
                                blurStart,
                                dst->fRowBytes,
                                src.fBounds.width(), src.fBounds.height());
            SkMask::FreeImage(blur);
        } else if (style != kNormal_SkBlurStyle) {
            auto dstStart = &dst->fImage[border.x() + border.y() * dst->fRowBytes];
            clamp_with_orig(dstStart,
                            dst->fRowBytes, src.fImage, src.fRowBytes,
                            src.fBounds.width(), src.fBounds.height(), style);
        }
    }

    if (style == kInner_SkBlurStyle) {
        dst->fBounds = src.fBounds; // restore trimmed bounds
        dst->fRowBytes = src.fRowBytes;
    }

    if (margin != nullptr) {
        *margin = border;
    }

    return true;
}

/* Convolving a box with itself three times results in a piecewise
   quadratic function:

   0                              x <= -1.5
   9/8 + 3/2 x + 1/2 x^2   -1.5 < x <= -.5
   3/4 - x^2                -.5 < x <= .5
   9/8 - 3/2 x + 1/2 x^2    0.5 < x <= 1.5
   0                        1.5 < x

   Mathematica:

   g[x_] := Piecewise [ {
     {9/8 + 3/2 x + 1/2 x^2 ,  -1.5 < x <= -.5},
     {3/4 - x^2             ,   -.5 < x <= .5},
     {9/8 - 3/2 x + 1/2 x^2 ,   0.5 < x <= 1.5}
   }, 0]

   To get the profile curve of the blurred step function at the rectangle
   edge, we evaluate the indefinite integral, which is piecewise cubic:

   0                                        x <= -1.5
   9/16 + 9/8 x + 3/4 x^2 + 1/6 x^3   -1.5 < x <= -0.5
   1/2 + 3/4 x - 1/3 x^3              -.5 < x <= .5
   7/16 + 9/8 x - 3/4 x^2 + 1/6 x^3     .5 < x <= 1.5
   1                                  1.5 < x

   in Mathematica code:

   gi[x_] := Piecewise[ {
     { 0 , x <= -1.5 },
     { 9/16 + 9/8 x + 3/4 x^2 + 1/6 x^3, -1.5 < x <= -0.5 },
     { 1/2 + 3/4 x - 1/3 x^3          ,  -.5 < x <= .5},
     { 7/16 + 9/8 x - 3/4 x^2 + 1/6 x^3,   .5 < x <= 1.5}
   },1]
*/

static float gaussianIntegral(float x) {
    if (x > 1.5f) {
        return 0.0f;
    }
    if (x < -1.5f) {
        return 1.0f;
    }

    float x2 = x*x;
    float x3 = x2*x;

    if ( x > 0.5f ) {
        return 0.5625f - (x3 / 6.0f - 3.0f * x2 * 0.25f + 1.125f * x);
    }
    if ( x > -0.5f ) {
        return 0.5f - (0.75f * x - x3 / 3.0f);
    }
    return 0.4375f + (-x3 / 6.0f - 3.0f * x2 * 0.25f - 1.125f * x);
}

/*  ComputeBlurProfile allocates and fills in an array of floating
    point values between 0 and 255 for the profile signature of
    a blurred half-plane with the given blur radius.  Since we're
    going to be doing screened multiplications (i.e., 1 - (1-x)(1-y))
    all the time, we actually fill in the profile pre-inverted
    (already done 255-x).

    It's the responsibility of the caller to delete the
    memory returned in profile_out.
*/

uint8_t* SkBlurMask::ComputeBlurProfile(SkScalar sigma) {
    int size = SkScalarCeilToInt(6*sigma);

    int center = size >> 1;
    uint8_t* profile = new uint8_t[size];

    float invr = 1.f/(2*sigma);

    profile[0] = 255;
    for (int x = 1 ; x < size ; ++x) {
        float scaled_x = (center - x - .5f) * invr;
        float gi = gaussianIntegral(scaled_x);
        profile[x] = 255 - (uint8_t) (255.f * gi);
    }

    return profile;
}

// TODO MAYBE: Maintain a profile cache to avoid recomputing this for
// commonly used radii.  Consider baking some of the most common blur radii
// directly in as static data?

// Implementation adapted from Michael Herf's approach:
// http://stereopsis.com/shadowrect/

uint8_t SkBlurMask::ProfileLookup(const uint8_t *profile, int loc, int blurred_width, int sharp_width) {
    int dx = SkAbs32(((loc << 1) + 1) - blurred_width) - sharp_width; // how far are we from the original edge?
    int ox = dx >> 1;
    if (ox < 0) {
        ox = 0;
    }

    return profile[ox];
}

void SkBlurMask::ComputeBlurredScanline(uint8_t *pixels, const uint8_t *profile,
                                        unsigned int width, SkScalar sigma) {

    unsigned int profile_size = SkScalarCeilToInt(6*sigma);
    SkAutoTMalloc<uint8_t> horizontalScanline(width);

    unsigned int sw = width - profile_size;
    // nearest odd number less than the profile size represents the center
    // of the (2x scaled) profile
    int center = ( profile_size & ~1 ) - 1;

    int w = sw - center;

    for (unsigned int x = 0 ; x < width ; ++x) {
       if (profile_size <= sw) {
           pixels[x] = ProfileLookup(profile, x, width, w);
       } else {
           float span = float(sw)/(2*sigma);
           float giX = 1.5f - (x+.5f)/(2*sigma);
           pixels[x] = (uint8_t) (255 * (gaussianIntegral(giX) - gaussianIntegral(giX + span)));
       }
    }
}

bool SkBlurMask::BlurRect(SkScalar sigma, SkMask *dst,
                          const SkRect &src, SkBlurStyle style,
                          SkIPoint *margin, SkMask::CreateMode createMode) {
    int profile_size = SkScalarCeilToInt(6*sigma);

    int pad = profile_size/2;
    if (margin) {
        margin->set( pad, pad );
    }

    dst->fBounds.set(SkScalarRoundToInt(src.fLeft - pad),
                     SkScalarRoundToInt(src.fTop - pad),
                     SkScalarRoundToInt(src.fRight + pad),
                     SkScalarRoundToInt(src.fBottom + pad));

    dst->fRowBytes = dst->fBounds.width();
    dst->fFormat = SkMask::kA8_Format;
    dst->fImage = nullptr;

    int             sw = SkScalarFloorToInt(src.width());
    int             sh = SkScalarFloorToInt(src.height());

    if (createMode == SkMask::kJustComputeBounds_CreateMode) {
        if (style == kInner_SkBlurStyle) {
            dst->fBounds.set(SkScalarRoundToInt(src.fLeft),
                             SkScalarRoundToInt(src.fTop),
                             SkScalarRoundToInt(src.fRight),
                             SkScalarRoundToInt(src.fBottom)); // restore trimmed bounds
            dst->fRowBytes = sw;
        }
        return true;
    }

    std::unique_ptr<uint8_t[]> profile(ComputeBlurProfile(sigma));

    size_t dstSize = dst->computeImageSize();
    if (0 == dstSize) {
        return false;   // too big to allocate, abort
    }

    uint8_t*        dp = SkMask::AllocImage(dstSize);

    dst->fImage = dp;

    int dstHeight = dst->fBounds.height();
    int dstWidth = dst->fBounds.width();

    uint8_t *outptr = dp;

    SkAutoTMalloc<uint8_t> horizontalScanline(dstWidth);
    SkAutoTMalloc<uint8_t> verticalScanline(dstHeight);

    ComputeBlurredScanline(horizontalScanline, profile.get(), dstWidth, sigma);
    ComputeBlurredScanline(verticalScanline, profile.get(), dstHeight, sigma);

    for (int y = 0 ; y < dstHeight ; ++y) {
        for (int x = 0 ; x < dstWidth ; x++) {
            unsigned int maskval = SkMulDiv255Round(horizontalScanline[x], verticalScanline[y]);
            *(outptr++) = maskval;
        }
    }

    if (style == kInner_SkBlurStyle) {
        // now we allocate the "real" dst, mirror the size of src
        size_t srcSize = (size_t)(src.width() * src.height());
        if (0 == srcSize) {
            return false;   // too big to allocate, abort
        }
        dst->fImage = SkMask::AllocImage(srcSize);
        for (int y = 0 ; y < sh ; y++) {
            uint8_t *blur_scanline = dp + (y+pad)*dstWidth + pad;
            uint8_t *inner_scanline = dst->fImage + y*sw;
            memcpy(inner_scanline, blur_scanline, sw);
        }
        SkMask::FreeImage(dp);

        dst->fBounds.set(SkScalarRoundToInt(src.fLeft),
                         SkScalarRoundToInt(src.fTop),
                         SkScalarRoundToInt(src.fRight),
                         SkScalarRoundToInt(src.fBottom)); // restore trimmed bounds
        dst->fRowBytes = sw;

    } else if (style == kOuter_SkBlurStyle) {
        for (int y = pad ; y < dstHeight-pad ; y++) {
            uint8_t *dst_scanline = dp + y*dstWidth + pad;
            memset(dst_scanline, 0, sw);
        }
    } else if (style == kSolid_SkBlurStyle) {
        for (int y = pad ; y < dstHeight-pad ; y++) {
            uint8_t *dst_scanline = dp + y*dstWidth + pad;
            memset(dst_scanline, 0xff, sw);
        }
    }
    // normal and solid styles are the same for analytic rect blurs, so don't
    // need to handle solid specially.

    return true;
}

bool SkBlurMask::BlurRRect(SkScalar sigma, SkMask *dst,
                           const SkRRect &src, SkBlurStyle style,
                           SkIPoint *margin, SkMask::CreateMode createMode) {
    // Temporary for now -- always fail, should cause caller to fall back
    // to old path.  Plumbing just to land API and parallelize effort.

    return false;
}
