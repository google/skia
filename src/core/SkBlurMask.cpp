/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlurMask.h"

#include "SkColorPriv.h"
#include "SkEndian.h"
#include "SkMaskBlurFilter.h"
#include "SkMath.h"
#include "SkMathPriv.h"
#include "SkTemplates.h"
#include "SkTo.h"

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


template <typename AlphaIter>
static void merge_src_with_blur(uint8_t dst[], int dstRB,
                                AlphaIter src, int srcRB,
                                const uint8_t blur[], int blurRB,
                                int sw, int sh) {
    dstRB -= sw;
    blurRB -= sw;
    while (--sh >= 0) {
        AlphaIter rowSrc(src);
        for (int x = sw - 1; x >= 0; --x) {
            *dst = SkToU8(SkAlphaMul(*blur, SkAlpha255To256(*rowSrc)));
            ++dst;
            ++rowSrc;
            ++blur;
        }
        dst += dstRB;
        src >>= srcRB;
        blur += blurRB;
    }
}

template <typename AlphaIter>
static void clamp_solid_with_orig(uint8_t dst[], int dstRowBytes,
                                  AlphaIter src, int srcRowBytes,
                                  int sw, int sh) {
    int x;
    while (--sh >= 0) {
        AlphaIter rowSrc(src);
        for (x = sw - 1; x >= 0; --x) {
            int s = *rowSrc;
            int d = *dst;
            *dst = SkToU8(s + d - SkMulDiv255Round(s, d));
            ++dst;
            ++rowSrc;
        }
        dst += dstRowBytes - sw;
        src >>= srcRowBytes;
    }
}

template <typename AlphaIter>
static void clamp_outer_with_orig(uint8_t dst[], int dstRowBytes,
                                  AlphaIter src, int srcRowBytes,
                                  int sw, int sh) {
    int x;
    while (--sh >= 0) {
        AlphaIter rowSrc(src);
        for (x = sw - 1; x >= 0; --x) {
            int srcValue = *rowSrc;
            if (srcValue) {
                *dst = SkToU8(SkAlphaMul(*dst, SkAlpha255To256(255 - srcValue)));
            }
            ++dst;
            ++rowSrc;
        }
        dst += dstRowBytes - sw;
        src >>= srcRowBytes;
    }
}
///////////////////////////////////////////////////////////////////////////////

// we use a local function to wrap the class static method to work around
// a bug in gcc98
void SkMask_FreeImage(uint8_t* image);
void SkMask_FreeImage(uint8_t* image) {
    SkMask::FreeImage(image);
}

bool SkBlurMask::BoxBlur(SkMask* dst, const SkMask& src, SkScalar sigma, SkBlurStyle style,
                         SkIPoint* margin) {
    if (src.fFormat != SkMask::kBW_Format &&
        src.fFormat != SkMask::kA8_Format &&
        src.fFormat != SkMask::kARGB32_Format &&
        src.fFormat != SkMask::kLCD16_Format)
    {
        return false;
    }

    SkMaskBlurFilter blurFilter{sigma, sigma};
    if (blurFilter.hasNoBlur()) {
        // If there is no effective blur most styles will just produce the original mask.
        // However, kOuter_SkBlurStyle will produce an empty mask.
        if (style == kOuter_SkBlurStyle) {
            dst->fImage = nullptr;
            dst->fBounds = SkIRect::MakeEmpty();
            dst->fRowBytes = dst->fBounds.width();
            dst->fFormat = SkMask::kA8_Format;
            if (margin != nullptr) {
                // This filter will disregard the src.fImage completely.
                // The margin is actually {-(src.fBounds.width() / 2), -(src.fBounds.height() / 2)}
                // but it is not clear if callers will fall over with negative margins.
                *margin = SkIPoint{0,0};
            }
            return true;
        }
        return false;
    }
    const SkIPoint border = blurFilter.blur(src, dst);
    // If src.fImage is null, then this call is only to calculate the border.
    if (src.fImage != nullptr && dst->fImage == nullptr) {
        return false;
    }

    if (margin != nullptr) {
        *margin = border;
    }

    if (src.fImage == nullptr) {
        if (style == kInner_SkBlurStyle) {
            dst->fBounds = src.fBounds; // restore trimmed bounds
            dst->fRowBytes = dst->fBounds.width();
        }
        return true;
    }

    switch (style) {
        case kNormal_SkBlurStyle:
            break;
        case kSolid_SkBlurStyle: {
            auto dstStart = &dst->fImage[border.x() + border.y() * dst->fRowBytes];
            switch (src.fFormat) {
                case SkMask::kBW_Format:
                    clamp_solid_with_orig(
                            dstStart, dst->fRowBytes,
                            SkMask::AlphaIter<SkMask::kBW_Format>(src.fImage, 0), src.fRowBytes,
                            src.fBounds.width(), src.fBounds.height());
                    break;
                case SkMask::kA8_Format:
                    clamp_solid_with_orig(
                            dstStart, dst->fRowBytes,
                            SkMask::AlphaIter<SkMask::kA8_Format>(src.fImage), src.fRowBytes,
                            src.fBounds.width(), src.fBounds.height());
                    break;
                case SkMask::kARGB32_Format: {
                    uint32_t* srcARGB = reinterpret_cast<uint32_t*>(src.fImage);
                    clamp_solid_with_orig(
                            dstStart, dst->fRowBytes,
                            SkMask::AlphaIter<SkMask::kARGB32_Format>(srcARGB), src.fRowBytes,
                            src.fBounds.width(), src.fBounds.height());
                } break;
                case SkMask::kLCD16_Format: {
                    uint16_t* srcLCD = reinterpret_cast<uint16_t*>(src.fImage);
                    clamp_solid_with_orig(
                            dstStart, dst->fRowBytes,
                            SkMask::AlphaIter<SkMask::kLCD16_Format>(srcLCD), src.fRowBytes,
                            src.fBounds.width(), src.fBounds.height());
                } break;
                default:
                    SK_ABORT("Unhandled format.");
            }
        } break;
        case kOuter_SkBlurStyle: {
            auto dstStart = &dst->fImage[border.x() + border.y() * dst->fRowBytes];
            switch (src.fFormat) {
                case SkMask::kBW_Format:
                    clamp_outer_with_orig(
                            dstStart, dst->fRowBytes,
                            SkMask::AlphaIter<SkMask::kBW_Format>(src.fImage, 0), src.fRowBytes,
                            src.fBounds.width(), src.fBounds.height());
                    break;
                case SkMask::kA8_Format:
                    clamp_outer_with_orig(
                            dstStart, dst->fRowBytes,
                            SkMask::AlphaIter<SkMask::kA8_Format>(src.fImage), src.fRowBytes,
                            src.fBounds.width(), src.fBounds.height());
                    break;
                case SkMask::kARGB32_Format: {
                    uint32_t* srcARGB = reinterpret_cast<uint32_t*>(src.fImage);
                    clamp_outer_with_orig(
                            dstStart, dst->fRowBytes,
                            SkMask::AlphaIter<SkMask::kARGB32_Format>(srcARGB), src.fRowBytes,
                            src.fBounds.width(), src.fBounds.height());
                } break;
                case SkMask::kLCD16_Format: {
                    uint16_t* srcLCD = reinterpret_cast<uint16_t*>(src.fImage);
                    clamp_outer_with_orig(
                            dstStart, dst->fRowBytes,
                            SkMask::AlphaIter<SkMask::kLCD16_Format>(srcLCD), src.fRowBytes,
                            src.fBounds.width(), src.fBounds.height());
                } break;
                default:
                    SK_ABORT("Unhandled format.");
            }
        } break;
        case kInner_SkBlurStyle: {
            // now we allocate the "real" dst, mirror the size of src
            SkMask blur = *dst;
            SkAutoMaskFreeImage autoFreeBlurMask(blur.fImage);
            dst->fBounds = src.fBounds;
            dst->fRowBytes = dst->fBounds.width();
            size_t dstSize = dst->computeImageSize();
            if (0 == dstSize) {
                return false;   // too big to allocate, abort
            }
            dst->fImage = SkMask::AllocImage(dstSize);
            auto blurStart = &blur.fImage[border.x() + border.y() * blur.fRowBytes];
            switch (src.fFormat) {
                case SkMask::kBW_Format:
                    merge_src_with_blur(
                            dst->fImage, dst->fRowBytes,
                            SkMask::AlphaIter<SkMask::kBW_Format>(src.fImage, 0), src.fRowBytes,
                            blurStart, blur.fRowBytes,
                            src.fBounds.width(), src.fBounds.height());
                    break;
                case SkMask::kA8_Format:
                    merge_src_with_blur(
                            dst->fImage, dst->fRowBytes,
                            SkMask::AlphaIter<SkMask::kA8_Format>(src.fImage), src.fRowBytes,
                            blurStart, blur.fRowBytes,
                            src.fBounds.width(), src.fBounds.height());
                    break;
                case SkMask::kARGB32_Format: {
                    uint32_t* srcARGB = reinterpret_cast<uint32_t*>(src.fImage);
                    merge_src_with_blur(
                            dst->fImage, dst->fRowBytes,
                            SkMask::AlphaIter<SkMask::kARGB32_Format>(srcARGB), src.fRowBytes,
                            blurStart, blur.fRowBytes,
                            src.fBounds.width(), src.fBounds.height());
                } break;
                case SkMask::kLCD16_Format: {
                    uint16_t* srcLCD = reinterpret_cast<uint16_t*>(src.fImage);
                    merge_src_with_blur(
                            dst->fImage, dst->fRowBytes,
                            SkMask::AlphaIter<SkMask::kLCD16_Format>(srcLCD), src.fRowBytes,
                            blurStart, blur.fRowBytes,
                            src.fBounds.width(), src.fBounds.height());
                } break;
                default:
                    SK_ABORT("Unhandled format.");
            }
        } break;
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

/*  ComputeBlurProfile fills in an array of floating
    point values between 0 and 255 for the profile signature of
    a blurred half-plane with the given blur radius.  Since we're
    going to be doing screened multiplications (i.e., 1 - (1-x)(1-y))
    all the time, we actually fill in the profile pre-inverted
    (already done 255-x).
*/

void SkBlurMask::ComputeBlurProfile(uint8_t* profile, int size, SkScalar sigma) {
    SkASSERT(SkScalarCeilToInt(6*sigma) == size);

    int center = size >> 1;

    float invr = 1.f/(2*sigma);

    profile[0] = 255;
    for (int x = 1 ; x < size ; ++x) {
        float scaled_x = (center - x - .5f) * invr;
        float gi = gaussianIntegral(scaled_x);
        profile[x] = 255 - (uint8_t) (255.f * gi);
    }
}

// TODO MAYBE: Maintain a profile cache to avoid recomputing this for
// commonly used radii.  Consider baking some of the most common blur radii
// directly in as static data?

// Implementation adapted from Michael Herf's approach:
// http://stereopsis.com/shadowrect/

uint8_t SkBlurMask::ProfileLookup(const uint8_t *profile, int loc,
                                  int blurredWidth, int sharpWidth) {
    // how far are we from the original edge?
    int dx = SkAbs32(((loc << 1) + 1) - blurredWidth) - sharpWidth;
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
    int profileSize = SkScalarCeilToInt(6*sigma);
    if (profileSize <= 0) {
        return false;   // no blur to compute
    }

    int pad = profileSize/2;
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

    SkAutoTMalloc<uint8_t> profile(profileSize);

    ComputeBlurProfile(profile, profileSize, sigma);

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

    ComputeBlurredScanline(horizontalScanline, profile, dstWidth, sigma);
    ComputeBlurredScanline(verticalScanline, profile, dstHeight, sigma);

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

// The "simple" blur is a direct implementation of separable convolution with a discrete
// gaussian kernel.  It's "ground truth" in a sense; too slow to be used, but very
// useful for correctness comparisons.

bool SkBlurMask::BlurGroundTruth(SkScalar sigma, SkMask* dst, const SkMask& src,
                                 SkBlurStyle style, SkIPoint* margin) {

    if (src.fFormat != SkMask::kA8_Format) {
        return false;
    }

    float variance = sigma * sigma;

    int windowSize = SkScalarCeilToInt(sigma*6);
    // round window size up to nearest odd number
    windowSize |= 1;

    SkAutoTMalloc<float> gaussWindow(windowSize);

    int halfWindow = windowSize >> 1;

    gaussWindow[halfWindow] = 1;

    float windowSum = 1;
    for (int x = 1 ; x <= halfWindow ; ++x) {
        float gaussian = expf(-x*x / (2*variance));
        gaussWindow[halfWindow + x] = gaussWindow[halfWindow-x] = gaussian;
        windowSum += 2*gaussian;
    }

    // leave the filter un-normalized for now; we will divide by the normalization
    // sum later;

    int pad = halfWindow;
    if (margin) {
        margin->set( pad, pad );
    }

    dst->fBounds = src.fBounds;
    dst->fBounds.outset(pad, pad);

    dst->fRowBytes = dst->fBounds.width();
    dst->fFormat = SkMask::kA8_Format;
    dst->fImage = nullptr;

    if (src.fImage) {

        size_t dstSize = dst->computeImageSize();
        if (0 == dstSize) {
            return false;   // too big to allocate, abort
        }

        int             srcWidth = src.fBounds.width();
        int             srcHeight = src.fBounds.height();
        int             dstWidth = dst->fBounds.width();

        const uint8_t*  srcPixels = src.fImage;
        uint8_t*        dstPixels = SkMask::AllocImage(dstSize);
        SkAutoMaskFreeImage autoFreeDstPixels(dstPixels);

        // do the actual blur.  First, make a padded copy of the source.
        // use double pad so we never have to check if we're outside anything

        int padWidth = srcWidth + 4*pad;
        int padHeight = srcHeight;
        int padSize = padWidth * padHeight;

        SkAutoTMalloc<uint8_t> padPixels(padSize);
        memset(padPixels, 0, padSize);

        for (int y = 0 ; y < srcHeight; ++y) {
            uint8_t* padptr = padPixels + y * padWidth + 2*pad;
            const uint8_t* srcptr = srcPixels + y * srcWidth;
            memcpy(padptr, srcptr, srcWidth);
        }

        // blur in X, transposing the result into a temporary floating point buffer.
        // also double-pad the intermediate result so that the second blur doesn't
        // have to do extra conditionals.

        int tmpWidth = padHeight + 4*pad;
        int tmpHeight = padWidth - 2*pad;
        int tmpSize = tmpWidth * tmpHeight;

        SkAutoTMalloc<float> tmpImage(tmpSize);
        memset(tmpImage, 0, tmpSize*sizeof(tmpImage[0]));

        for (int y = 0 ; y < padHeight ; ++y) {
            uint8_t *srcScanline = padPixels + y*padWidth;
            for (int x = pad ; x < padWidth - pad ; ++x) {
                float *outPixel = tmpImage + (x-pad)*tmpWidth + y + 2*pad; // transposed output
                uint8_t *windowCenter = srcScanline + x;
                for (int i = -pad ; i <= pad ; ++i) {
                    *outPixel += gaussWindow[pad+i]*windowCenter[i];
                }
                *outPixel /= windowSum;
            }
        }

        // blur in Y; now filling in the actual desired destination.  We have to do
        // the transpose again; these transposes guarantee that we read memory in
        // linear order.

        for (int y = 0 ; y < tmpHeight ; ++y) {
            float *srcScanline = tmpImage + y*tmpWidth;
            for (int x = pad ; x < tmpWidth - pad ; ++x) {
                float *windowCenter = srcScanline + x;
                float finalValue = 0;
                for (int i = -pad ; i <= pad ; ++i) {
                    finalValue += gaussWindow[pad+i]*windowCenter[i];
                }
                finalValue /= windowSum;
                uint8_t *outPixel = dstPixels + (x-pad)*dstWidth + y; // transposed output
                int integerPixel = int(finalValue + 0.5f);
                *outPixel = SkClampMax( SkClampPos(integerPixel), 255 );
            }
        }

        dst->fImage = dstPixels;
        switch (style) {
            case kNormal_SkBlurStyle:
                break;
            case kSolid_SkBlurStyle: {
                clamp_solid_with_orig(
                        dstPixels + pad*dst->fRowBytes + pad, dst->fRowBytes,
                        SkMask::AlphaIter<SkMask::kA8_Format>(srcPixels), src.fRowBytes,
                        srcWidth, srcHeight);
            } break;
            case kOuter_SkBlurStyle: {
                clamp_outer_with_orig(
                        dstPixels + pad*dst->fRowBytes + pad, dst->fRowBytes,
                        SkMask::AlphaIter<SkMask::kA8_Format>(srcPixels), src.fRowBytes,
                        srcWidth, srcHeight);
            } break;
            case kInner_SkBlurStyle: {
                // now we allocate the "real" dst, mirror the size of src
                size_t srcSize = src.computeImageSize();
                if (0 == srcSize) {
                    return false;   // too big to allocate, abort
                }
                dst->fImage = SkMask::AllocImage(srcSize);
                merge_src_with_blur(dst->fImage, src.fRowBytes,
                    SkMask::AlphaIter<SkMask::kA8_Format>(srcPixels), src.fRowBytes,
                    dstPixels + pad*dst->fRowBytes + pad,
                    dst->fRowBytes, srcWidth, srcHeight);
                SkMask::FreeImage(dstPixels);
            } break;
        }
        autoFreeDstPixels.release();
    }

    if (style == kInner_SkBlurStyle) {
        dst->fBounds = src.fBounds; // restore trimmed bounds
        dst->fRowBytes = src.fRowBytes;
    }

    return true;
}
