/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_TextureFormat_DEFINED
#define skgpu_graphite_TextureFormat_DEFINED

#include "include/core/SkTextureCompressionType.h"

#include <stddef.h>
#include <cstdint>

namespace skgpu::graphite {

/**
 * TextureFormat encapsulates all formats that Graphite backends support for color, depth, and
 * stencil textures. Not every TextureFormat is available on every backend or every device.
 *
 * TextureFormat names follow the convention of components being written in little Endian order.
 * The channel's or groups of identically typed channel's bit depth and type follow the channel
 * name. A numeral `n` represents an unsigned integer of bit depth `n` normalized to [0,1] while
 * `nF` represents a floating point number having `n` bits. If a component has a different bit
 * depth than the previous, an underscore separates them. Optional behavioral tags follow, such as
 * hardware sRGB decoding, extended range, or compression algorithms.
 *
 * TextureFormat is the union of texture formats from each backend that are of interest to Skia
 * and Graphite. As such, the non-floating point formats are restricted to just the unsigned
 * normalized varieties (vs. signed normalized or integral textures). The exception to this is
 * formats that include a stencil channel, which is always an unsigned integer. Floating point
 * formats are limited to 16-bit and 32-bit; there is not enough demand yet for 64-bit doubles yet.
 *
 * Not all backends support all texture format enum values, and not all backends will always support
 * a given format on the current device. Backends are responsible for mapping from TextureFormat to
 * the actual backend format value in Caps. A TextureFormat is considered supported when such a
 * mapping is defined.
 *
 * TextureFormat describes the data representation that the texture holds when copied to or from
 * a buffer. Regardless of the data channel ordering, the GPU will automatically swizzle reads and
 * writes to map to/from the in-shader RGBA order:
 *  - R formats sample in the shader as R001; and store only R of the out color.
 *  - A formats sample in the shader as 000A; and store only A of the out color.
 *  - RG formats sample in the shader as RG01; and store only RG of the out color.
 *  - RGB/BGR formats sample as RGB1; and store only RGB of the out color.
 *  - RGBA/BGRA/ARGB/ABGR all sample as RGBA and store RGBA.
 *
 * Unlike TextureFormat, SkColorType encapsulates both a data representation and semantics that
 * introduce a possible layer of indirection when using a TextureFormat to fulfill an SkColorType.
 * These operations may require modifying how the values are rendered or sampled, and/or performing
 * transformations on the CPU before/after a copy. Every SkColorType has a list of TextureFormats
 * that can be used with that type with minimal intervention. Formats are ordered from most to least
 * compatible by:
 *  1. Exact match: does not require any shader swizzling or CPU manipulation.
 *  2. Lossless w/ no semantic differences: does not require shader swizzling, but does require
 *     CPU manipulation (e.g. swap RG or force opaque).
 *  3. Lossless w/ semantic differences: requires shader swizzling, and may or may not require
 *     CPU manipulation for upload and readback.
 *  4. Lossy or data mismatch w/o any better fit. This is to support TextureFormats that aren't
 *     representable by any SkColorType on the CPU, but requires a valid SkColorType for the
 *     SkImageInfo of the textures' SkImage or SkSurface, e.g. compressed texture formats,
 *     multiplanar formats, or 32-bit float formats that can be used entirely within the GPU without
 *     needing to interpret it as a SkColorType on the CPU.
 *
 * The mapping between SkColorType and TextureFormat is defined statically. When a client creates a
 * new SkImage or SkSurface from an SkImageInfo, the TextureFormat is chosen as the first compatible
 * format for that SkColorType that has backend support on the device. When a client wraps a
 * BackendTexture, they also provide an SkColorType. The wrap fails if that color type's compatible
 * format list does not include the format of the provided BackendTexture.
 */
enum class TextureFormat : uint8_t {
    kUnsupported,
    // Name          //         VK_FORMAT          |   wgpu::TextureFormat   |    MTLPixelFormat
    // --------------//----------------------------|-------------------------|----------------------
    // 1 channel     //                            |                         |
    kR8,             // _R8_UNORM                  | ::R8Unorm               | R8Unorm
    kR16,            // _R16_UNORM                 | ::R16Unorm              | R16Unorm
    kR16F,           // _R16_SFLOAT                | ::R16Float              | R16Float
    kR32F,           // _R32_SFLOAT                | ::R32Float              | R32Float
    kA8,             //             -              |            -            | A8Unorm
    // 2 channel     //----------------------------|-------------------------|----------------------
    kRG8,            // _R8G8_UNORM                | ::RG8Unorm              | RG8Unorm
    kRG16,           // _R16G16_UNORM              | ::RG16Unorm             | RG16Unorm
    kRG16F,          // _R16G16_SFLOAT             | ::RG16Float             | RG16Float
    kRG32F,          // _R32G32_SFLOAT             | ::RG32Float             | RG32Float
    // 3 channel     //----------------------------|-------------------------|----------------------
    kRGB8,           // _R8G8B8_UNORM              |            -            |          -
    kBGR8,           // _B8G8R8_UNORM              |            -            |          -
    kB5_G6_R5,       // _R5G6B5_UNORM_PACK16       |            -            | B5G6R5Unorm
    kR5_G6_B5,       // _B5G6R5_UNORM_PACK16       |            -            |          -
    kRGB16,          // _R16G16B16_UNORM           |            -            |          -
    kRGB16F,         // _R16G16B16_SFLOAT          |            -            |          -
    kRGB32F,         // _R32G32B32_SFLOAT          |            -            |          -
    kRGB8_sRGB,      // _R8G8B8_SRGB               |            -            |          -
    kBGR10_XR,       //             -              |            -            | BGR10_XR
    // 4 channel     //----------------------------|-------------------------|----------------------
    kRGBA8,          // _R8G8B8A8_UNORM            | ::RGBA8Unorm            | RGBA8Unorm
    kRGBA16,         // _R16G16B16A16_UNORM        | ::RGBA16Unorm           | RGBA16Unorm
    kRGBA16F,        // _R16G16B16A16_SFLOAT       | ::RGBA16Float           | RGBA16Float
    kRGBA32F,        // _R32G32B32A32_SFLOAT       | ::RGBA32Float           | RGBA32Float
    kRGB10_A2,       // _A2B10G10R10_UNORM_PACK32  | ::RGB10A2Unorm          | RGB10A2Unorm
    kRGBA8_sRGB,     // _R8G8B8A8_SRGB             | ::RGBA8UnormSrgb        | RGBA8Unorm_sRGB
    kBGRA8,          // _B8G8R8A8_UNORM            | ::BGRA8Unorm            | BGRA8Unorm
    kBGR10_A2,       // _A2R10G10B10_UNORM_PACK32  |            -            | BGR10A2Unorm
    kBGRA8_sRGB,     // _B8G8R8A8_SRGB             | ::BGRA8UnormSrgb        | BGRA8Unorm_sRGB
    kABGR4,          // _R4G4B4A4_UNORM_PACK16     |            -            | ABGR4Unorm
    kARGB4,          // _B4G4R4A4_UNORM_PACK16     |            -            |          -
    kBGRA10x6_XR,    //             -              |            -            | BGRA10_XR
    // Compressed    //----------------------------|-------------------------|----------------------
    kRGB8_ETC2,      // _ETC2_R8G8B8_UNORM_BLOCK   | ::ETC2RGB8Unorm         | ETC2_RGB8
    kRGB8_ETC2_sRGB, // _ETC2_R8G8B8_SRGB_BLOCK    | ::ETC2RGB8UnormSrgb     | ETC2_RGB8_sRGB
    kRGB8_BC1,       // _BC1_RGB_UNORM_BLOCK       |            -            |          -
    kRGBA8_BC1,      // _BC1_RGBA_UNORM_BLOCK      | ::BC1RGBAUnorm          | BC1_RGBA
    kRGBA8_BC1_sRGB, // _BC1_RGBA_SRGB_BLOCK       | ::BC1RGBAUnormSrgb      | BC1_RGBA_sRGB
    // Multi-planar  //----------------------------|-------------------------|----------------------
    kYUV8_P2_420,    // _G8_B8R8_2PLANE_420_UNORM  | ::R8BG8Biplanar420Unorm |          -
    kYUV8_P3_420,    // _G8_B8_R8_3PLANE_420_UNORM |            -            |          -
    kYUV10x6_P2_420, // _G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16           |          -
                     //                            | ::R10X6BG10X6Biplanar420Unorm
    kExternal,       // VkExternalFormatANDROID    | ::External              |          -
    // Non-color     //----------------------------|-------------------------|----------------------
    kS8,             // _S8_UINT                   | ::Stencil8              | Stencil8
    kD16,            // _D16_UNORM                 | ::Depth16Unorm          | Depth16Unorm
    kD32F,           // _D32_SFLOAT                | ::Depth32Float          | Depth32Float
    kD24_S8,         // _D24_UNORM_S8_UINT         | ::Depth24PlusStencil8   | Depth24Unorm_Stencil8
    kD32F_S8,        // _D32_SFLOAT_S8_UINT        | ::Depth32FloatStencil8  | Depth32Float_Stencil8

    kLast = kD32F_S8
};
static constexpr int kTextureFormatCount = static_cast<int>(TextureFormat::kLast) + 1;

const char* TextureFormatName(TextureFormat);

SkTextureCompressionType TextureFormatCompressionType(TextureFormat);

size_t TextureFormatBytesPerBlock(TextureFormat);

// The value is mask of SkColorChannelFlag values.
uint32_t TextureFormatChannelMask(TextureFormat);

bool TextureFormatIsDepthOrStencil(TextureFormat);

bool TextureFormatHasDepth(TextureFormat);

bool TextureFormatHasStencil(TextureFormat);

bool TextureFormatIsMultiplanar(TextureFormat);

} // namespace skgpu::graphite

#endif // skgpu_graphite_TextureFormat_DEFINED
