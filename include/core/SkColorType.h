/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorType_DEFINED
#define SkColorType_DEFINED

#include "include/core/SkTypes.h"

/** \enum SkColorType
    Describes how pixel bits encode color. A pixel may be an alpha mask, a grayscale, RGB, or ARGB.

    kN32_SkColorType selects the native 32-bit ARGB format for the current configuration. This can
    lead to inconsistent results across platforms, so use with caution.

    By default, Skia operates with the assumption of a little-Endian system. The names of each
    SkColorType implicitly define the channel ordering and size in memory. Due to historical reasons
    the names do not follow 100% identical convention, but are typically labeled from least
    significant to most significant. To help clarify when the actual data layout differs from the
    default convention, every SkColorType's comment includes a bit-labeled description of a pixel
    in that color type on a LE system.

    Unless specified otherwise, a channel's value is treated as an unsigned integer with a range of
    of [0, 2^N-1] and this is mapped uniformly to a floating point value of [0.0, 1.0]. Some color
    types instead store data directly in 32-bit floating point (assumed to be IEEE), or in 16-bit
    "half" floating point values. A half float, or F16/float16, is interpreted as FP 1-5-10 or
       Bits: [sign:15 exp:14..10 man:9..0]
*/
enum SkColorType : int {
    // Unknown or unrepresentable as an SkColorType.
    kUnknown_SkColorType,
    // Single channel data (8-bit) interpreted as an alpha value. RGB are 0.
    //   Bits: [A:7..0]
    kAlpha_8_SkColorType,
    // Three channel BGR data (5 bits red, 6 bits green, 5 bits blue) packed into a LE 16-bit word.
    // NOTE: The name of this enum value does not match the standard convention for SkColorType.
    //   Bits: [R:15..11 G:10..5 B:4..0]
    kRGB_565_SkColorType,
    // Four channel ABGR data (4 bits per channel) packed into a LE 16-bit word.
    // NOTE: The name of this enum value does not match the standard convention for SkColorType.
    //   Bits: [R:15..12 G:11..8 B:7..4 A:3..0]
    kARGB_4444_SkColorType,
    // Four channel RGBA data (8 bits per channel) packed into a LE 32-bit word.
    //   Bits: [A:31..24 B:23..16 G:15..8 R:7..0]
    kRGBA_8888_SkColorType,
    // Three channel RGB data (8 bits per channel) packed into a LE 32-bit word. The remaining bits
    // are ignored and alpha is forced to opaque.
    //   Bits: [x:31..24 B:23..16 G:15..8 R:7..0]
    kRGB_888x_SkColorType,
    // Four channel BGRA data (8 bits per channel) packed into a LE 32-bit word. R and B are swapped
    // relative to kRGBA_8888.
    //   Bits: [A:31..24 R:23..16 G:15..8 B:7..0]
    kBGRA_8888_SkColorType,
    // Four channel RGBA data (10 bits per color, 2 bits for alpha) packed into a LE 32-bit word.
    //   Bits: [A:31..30 B:29..20 G:19..10 R:9..0]
    kRGBA_1010102_SkColorType,
    // Four channel BGRA data (10 bits per color, 2 bits for alpha) packed into a LE 32-bit word.
    // R and B are swapped relative to kRGBA_1010102.
    //   Bits: [A:31..30 R:29..20 G:19..10 B:9..0]
    kBGRA_1010102_SkColorType,
    // Three channel RGB data (10 bits per channel) packed into a LE 32-bit word. The remaining bits
    // are ignored and alpha is forced to opaque.
    //   Bits: [x:31..30 B:29..20 G:19..10 R:9..0]
    kRGB_101010x_SkColorType,
    // Three channel BGR data (10 bits per channel) packed into a LE 32-bit word. The remaining bits
    // are ignored and alpha is forced to opaque. R and B are swapped relative to kRGB_101010x.
    //   Bits: [x:31..30 R:29..20 G:19..10 B:9..0]
    kBGR_101010x_SkColorType,
    // Three channel BGR data (10 bits per channel) packed into a LE 32-bit word. The remaining bits
    // are ignored and alpha is forced to opaque. Instead of normalizing [0, 1023] to [0.0, 1.0] the
    // color channels map to an extended range of [-0.752941, 1.25098], compatible with
    // MTLPixelFormatBGR10_XR.
    //   Bits: [x:31..30 R:29..20 G:19..10 B:9..0]
    kBGR_101010x_XR_SkColorType,
    // Four channel BGRA data (10 bits per channel) packed into a LE 64-bit word. Each channel is
    // preceded by 6 bits of padding.  Instead of normalizing [0, 1023] to [0.0, 1.0] the color and
    // alpha channels map to an extended range of [-0.752941, 1.25098], compatible with
    // MTLPixelFormatBGRA10_XR.
    //   Bits: [A:63..54 x:53..48 R:47..38 x:37..32 G:31..22 x:21..16 B:15..6 x:5..0]
    kBGRA_10101010_XR_SkColorType,
    // Four channel RGBA data (10 bits per channel) packed into a LE 64-bit word. Each channel is
    // preceded by 6 bits of padding.
    //   Bits: [A:63..54 x:53..48 B:47..38 x:37..32 G:31..22 x:21..16 R:15..6 x:5..0]
    kRGBA_10x6_SkColorType,
    // Single channel data (8-bit) interpreted as a grayscale value (e.g. replicated to RGB).
    //   Bits: [G:7..0]
    kGray_8_SkColorType,
    // Four channel RGBA data (16-bit half-float per channel) packed into a LE 64-bit word. Values
    // are assumed to be in [0.0,1.0] range, unlike kRGBA_F16.
    //   Bits: [A:63..48 B:47..32 G:31..16 R:15..0]
    kRGBA_F16Norm_SkColorType,
    // Four channel RGBA data (16-bit half-float per channel) packed into a LE 64-bit word.
    // This has extended range compared to kRGBA_F16Norm.
    //   Bits: [A:63..48 B:47..32 G:31..16 R:15..0]
    kRGBA_F16_SkColorType,
    // Three channel RGB data (16-bit half-float per channel) packed into a LE 64-bit word. The last
    // 16 bits are ignored and alpha is forced to opaque.
    //   Bits: [x:63..48 B:47..32 G:31..16 R:15..0]
    kRGB_F16F16F16x_SkColorType,
    // Four channel RGBA data (32-bit float per channel) packed into a LE 128-bit word.
    //   Bits: [A:127..96 B:95..64 G:63..32 R:31..0]
    kRGBA_F32_SkColorType,

    // The following 8 colortypes are just for reading from - not for rendering to

    // Two channel RG data (8 bits per channel). Blue is forced to 0, alpha is forced to opaque.
    //   Bits: [G:15..8 R:7..0]
    kR8G8_unorm_SkColorType,
    // Single channel data (16-bit half-float) interpreted as alpha. RGB are 0.
    //   Bits: [A:15..0]
    kA16_float_SkColorType,
    // Two channel RG data (16-bit half-float per channel) packed into a LE 32-bit word.
    // Blue is forced to 0, alpha is forced to opaque.
    //   Bits: [G:31..16 R:15..0]
    kR16G16_float_SkColorType,
    // Single channel data (16 bits) interpreted as alpha. RGB are 0.
    //   Bits: [A:15..0]
    kA16_unorm_SkColorType,
    // Two channel RG data (16 bits per channel) packed into a LE 32-bit word. B is forced to 0,
    // alpha is forced to opaque.
    //   Bits: [G:31..16 R:15..0]
    kR16G16_unorm_SkColorType,
    // Four channel RGBA data (16 bits per channel) packed into a LE 64-bit word.
    //   Bits: [A:63..48 B:47..32 G:31..16 R:15..0]
    kR16G16B16A16_unorm_SkColorType,
    // Four channel RGBA data (8 bits per channel) packed into a LE 32-bit word. The RGB values are
    // assumed to be encoded with the sRGB transfer function, which can be decoded automatically
    // by GPU hardware with certain texture formats.
    //   Bits: [A:31..24 B:23..16 G:15..8 R:7..0]
    kSRGBA_8888_SkColorType,
    // Single channel data (8 bits) interpreted as red. G and B are forced to 0, alpha is forced to
    // opaque.
    //    Bits: [R:7..0]
    kR8_unorm_SkColorType,

    kLastEnum_SkColorType     = kR8_unorm_SkColorType, //!< last valid value

#if SK_PMCOLOR_BYTE_ORDER(B,G,R,A)
    kN32_SkColorType          = kBGRA_8888_SkColorType,//!< native 32-bit BGRA encoding

#elif SK_PMCOLOR_BYTE_ORDER(R,G,B,A)
    kN32_SkColorType          = kRGBA_8888_SkColorType,//!< native 32-bit RGBA encoding

#else
    #error "SK_*32_SHIFT values must correspond to BGRA or RGBA byte order"
#endif
};
static constexpr int kSkColorTypeCnt = static_cast<int>(kLastEnum_SkColorType) + 1;

#endif
