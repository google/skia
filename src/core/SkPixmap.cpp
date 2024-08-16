/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPixmap.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkUnPreMultiply.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkTPin.h"
#include "src/base/SkHalf.h"
#include "src/base/SkVx.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkMask.h"
#include "src/core/SkReadPixelsRec.h"
#include "src/core/SkSwizzlePriv.h"
#include "src/opts/SkMemset_opts.h"

#include <cstdint>
#include <cstring>
#include <iterator>
#include <utility>

void SkPixmap::reset() {
    fPixels = nullptr;
    fRowBytes = 0;
    fInfo = SkImageInfo::MakeUnknown();
}

void SkPixmap::reset(const SkImageInfo& info, const void* addr, size_t rowBytes) {
    if (addr) {
        SkASSERT(info.validRowBytes(rowBytes));
    }
    fPixels = addr;
    fRowBytes = rowBytes;
    fInfo = info;
}

bool SkPixmap::reset(const SkMask& src) {
    if (SkMask::kA8_Format == src.fFormat) {
        this->reset(SkImageInfo::MakeA8(src.fBounds.width(), src.fBounds.height()),
                    src.fImage, src.fRowBytes);
        return true;
    }
    this->reset();
    return false;
}

void SkPixmap::setColorSpace(sk_sp<SkColorSpace> cs) {
    fInfo = fInfo.makeColorSpace(std::move(cs));
}

SkColorSpace* SkPixmap::colorSpace() const { return fInfo.colorSpace(); }

sk_sp<SkColorSpace> SkPixmap::refColorSpace() const { return fInfo.refColorSpace(); }

bool SkPixmap::extractSubset(SkPixmap* result, const SkIRect& subset) const {
    SkIRect srcRect, r;
    srcRect.setWH(this->width(), this->height());
    if (!r.intersect(srcRect, subset)) {
        return false;   // r is empty (i.e. no intersection)
    }

    // If the upper left of the rectangle was outside the bounds of this SkBitmap, we should have
    // exited above.
    SkASSERT(static_cast<unsigned>(r.fLeft) < static_cast<unsigned>(this->width()));
    SkASSERT(static_cast<unsigned>(r.fTop) < static_cast<unsigned>(this->height()));

    const void* pixels = nullptr;
    if (fPixels) {
        const size_t bpp = fInfo.bytesPerPixel();
        pixels = (const uint8_t*)fPixels + r.fTop * fRowBytes + r.fLeft * bpp;
    }
    result->reset(fInfo.makeDimensions(r.size()), pixels, fRowBytes);
    return true;
}

// This is the same as SkPixmap::addr(x,y), but this version gets inlined, while the public
// method does not. Perhaps we could bloat it so it can be inlined, but that would grow code-size
// everywhere, instead of just here (on behalf of getAlphaf()).
static const void* fast_getaddr(const SkPixmap& pm, int x, int y) {
    x <<= SkColorTypeShiftPerPixel(pm.colorType());
    return static_cast<const char*>(pm.addr()) + y * pm.rowBytes() + x;
}

float SkPixmap::getAlphaf(int x, int y) const {
    SkASSERT(this->addr());
    SkASSERT((unsigned)x < (unsigned)this->width());
    SkASSERT((unsigned)y < (unsigned)this->height());

    float value = 0;
    const void* srcPtr = fast_getaddr(*this, x, y);

    switch (this->colorType()) {
        case kUnknown_SkColorType:
            return 0;
        case kGray_8_SkColorType:
        case kR8G8_unorm_SkColorType:
        case kR16G16_unorm_SkColorType:
        case kR16G16_float_SkColorType:
        case kRGB_565_SkColorType:
        case kRGB_888x_SkColorType:
        case kRGB_101010x_SkColorType:
        case kBGR_101010x_SkColorType:
        case kBGR_101010x_XR_SkColorType:
        case kRGB_F16F16F16x_SkColorType:
        case kR8_unorm_SkColorType:
            return 1;
        case kAlpha_8_SkColorType:
            value = static_cast<const uint8_t*>(srcPtr)[0] * (1.0f/255);
            break;
        case kA16_unorm_SkColorType:
            value = static_cast<const uint16_t*>(srcPtr)[0] * (1.0f/65535);
            break;
        case kA16_float_SkColorType: {
            SkHalf half = static_cast<const SkHalf*>(srcPtr)[0];
            value = SkHalfToFloat(half);
            break;
        }
        case kARGB_4444_SkColorType: {
            uint16_t u16 = static_cast<const uint16_t*>(srcPtr)[0];
            value = SkGetPackedA4444(u16) * (1.0f/15);
            break;
        }
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
        case kSRGBA_8888_SkColorType:
            value = static_cast<const uint8_t*>(srcPtr)[3] * (1.0f/255);
            break;
        case kRGBA_1010102_SkColorType:
        case kBGRA_1010102_SkColorType: {
            uint32_t u32 = static_cast<const uint32_t*>(srcPtr)[0];
            value = (u32 >> 30) * (1.0f/3);
            break;
        }
        case kBGRA_10101010_XR_SkColorType: {
            uint64_t u64 = static_cast<const uint64_t*>(srcPtr)[0];
            value = ((u64 >> 54) - 384) / 510.f;
            break;
        }
        case kRGBA_10x6_SkColorType: {
            uint64_t u64 = static_cast<const uint64_t*>(srcPtr)[0];
            value = (u64 >> 54) * (1.0f/1023);
            break;
        }
        case kR16G16B16A16_unorm_SkColorType: {
            uint64_t u64 = static_cast<const uint64_t*>(srcPtr)[0];
            value = (u64 >> 48) * (1.0f/65535);
            break;
        }
        case kRGBA_F16Norm_SkColorType:
        case kRGBA_F16_SkColorType: {
            value = from_half(skvx::half4::Load(srcPtr))[3];
            break;
        }
        case kRGBA_F32_SkColorType:
            value = static_cast<const float*>(srcPtr)[3];
            break;
    }
    return value;
}

bool SkPixmap::readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                          int x, int y) const {
    if (!SkImageInfoValidConversion(dstInfo, fInfo)) {
        return false;
    }

    SkReadPixelsRec rec(dstInfo, dstPixels, dstRB, x, y);
    if (!rec.trim(fInfo.width(), fInfo.height())) {
        return false;
    }

    const void* srcPixels = this->addr(rec.fX, rec.fY);
    const SkImageInfo srcInfo = fInfo.makeDimensions(rec.fInfo.dimensions());
    return SkConvertPixels(rec.fInfo, rec.fPixels, rec.fRowBytes, srcInfo, srcPixels,
                           this->rowBytes());
}

SkColor SkPixmap::getColor(int x, int y) const {
    SkASSERT(this->addr());
    SkASSERT((unsigned)x < (unsigned)this->width());
    SkASSERT((unsigned)y < (unsigned)this->height());

    const bool needsUnpremul = (kPremul_SkAlphaType == fInfo.alphaType());
    auto toColor = [needsUnpremul](uint32_t maybePremulColor) {
        return needsUnpremul ? SkUnPreMultiply::PMColorToColor(maybePremulColor)
                             : SkSwizzle_BGRA_to_PMColor(maybePremulColor);
    };

    switch (this->colorType()) {
        case kGray_8_SkColorType: {
            uint8_t value = *this->addr8(x, y);
            return SkColorSetRGB(value, value, value);
        }
        case kR8_unorm_SkColorType: {
            uint8_t value = *this->addr8(x, y);
            return SkColorSetRGB(value, 0, 0);
        }
        case kAlpha_8_SkColorType: {
            return SkColorSetA(0, *this->addr8(x, y));
        }
        case kA16_unorm_SkColorType: {
            uint16_t value = *this->addr16(x, y);
            return SkColorSetA(0, value * (255 / 65535.0f));
        }
        case kA16_float_SkColorType: {
            SkHalf value = *this->addr16(x, y);
            return SkColorSetA(0, 255 * SkHalfToFloat(value));
        }
        case kRGB_565_SkColorType: {
            return SkPixel16ToColor(*this->addr16(x, y));
        }
        case kARGB_4444_SkColorType: {
            uint16_t value = *this->addr16(x, y);
            SkPMColor c = SkPixel4444ToPixel32(value);
            return toColor(c);
        }
        case kR8G8_unorm_SkColorType: {
            uint16_t value = *this->addr16(x, y);
            return (uint32_t)( ((value >>  0) & 0xff) ) << 16
                 | (uint32_t)( ((value >>  8) & 0xff) ) <<  8
                 | 0xff000000;
        }
        case kR16G16_unorm_SkColorType: {
            uint32_t value = *this->addr32(x, y);
            return (uint32_t)( ((value >>  0) & 0xffff) * (255/65535.0f) ) << 16
                 | (uint32_t)( ((value >> 16) & 0xffff) * (255/65535.0f) ) <<  8
                 | 0xff000000;
        }
        case kR16G16_float_SkColorType: {
            uint32_t value = *this->addr32(x, y);
            uint32_t r = 255 * SkHalfToFloat((value >>  0) & 0xffff);
            uint32_t g = 255 * SkHalfToFloat((value >> 16) & 0xffff);
            return (r << 16) | (g << 8) | 0xff000000;
        }
        case kRGB_888x_SkColorType: {
            uint32_t value = *this->addr32(x, y);
            return SkSwizzle_RB(value | 0xff000000);
        }
        case kBGRA_8888_SkColorType: {
            uint32_t value = *this->addr32(x, y);
            SkPMColor c = SkSwizzle_BGRA_to_PMColor(value);
            return toColor(c);
        }
        case kRGBA_8888_SkColorType: {
            uint32_t value = *this->addr32(x, y);
            SkPMColor c = SkSwizzle_RGBA_to_PMColor(value);
            return toColor(c);
        }
        case kSRGBA_8888_SkColorType: {
            auto srgb_to_linear = [](float x) {
                return (x <= 0.04045f) ? x * (1 / 12.92f)
                                       : std::pow(x * (1 / 1.055f) + (0.055f / 1.055f), 2.4f);
            };

            uint32_t value = *this->addr32(x, y);
            float r = ((value >>  0) & 0xff) * (1/255.0f),
                  g = ((value >>  8) & 0xff) * (1/255.0f),
                  b = ((value >> 16) & 0xff) * (1/255.0f),
                  a = ((value >> 24) & 0xff) * (1/255.0f);
            r = srgb_to_linear(r);
            g = srgb_to_linear(g);
            b = srgb_to_linear(b);
            if (a != 0 && needsUnpremul) {
                r = SkTPin(r/a, 0.0f, 1.0f);
                g = SkTPin(g/a, 0.0f, 1.0f);
                b = SkTPin(b/a, 0.0f, 1.0f);
            }
            return (uint32_t)( r * 255.0f ) << 16
                 | (uint32_t)( g * 255.0f ) <<  8
                 | (uint32_t)( b * 255.0f ) <<  0
                 | (uint32_t)( a * 255.0f ) << 24;
        }
        case kRGB_101010x_SkColorType: {
            uint32_t value = *this->addr32(x, y);
            // Convert 10-bit rgb to 8-bit bgr, and mask in 0xff alpha at the top.
            return (uint32_t)( ((value >>  0) & 0x3ff) * (255/1023.0f) ) << 16
                 | (uint32_t)( ((value >> 10) & 0x3ff) * (255/1023.0f) ) <<  8
                 | (uint32_t)( ((value >> 20) & 0x3ff) * (255/1023.0f) ) <<  0
                 | 0xff000000;
        }
        case kBGR_101010x_XR_SkColorType: {
            SkASSERT(false);
            return 0;
        }
        case kBGR_101010x_SkColorType: {
            uint32_t value = *this->addr32(x, y);
            // Convert 10-bit bgr to 8-bit bgr, and mask in 0xff alpha at the top.
            return (uint32_t)( ((value >>  0) & 0x3ff) * (255/1023.0f) ) <<  0
                 | (uint32_t)( ((value >> 10) & 0x3ff) * (255/1023.0f) ) <<  8
                 | (uint32_t)( ((value >> 20) & 0x3ff) * (255/1023.0f) ) << 16
                 | 0xff000000;
        }
        case kRGBA_1010102_SkColorType:
        case kBGRA_1010102_SkColorType: {
            uint32_t value = *this->addr32(x, y);

            float r = ((value >>  0) & 0x3ff) * (1/1023.0f),
                  g = ((value >> 10) & 0x3ff) * (1/1023.0f),
                  b = ((value >> 20) & 0x3ff) * (1/1023.0f),
                  a = ((value >> 30) & 0x3  ) * (1/   3.0f);
            if (this->colorType() == kBGRA_1010102_SkColorType) {
                std::swap(r,b);
            }
            if (a != 0 && needsUnpremul) {
                r = SkTPin(r/a, 0.0f, 1.0f);
                g = SkTPin(g/a, 0.0f, 1.0f);
                b = SkTPin(b/a, 0.0f, 1.0f);
            }
            return (uint32_t)( r * 255.0f ) << 16
                 | (uint32_t)( g * 255.0f ) <<  8
                 | (uint32_t)( b * 255.0f ) <<  0
                 | (uint32_t)( a * 255.0f ) << 24;
        }
        case kBGRA_10101010_XR_SkColorType: {
            SkASSERT(false);
            return 0;
        }
        case kRGBA_10x6_SkColorType: {
            uint64_t value = *this->addr64(x, y);
            float r = ((value >>  6) & 0x3ff) * (1/1023.0f),
                  g = ((value >> 22) & 0x3ff) * (1/1023.0f),
                  b = ((value >> 38) & 0x3ff) * (1/1023.0f),
                  a = ((value >> 54) & 0x3ff) * (1/1023.0f);
            return (uint32_t)( r * 255.0f ) << 16
                 | (uint32_t)( g * 255.0f ) <<  8
                 | (uint32_t)( b * 255.0f ) <<  0
                 | (uint32_t)( a * 255.0f ) << 24;
        }
        case kR16G16B16A16_unorm_SkColorType: {
            uint64_t value = *this->addr64(x, y);

            float r = ((value      ) & 0xffff) * (1/65535.0f),
                  g = ((value >> 16) & 0xffff) * (1/65535.0f),
                  b = ((value >> 32) & 0xffff) * (1/65535.0f),
                  a = ((value >> 48) & 0xffff) * (1/65535.0f);
            if (a != 0 && needsUnpremul) {
                r *= (1.0f/a);
                g *= (1.0f/a);
                b *= (1.0f/a);
            }
            return (uint32_t)( r * 255.0f ) << 16
                 | (uint32_t)( g * 255.0f ) <<  8
                 | (uint32_t)( b * 255.0f ) <<  0
                 | (uint32_t)( a * 255.0f ) << 24;
        }
        case kRGB_F16F16F16x_SkColorType: {
            const uint64_t* addr =
                (const uint64_t*)fPixels + y * (fRowBytes >> 3) + x;
            skvx::float4 p4 = from_half(skvx::half4::Load(addr));
            p4[3] = 1.0f;
            // p4 is RGBA, but we want BGRA, so we need to swap next
            return Sk4f_toL32(swizzle_rb(p4));
        }
        case kRGBA_F16Norm_SkColorType:
        case kRGBA_F16_SkColorType: {
            const uint64_t* addr =
                (const uint64_t*)fPixels + y * (fRowBytes >> 3) + x;
            skvx::float4 p4 = from_half(skvx::half4::Load(addr));
            if (p4[3] && needsUnpremul) {
                float inva = 1 / p4[3];
                p4 = p4 * skvx::float4(inva, inva, inva, 1);
            }
            // p4 is RGBA, but we want BGRA, so we need to swap next
            return Sk4f_toL32(swizzle_rb(p4));
        }
        case kRGBA_F32_SkColorType: {
            const float* rgba =
                (const float*)fPixels + 4*y*(fRowBytes >> 4) + 4*x;
            skvx::float4 p4 = skvx::float4::Load(rgba);
            // From here on, just like F16:
            if (p4[3] && needsUnpremul) {
                float inva = 1 / p4[3];
                p4 = p4 * skvx::float4(inva, inva, inva, 1);
            }
            // p4 is RGBA, but we want BGRA, so we need to swap next
            return Sk4f_toL32(swizzle_rb(p4));
        }
        case kUnknown_SkColorType:
            break;
    }
    SkDEBUGFAIL("");
    return SkColorSetARGB(0, 0, 0, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

SkColor4f SkPixmap::getColor4f(int x, int y) const {
    SkASSERT(this->addr());
    SkASSERT((unsigned)x < (unsigned)this->width());
    SkASSERT((unsigned)y < (unsigned)this->height());

    const bool needsUnpremul = (kPremul_SkAlphaType == fInfo.alphaType());
    auto toColor = [needsUnpremul](uint32_t maybePremulColor) {
        return needsUnpremul ? SkUnPreMultiply::PMColorToColor(maybePremulColor)
                             : SkSwizzle_BGRA_to_PMColor(maybePremulColor);
    };

    switch (this->colorType()) {
        case kGray_8_SkColorType: {
            float value = *this->addr8(x, y) / 255.0f;
            return SkColor4f{value, value, value, 1.0};
        }
        case kR8_unorm_SkColorType: {
            float value = *this->addr8(x, y) / 255.0f;
            return SkColor4f{value, 0.0f, 0.0f, 1.0f};
        }
        case kAlpha_8_SkColorType: {
            float value = *this->addr8(x, y) / 255.0f;
            return SkColor4f{0.0f, 0.0f, 0.0f, value};
        }
        case kA16_unorm_SkColorType: {
            float value = *this->addr16(x, y) / 65535.0f;
            return SkColor4f{0.0f, 0.0f, 0.0f, value};
        }
        case kA16_float_SkColorType: {
            SkHalf value = *this->addr16(x, y);
            return SkColor4f{0.0f, 0.0f, 0.0f, SkHalfToFloat(value)};
        }
        case kRGB_565_SkColorType: {
            SkColor c = SkPixel16ToColor(*this->addr16(x, y));
            return SkColor4f::FromColor(c);
        }
        case kARGB_4444_SkColorType: {
            uint16_t value = *this->addr16(x, y);
            SkPMColor c = SkPixel4444ToPixel32(value);
            return SkColor4f::FromColor(toColor(c));
        }
        case kR8G8_unorm_SkColorType: {
            uint16_t value = *this->addr16(x, y);
            SkColor c = (uint32_t)(((value >> 0) & 0xff)) << 16 |
                        (uint32_t)(((value >> 8) & 0xff)) << 8 | 0xff000000;
            return SkColor4f::FromColor(c);
        }
        case kR16G16_unorm_SkColorType: {
            uint32_t value = *this->addr32(x, y);
            SkColor c = (uint32_t)(((value >> 0) & 0xffff) * (255 / 65535.0f)) << 16 |
                        (uint32_t)(((value >> 16) & 0xffff) * (255 / 65535.0f)) << 8 | 0xff000000;
            return SkColor4f::FromColor(c);
        }
        case kR16G16_float_SkColorType: {
            uint32_t value = *this->addr32(x, y);
            float r = SkHalfToFloat((value >> 0) & 0xffff);
            float g = SkHalfToFloat((value >> 16) & 0xffff);
            return SkColor4f{r, g, 0.0, 1.0};
        }
        case kRGB_888x_SkColorType: {
            uint32_t value = *this->addr32(x, y);
            SkColor c = SkSwizzle_RB(value | 0xff000000);
            return SkColor4f::FromColor(c);
        }
        case kBGRA_8888_SkColorType: {
            uint32_t value = *this->addr32(x, y);
            SkPMColor c = SkSwizzle_BGRA_to_PMColor(value);
            return SkColor4f::FromColor(toColor(c));
        }
        case kRGBA_8888_SkColorType: {
            uint32_t value = *this->addr32(x, y);
            SkPMColor c = SkSwizzle_RGBA_to_PMColor(value);
            return SkColor4f::FromColor(toColor(c));
        }
        case kSRGBA_8888_SkColorType: {
            auto srgb_to_linear = [](float x) {
                return (x <= 0.04045f) ? x * (1 / 12.92f)
                                       : std::pow(x * (1 / 1.055f) + (0.055f / 1.055f), 2.4f);
            };

            uint32_t value = *this->addr32(x, y);
            float r = ((value >> 0) & 0xff) * (1 / 255.0f),
                  g = ((value >> 8) & 0xff) * (1 / 255.0f),
                  b = ((value >> 16) & 0xff) * (1 / 255.0f),
                  a = ((value >> 24) & 0xff) * (1 / 255.0f);
            r = srgb_to_linear(r);
            g = srgb_to_linear(g);
            b = srgb_to_linear(b);
            if (a != 0 && needsUnpremul) {
                r = SkTPin(r / a, 0.0f, 1.0f);
                g = SkTPin(g / a, 0.0f, 1.0f);
                b = SkTPin(b / a, 0.0f, 1.0f);
            }
            return SkColor4f{r, g, b, a};
        }
        case kBGR_101010x_XR_SkColorType: {
            SkASSERT(false);
            return {};
        }
        case kRGB_101010x_SkColorType: {
            uint32_t value = *this->addr32(x, y);
            // Convert 10-bit rgb to float rgb, and mask in 0xff alpha at the top.
            float r = (uint32_t)((value >> 0) & 0x3ff) / (1023.0f);
            float g = (uint32_t)((value >> 10) & 0x3ff) / (1023.0f);
            float b = (uint32_t)((value >> 20) & 0x3ff) / (1023.0f);
            float a = 1.0f;
            return SkColor4f{r, g, b, a};
        }
        case kBGR_101010x_SkColorType: {
            uint32_t value = *this->addr32(x, y);
            // Convert 10-bit bgr to float rgb, and mask in 0xff alpha at the top.
            float r = (uint32_t)((value >> 20) & 0x3ff) / (1023.0f);
            float g = (uint32_t)((value >> 10) & 0x3ff) / (1023.0f);
            float b = (uint32_t)((value >> 0) & 0x3ff) / (1023.0f);
            float a = 1.0f;
            return SkColor4f{r, g, b, a};
        }
        case kRGBA_1010102_SkColorType:
        case kBGRA_1010102_SkColorType: {
            uint32_t value = *this->addr32(x, y);

            float r = ((value >> 0) & 0x3ff) * (1 / 1023.0f),
                  g = ((value >> 10) & 0x3ff) * (1 / 1023.0f),
                  b = ((value >> 20) & 0x3ff) * (1 / 1023.0f),
                  a = ((value >> 30) & 0x3) * (1 / 3.0f);
            if (this->colorType() == kBGRA_1010102_SkColorType) {
                std::swap(r, b);
            }
            if (a != 0 && needsUnpremul) {
                r = SkTPin(r / a, 0.0f, 1.0f);
                g = SkTPin(g / a, 0.0f, 1.0f);
                b = SkTPin(b / a, 0.0f, 1.0f);
            }
            return SkColor4f{r, g, b, a};
        }
        case kBGRA_10101010_XR_SkColorType: {
            SkASSERT(false);
            return {};
        }
        case kRGBA_10x6_SkColorType: {
            uint64_t value = *this->addr64(x, y);

            float r = ((value >>  6) & 0x3ff) * (1/1023.0f),
                  g = ((value >> 22) & 0x3ff) * (1/1023.0f),
                  b = ((value >> 38) & 0x3ff) * (1/1023.0f),
                  a = ((value >> 54) & 0x3ff) * (1/1023.0f);
            return SkColor4f{r, g, b, a};
        }
        case kR16G16B16A16_unorm_SkColorType: {
            uint64_t value = *this->addr64(x, y);

            float r = ((value)&0xffff) * (1 / 65535.0f),
                  g = ((value >> 16) & 0xffff) * (1 / 65535.0f),
                  b = ((value >> 32) & 0xffff) * (1 / 65535.0f),
                  a = ((value >> 48) & 0xffff) * (1 / 65535.0f);
            if (a != 0 && needsUnpremul) {
                r *= (1.0f / a);
                g *= (1.0f / a);
                b *= (1.0f / a);
            }
            return SkColor4f{r, g, b, a};
        }
        case kRGBA_F16Norm_SkColorType:
        case kRGBA_F16_SkColorType: {
            const uint64_t* addr = (const uint64_t*)fPixels + y * (fRowBytes >> 3) + x;
            skvx::float4 p4 = from_half(skvx::half4::Load(addr));
            if (p4[3] && needsUnpremul) {
                float inva = 1 / p4[3];
                p4 = p4 * skvx::float4(inva, inva, inva, 1);
            }
            return SkColor4f{p4[0], p4[1], p4[2], p4[3]};
        }
        case kRGB_F16F16F16x_SkColorType: {
            const uint64_t* addr = (const uint64_t*)fPixels + y * (fRowBytes >> 3) + x;
            skvx::float4 p4 = from_half(skvx::half4::Load(addr));
            p4[3] = 1.0f;
            return SkColor4f{p4[0], p4[1], p4[2], p4[3]};
        }
        case kRGBA_F32_SkColorType: {
            const float* rgba = (const float*)fPixels + 4 * y * (fRowBytes >> 4) + 4 * x;
            skvx::float4 p4 = skvx::float4::Load(rgba);
            // From here on, just like F16:
            if (p4[3] && needsUnpremul) {
                float inva = 1 / p4[3];
                p4 = p4 * skvx::float4(inva, inva, inva, 1);
            }
            return SkColor4f{p4[0], p4[1], p4[2], p4[3]};
        }
        case kUnknown_SkColorType:
            break;
    }
    SkDEBUGFAIL("");
    return SkColors::kTransparent;
}

bool SkPixmap::computeIsOpaque() const {
    const int height = this->height();
    const int width = this->width();

    switch (this->colorType()) {
        case kAlpha_8_SkColorType: {
            unsigned a = 0xFF;
            for (int y = 0; y < height; ++y) {
                const uint8_t* row = this->addr8(0, y);
                for (int x = 0; x < width; ++x) {
                    a &= row[x];
                }
                if (0xFF != a) {
                    return false;
                }
            }
            return true;
        }
        case kA16_unorm_SkColorType: {
            unsigned a = 0xFFFF;
            for (int y = 0; y < height; ++y) {
                const uint16_t* row = this->addr16(0, y);
                for (int x = 0; x < width; ++x) {
                    a &= row[x];
                }
                if (0xFFFF != a) {
                    return false;
                }
            }
            return true;
        }
        case kA16_float_SkColorType: {
            for (int y = 0; y < height; ++y) {
                const SkHalf* row = this->addr16(0, y);
                for (int x = 0; x < width; ++x) {
                    if (row[x] < SK_Half1) {
                        return false;
                    }
                }
            }
            return true;
        }
        case kRGB_565_SkColorType:
        case kGray_8_SkColorType:
        case kR8G8_unorm_SkColorType:
        case kR16G16_unorm_SkColorType:
        case kR16G16_float_SkColorType:
        case kRGB_888x_SkColorType:
        case kRGB_101010x_SkColorType:
        case kBGR_101010x_SkColorType:
        case kRGB_F16F16F16x_SkColorType:
        case kBGR_101010x_XR_SkColorType:
        case kR8_unorm_SkColorType:
            return true;
        case kARGB_4444_SkColorType: {
            unsigned c = 0xFFFF;
            for (int y = 0; y < height; ++y) {
                const SkPMColor16* row = this->addr16(0, y);
                for (int x = 0; x < width; ++x) {
                    c &= row[x];
                }
                if (0xF != SkGetPackedA4444(c)) {
                    return false;
                }
            }
            return true;
        }
        case kBGRA_8888_SkColorType:
        case kRGBA_8888_SkColorType:
        case kSRGBA_8888_SkColorType: {
            SkPMColor c = (SkPMColor)~0;
            for (int y = 0; y < height; ++y) {
                const SkPMColor* row = this->addr32(0, y);
                for (int x = 0; x < width; ++x) {
                    c &= row[x];
                }
                if (0xFF != SkGetPackedA32(c)) {
                    return false;
                }
            }
            return true;
        }
        case kRGBA_F16Norm_SkColorType:
        case kRGBA_F16_SkColorType: {
            const SkHalf* row = (const SkHalf*)this->addr();
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    if (row[4 * x + 3] < SK_Half1) {
                        return false;
                    }
                }
                row += this->rowBytes() >> 1;
            }
            return true;
        }
        case kRGBA_F32_SkColorType: {
            const float* row = (const float*)this->addr();
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    if (row[4 * x + 3] < 1.0f) {
                        return false;
                    }
                }
                row += this->rowBytes() >> 2;
            }
            return true;
        }
        case kRGBA_1010102_SkColorType:
        case kBGRA_1010102_SkColorType: {
            uint32_t c = ~0;
            for (int y = 0; y < height; ++y) {
                const uint32_t* row = this->addr32(0, y);
                for (int x = 0; x < width; ++x) {
                    c &= row[x];
                }
                if (0b11 != c >> 30) {
                    return false;
                }
            }
            return true;
        }
        case kBGRA_10101010_XR_SkColorType:{
            static constexpr uint64_t kOne = 510 + 384;
            for (int y = 0; y < height; ++y) {
                const uint64_t* row = this->addr64(0, y);
                for (int x = 0; x < width; ++x) {
                    if ((row[x] >> 54) < kOne) {
                        return false;
                    }
                }
            }
            return true;
        }
        case kRGBA_10x6_SkColorType: {
            uint16_t acc = 0xFFC0;  // Ignore bottom six bits
            for (int y = 0; y < height; ++y) {
                const uint64_t* row = this->addr64(0, y);
                for (int x = 0; x < width; ++x) {
                    acc &= (row[x] >> 48);
                }
                if (0xFFC0 != acc) {
                    return false;
                }
            }
            return true;
        }
        case kR16G16B16A16_unorm_SkColorType: {
            uint16_t acc = 0xFFFF;
            for (int y = 0; y < height; ++y) {
                const uint64_t* row = this->addr64(0, y);
                for (int x = 0; x < width; ++x) {
                    acc &= (row[x] >> 48);
                }
                if (0xFFFF != acc) {
                    return false;
                }
            }
            return true;
        }
        case kUnknown_SkColorType:
            SkDEBUGFAIL("");
            break;
    }
    return false;
}

bool SkPixmap::erase(SkColor color, const SkIRect& subset) const {
    return this->erase(SkColor4f::FromColor(color), &subset);
}

bool SkPixmap::erase(const SkColor4f& color, const SkIRect* subset) const {
    if (this->colorType() == kUnknown_SkColorType) {
        return false;
    }

    SkIRect clip = this->bounds();
    if (subset && !clip.intersect(*subset)) {
        return false;   // is this check really needed (i.e. to return false in this case?)
    }

    // Erase is meant to simulate drawing in kSRC mode -- which means we have to convert out
    // unpremul input into premul (which we always do when we draw).
    const auto c = color.premul();

    const auto dst = SkImageInfo::Make(1, 1, this->colorType(), this->alphaType(),
                                       sk_ref_sp(this->colorSpace()));
    const auto src = SkImageInfo::Make(1, 1, kRGBA_F32_SkColorType, kPremul_SkAlphaType, nullptr);

    uint64_t dstPixel[2] = {};   // be large enough for our widest config (F32 x 4)
    SkASSERT((size_t)dst.bytesPerPixel() <= sizeof(dstPixel));

    if (!SkConvertPixels(dst, dstPixel, sizeof(dstPixel), src, &c, sizeof(c))) {
        return false;
    }

    if (this->colorType() == kRGBA_F32_SkColorType) {
        SkColor4f dstColor;
        memcpy(&dstColor, dstPixel, sizeof(dstColor));
        for (int y = clip.fTop; y < clip.fBottom; ++y) {
            SkColor4f* addr = (SkColor4f*)this->writable_addr(clip.fLeft, y);
            SK_OPTS_NS::memsetT(addr, dstColor, clip.width());
        }
    } else {
        using MemSet = void(*)(void*, uint64_t c, int count);
        const MemSet procs[] = {
            [](void* addr, uint64_t c, int count) {
                SkASSERT(c == (uint8_t)c);
                SK_OPTS_NS::memsetT((uint8_t*)addr, (uint8_t)c, count);
            },
            [](void* addr, uint64_t c, int count) {
                SkASSERT(c == (uint16_t)c);
                SK_OPTS_NS::memsetT((uint16_t*)addr, (uint16_t)c, count);
            },
            [](void* addr, uint64_t c, int count) {
                SkASSERT(c == (uint32_t)c);
                SK_OPTS_NS::memsetT((uint32_t*)addr, (uint32_t)c, count);
            },
            [](void* addr, uint64_t c, int count) {
                SK_OPTS_NS::memsetT((uint64_t*)addr, c, count);
            },
        };

        unsigned shift = SkColorTypeShiftPerPixel(this->colorType());
        SkASSERT(shift < std::size(procs));
        auto proc = procs[shift];

        for (int y = clip.fTop; y < clip.fBottom; ++y) {
            proc(this->writable_addr(clip.fLeft, y), dstPixel[0], clip.width());
        }
    }
    return true;
}
