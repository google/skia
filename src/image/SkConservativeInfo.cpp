/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpace_Base.h"
#include "SkConservativeInfo.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#endif

namespace SkConservativeInfo {

// Abstraction of GrCaps that handles the cases where we don't have a caps pointer (because
// we're in raster mode), or where GPU support is entirely missing. In theory, we only need the
// chosen format to be texturable, but that lets us choose F16 on GLES implemenations where we
// won't be able to read the texture back. We'd like to ensure that SkImake::makeNonTextureImage
// works, so we require that the formats we choose are renderable (as a proxy for being readable).
struct Caps {
    Caps(const GrCaps* caps) : fCaps(caps) {}

#if SK_SUPPORT_GPU
    bool supportsHalfFloat() const {
        return !fCaps ||
            (fCaps->isConfigTexturable(kRGBA_half_GrPixelConfig) &&
             fCaps->isConfigRenderable(kRGBA_half_GrPixelConfig, false));
    }

    bool supportsSRGB() const {
        return !fCaps ||
            (fCaps->srgbSupport() && fCaps->isConfigTexturable(kSRGBA_8888_GrPixelConfig));
    }

    bool supportsSBGR() const {
        return !fCaps || fCaps->srgbSupport();
    }
#else
    bool supportsHalfFloat() const { return true; }
    bool supportsSRGB() const { return true; }
    bool supportsSBGR() const { return true; }
#endif

    const GrCaps* fCaps;
};

Format ChooseFormat(const SkImageInfo& info, SkColorSpace* dstColorSpace, const GrCaps* grCaps) {
    SkColorSpace* cs = info.colorSpace();
    if (!cs || !dstColorSpace) {
        return kLegacy_Format;
    }

    Caps caps(grCaps);
    switch (info.colorType()) {
        case kUnknown_SkColorType:
        case kAlpha_8_SkColorType:
        case kRGB_565_SkColorType:
        case kARGB_4444_SkColorType:
            // We don't support color space on these formats, so always decode in legacy mode:
            // TODO: Ask the codec to decode these to something else (at least sRGB 8888)?
            return kLegacy_Format;

        case kIndex_8_SkColorType:
            // We can't draw from indexed textures with a color space, so ask the codec to expand
            if (cs->gammaCloseToSRGB()) {
                if (caps.supportsSRGB()) {
                    return kSRGB8888_Format;
                } else if (caps.supportsHalfFloat()) {
                    return kLinearF16_Format;
                } else {
                    return kLegacy_Format;
                }
            } else {
                if (caps.supportsHalfFloat()) {
                    return kLinearF16_Format;
                } else if (caps.supportsSRGB()) {
                    return kSRGB8888_Format;
                } else {
                    return kLegacy_Format;
                }
            }

        case kGray_8_SkColorType:
            // TODO: What do we do with grayscale sources that have strange color spaces attached?
            // The codecs and color space xform don't handle this correctly (yet), so drop it on
            // the floor. (Also, inflating by a factor of 8 is going to be unfortunate).
            // As it is, we don't directly support sRGB grayscale, so ask the codec to convert
            // it for us. This bypasses some really sketchy code GrUploadPixmapToTexture.
            if (cs->gammaCloseToSRGB() && caps.supportsSRGB()) {
                return kSRGB8888_Format;
            } else {
                return kLegacy_Format;
            }

        case kRGBA_8888_SkColorType:
            if (cs->gammaCloseToSRGB()) {
                if (caps.supportsSRGB()) {
                    return kAsIs_Format;
                } else if (caps.supportsHalfFloat()) {
                    return kLinearF16_Format;
                } else {
                    return kLegacy_Format;
                }
            } else {
                if (caps.supportsHalfFloat()) {
                    return kLinearF16_Format;
                } else if (caps.supportsSRGB()) {
                    return kSRGB8888_Format;
                } else {
                    return kLegacy_Format;
                }
            }

        case kBGRA_8888_SkColorType:
            // Odd case. sBGRA isn't a real thing, so we may not have this texturable.
            if (caps.supportsSBGR()) {
                if (cs->gammaCloseToSRGB()) {
                    return kAsIs_Format;
                } else if (caps.supportsHalfFloat()) {
                    return kLinearF16_Format;
                } else if (caps.supportsSRGB()) {
                    return kSRGB8888_Format;
                } else {
                    // sBGRA support without sRGBA is highly unlikely (impossible?) Nevertheless.
                    return kLegacy_Format;
                }
            } else {
                if (cs->gammaCloseToSRGB()) {
                    if (caps.supportsSRGB()) {
                        return kSRGB8888_Format;
                    } else if (caps.supportsHalfFloat()) {
                        return kLinearF16_Format;
                    } else {
                        return kLegacy_Format;
                    }
                } else {
                    if (caps.supportsHalfFloat()) {
                        return kLinearF16_Format;
                    } else if (caps.supportsSRGB()) {
                        return kSRGB8888_Format;
                    } else {
                        return kLegacy_Format;
                    }
                }
            }

        case kRGBA_F16_SkColorType:
            if (!caps.supportsHalfFloat()) {
                if (caps.supportsSRGB()) {
                    return kSRGB8888_Format;
                } else {
                    return kLegacy_Format;
                }
            } else if (cs->gammaIsLinear()) {
                return kAsIs_Format;
            } else {
                return kLinearF16_Format;
            }
    }
    SkDEBUGFAIL("Unreachable");
    return kLegacy_Format;
}

SkImageInfo Make(const SkImageInfo& info, Format format) {
    switch (format) {
        case kLegacy_Format:
            return info.makeColorSpace(nullptr);
        case kAsIs_Format:
            return info;
        case kLinearF16_Format:
            return info
                .makeColorType(kRGBA_F16_SkColorType)
                .makeColorSpace(as_CSB(info.colorSpace())->makeLinearGamma());
        case kSRGB8888_Format:
            return info
                .makeColorType(kRGBA_8888_SkColorType)
                .makeColorSpace(as_CSB(info.colorSpace())->makeSRGBGamma());
        default:
            SkDEBUGFAIL("Invalid cached format");
            return info;
    }
}

SkImageInfo Make(const SkImageInfo& info, SkColorSpace* dstColorSpace, const GrCaps* caps) {
    Format format = ChooseFormat(info, dstColorSpace, caps);
    return Make(info, format);
}

SkImageInfo Make(const SkImageInfo& info) {
    sk_sp<SkColorSpace> nonNull = SkColorSpace::MakeNamed(SkColorSpace::kSRGB_Named);
    return Make(info, nonNull.get(), nullptr);
}

};
