/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "include/gpu/ShaderErrorHandler.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/mtl/MtlBackendContext.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/TextureFormat.h"
#include "src/gpu/graphite/mtl/MtlGraphiteUtils.h"
#include "src/gpu/graphite/mtl/MtlQueueManager.h"
#include "src/gpu/graphite/mtl/MtlSharedContext.h"
#include "src/gpu/mtl/MtlUtilsPriv.h"

#import <Metal/Metal.h>

namespace skgpu::graphite {

namespace ContextFactory {

std::unique_ptr<Context> MakeMetal(const MtlBackendContext& backendContext,
                                   const ContextOptions& options) {
    sk_sp<SharedContext> sharedContext = MtlSharedContext::Make(backendContext, options);
    if (!sharedContext) {
        return nullptr;
    }

    sk_cfp<id<MTLCommandQueue>> queue =
            sk_ret_cfp((id<MTLCommandQueue>)(backendContext.fQueue.get()));
    auto queueManager = std::make_unique<MtlQueueManager>(std::move(queue), sharedContext.get());
    if (!queueManager) {
        return nullptr;
    }

    return ContextCtorAccessor::MakeContext(std::move(sharedContext),
                                            std::move(queueManager),
                                            options);
}

} // namespace ContextFactory

sk_cfp<id<MTLLibrary>> MtlCompileShaderLibrary(const MtlSharedContext* sharedContext,
                                               std::string_view label,
                                               std::string_view msl,
                                               ShaderErrorHandler* errorHandler) {
    TRACE_EVENT0("skia.shaders", "driver_compile_shader");
    NSString* nsSource = [[NSString alloc] initWithBytesNoCopy:const_cast<char*>(msl.data())
                                                        length:msl.size()
                                                      encoding:NSUTF8StringEncoding
                                                  freeWhenDone:NO];
    if (!nsSource) {
        return nil;
    }
    MTLCompileOptions* options = [[MTLCompileOptions alloc] init];

    // Framebuffer fetch is supported in MSL 2.3 in MacOS 11+.
    if (@available(macOS 11.0, iOS 14.0, tvOS 14.0, *)) {
        options.languageVersion = MTLLanguageVersion2_3;
    } else {
        // Supported by iOS 13, Graphite's minimum iOS version
        options.languageVersion = MTLLanguageVersion2_2;
    }

    NSError* error = nil;
    // TODO: do we need a version with a timeout?
    sk_cfp<id<MTLLibrary>> compiledLibrary(
            [sharedContext->device() newLibraryWithSource:(NSString* _Nonnull)nsSource
                                                  options:options
                                                    error:&error]);
    if (!compiledLibrary) {
        std::string mslStr(msl);
        errorHandler->compileError(
                mslStr.c_str(), error.debugDescription.UTF8String, /*shaderWasCached=*/false);
        return nil;
    }

    NSString* nsLabel = [[NSString alloc] initWithBytesNoCopy:const_cast<char*>(label.data())
                                                       length:label.size()
                                                     encoding:NSUTF8StringEncoding
                                                 freeWhenDone:NO];
    compiledLibrary.get().label = nsLabel;
    return compiledLibrary;
}

namespace {

// Most formats in MTL_FORMAT_MAPPING for TextureFormat are always available for Graphite's
// supported Mac and iOS versions. These formats are not. To simplify the mapping, these constants
// are extracted from Frameworks/Metal/MTLPixelFormat.h to avoid guarding with an @available check,
// although MtlCaps must still do so when enabling a particular format.

#define MTL_PIXEL_FORMAT(name, value) \
    static constexpr MTLPixelFormat name = static_cast<MTLPixelFormat>(value)

// Either unsupported on iOS or high min iOS, so considered "mac-only"
MTL_PIXEL_FORMAT(MTLPixelFormatBC1_RGBA_, 130);
MTL_PIXEL_FORMAT(MTLPixelFormatBC1_RGBA_sRGB_, 131);
MTL_PIXEL_FORMAT(MTLPixelFormatDepth24Unorm_Stencil8_, 255);
MTL_PIXEL_FORMAT(MTLPixelFormatX24_Stencil8_, 262);

#if defined(SK_BUILD_FOR_MAC)
static_assert(MTLPixelFormatBC1_RGBA_ == MTLPixelFormatBC1_RGBA);
static_assert(MTLPixelFormatBC1_RGBA_sRGB_ == MTLPixelFormatBC1_RGBA_sRGB);
static_assert(MTLPixelFormatDepth24Unorm_Stencil8_ == MTLPixelFormatDepth24Unorm_Stencil8);
static_assert(MTLPixelFormatX24_Stencil8_ == MTLPixelFormatX24_Stencil8);
#endif

struct FamilyFeatureMap {
    using FormatFeatureSet = std::pair<MTLPixelFormat, SkEnumBitMask<MTLFeatureFlag>>;
    using BulkFeature = std::pair</*condition*/SkEnumBitMask<MTLFeatureFlag>,
                                  /*newFeatureFlags*/SkEnumBitMask<MTLFeatureFlag>>;

    FamilyFeatureMap(std::initializer_list<FormatFeatureSet> features)
            : FamilyFeatureMap(/*parent=*/nullptr, features, /*bulkExtension=*/{{}, {}}) {}

    FamilyFeatureMap(const FamilyFeatureMap* parent,
                     std::initializer_list<FormatFeatureSet> extensions,
                     BulkFeature bulkExtension) {
        for (int i = 0; i < kTextureFormatCount; ++i) {
            fFormatFeatures[i] = parent ? parent->fFormatFeatures[i] : MTLFeatureFlag::NotAvailable;
            // Apply bulk extension based on parent features, not any specific new extensions
            if ((fFormatFeatures[i] & bulkExtension.first) == bulkExtension.first) {
                fFormatFeatures[i] |= bulkExtension.second;
            }
        }
        for (const FormatFeatureSet& set : extensions) {
            TextureFormat tf = MTLPixelFormatToTextureFormat(set.first);
            if (tf == TextureFormat::kUnsupported) { continue; } // Skip if Graphite doesn't use it

            fFormatFeatures[(int) tf] |= set.second;
        }
        SkASSERT(!SkToBool(fFormatFeatures[(int) TextureFormat::kUnsupported]));
    }

    // We only care about MTLPixelFormats that map back to a TextureFormat.
    std::array<SkEnumBitMask<MTLFeatureFlag>, kTextureFormatCount> fFormatFeatures;
};

FamilyFeatureMap extend(const FamilyFeatureMap& parent,
                        std::initializer_list<FamilyFeatureMap::FormatFeatureSet> extensions,
                        FamilyFeatureMap::BulkFeature bulkExtension = {{}, {}}) {
    return FamilyFeatureMap(&parent, extensions, bulkExtension);
}

using enum MTLFeatureFlag; // Keeps things legible in the tables below!

// To read this table with the PDF, every MTLPixelFormat in the first column is included here in
// the same order. The FeatureFlag bitmasks should match the "Apple2" column exactly. Subsequent
// Apple families and their columns are separate `FamilyFeatureMap` variables that extend from
// kApple2. Only the format rows that differ from the prior family are written, and only the new
// features are reported (e.g. the diff between columns). An exception to this is when the new
// family is upgraded to "All", in which case writing "All" better matches the table than attempting
// to work out which specific feature became supported.
static const FamilyFeatureMap kApple2 = {
    // Ordinary 8-bit pixel foramts
    {MTLPixelFormatA8Unorm,                Filter},
    {MTLPixelFormatR8Unorm,                All},
    {MTLPixelFormatR8Unorm_sRGB,           All},
    {MTLPixelFormatR8Snorm,                All},
    {MTLPixelFormatR8Uint,                 Write | Color | MSAA},
    {MTLPixelFormatR8Sint,                 Write | Color | MSAA},
    // Ordinary 16-bit pixel formats
    {MTLPixelFormatR16Unorm,               Filter | Write | Color | MSAA | Blend},
    {MTLPixelFormatR16Snorm,               Filter | Write | Color | MSAA | Blend},
    {MTLPixelFormatR16Uint,                Write | Color | MSAA},
    {MTLPixelFormatR16Sint,                Write | Color | MSAA},
    {MTLPixelFormatR16Float,               All},
    {MTLPixelFormatRG8Unorm,               All},
    {MTLPixelFormatRG8Unorm_sRGB,          All},
    {MTLPixelFormatRG8Snorm,               All},
    {MTLPixelFormatRG8Uint,                Write | Color | MSAA},
    {MTLPixelFormatRG8Sint,                Write | Color | MSAA},
    // Packed 16-bit pixel formats
    {MTLPixelFormatB5G6R5Unorm,            Filter | Color | MSAA | Resolve | Blend},
    {MTLPixelFormatA1BGR5Unorm,            Filter | Color | MSAA | Resolve | Blend},
    {MTLPixelFormatABGR4Unorm,             Filter | Color | MSAA | Resolve | Blend},
    {MTLPixelFormatBGR5A1Unorm,            Filter | Color | MSAA | Resolve | Blend},
    // Ordinary 32-bit pixel formats
    {MTLPixelFormatR32Uint,                Write | Color},
    {MTLPixelFormatR32Sint,                Write | Color},
    {MTLPixelFormatR32Float,               Write | Color | MSAA | Blend},
    {MTLPixelFormatRG16Unorm,              Filter | Write | Color | MSAA | Blend},
    {MTLPixelFormatRG16Snorm,              Filter | Write | Color | MSAA | Blend},
    {MTLPixelFormatRG16Uint,               Write | Color | MSAA},
    {MTLPixelFormatRG16Sint,               Write | Color | MSAA},
    {MTLPixelFormatRG16Float,              All},
    {MTLPixelFormatRGBA8Unorm,             All},
    {MTLPixelFormatRGBA8Unorm_sRGB,        All},
    {MTLPixelFormatRGBA8Snorm,             All},
    {MTLPixelFormatRGBA8Uint,              Write | Color | MSAA},
    {MTLPixelFormatRGBA8Sint,              Write | Color | MSAA},
    {MTLPixelFormatBGRA8Unorm,             All},
    {MTLPixelFormatBGRA8Unorm_sRGB,        All},
    // Packed 32-bit pixel formats
    {MTLPixelFormatRGB10A2Unorm,           Filter | Color | MSAA | Resolve | Blend},
    {MTLPixelFormatBGR10A2Unorm,           All},
    {MTLPixelFormatRGB10A2Uint,            Color | MSAA},
    {MTLPixelFormatRG11B10Float,           Filter | Color | MSAA | Resolve | Blend},
    {MTLPixelFormatRGB9E5Float,            Filter | Color | MSAA | Resolve | Blend},
    // Ordinary 64-bit pixel formats
    {MTLPixelFormatRG32Uint,               Write | Color},
    {MTLPixelFormatRG32Sint,               Write | Color},
    {MTLPixelFormatRG32Float,              Write | Color | Blend},
    {MTLPixelFormatRGBA16Unorm,            Filter | Write | Color | MSAA | Blend},
    {MTLPixelFormatRGBA16Snorm,            Filter | Write | Color | MSAA | Blend},
    {MTLPixelFormatRGBA16Uint,             Write | Color | MSAA},
    {MTLPixelFormatRGBA16Sint,             Write | Color | MSAA},
    {MTLPixelFormatRGBA16Float,            All},
    // Ordinary 128-bit pixel formats
    {MTLPixelFormatRGBA32Uint,             Write | Color},
    {MTLPixelFormatRGBA32Sint,             Write | Color},
    {MTLPixelFormatRGBA32Float,            Write | Color | Blend},
    // Compressed pixel formats (only compression types supported by Skia)
    {MTLPixelFormatETC2_RGB8,              Filter},
    {MTLPixelFormatETC2_RGB8_sRGB,         Filter},
    {MTLPixelFormatBC1_RGBA_,              NotAvailable},
    {MTLPixelFormatBC1_RGBA_sRGB_,         NotAvailable},
    // YUV pixel foramts
    {MTLPixelFormatGBGR422,                Filter},
    {MTLPixelFormatBGRG422,                Filter},
    // Depth and stencil pixel formats
    {MTLPixelFormatDepth16Unorm,           Filter | MSAA},
    {MTLPixelFormatDepth32Float,           MSAA},
    {MTLPixelFormatStencil8,               MSAA},
    {MTLPixelFormatDepth24Unorm_Stencil8_, NotAvailable},
    {MTLPixelFormatDepth32Float_Stencil8,  MSAA},
    {MTLPixelFormatX24_Stencil8_,          NotAvailable},
    {MTLPixelFormatX32_Stencil8,           NotAvailable},
    // Extended-range and wide-color pixel formats
    {MTLPixelFormatBGRA10_XR,              NotAvailable},
    {MTLPixelFormatBGRA10_XR_sRGB,         NotAvailable},
    {MTLPixelFormatBGR10_XR,               NotAvailable},
    {MTLPixelFormatBGR10_XR_sRGB,          NotAvailable}
};

static const FamilyFeatureMap kApple3 = extend(kApple2, {
    // Ordinary 8-bit pixel formats
    {MTLPixelFormatA8Unorm,                All},
    // Packed 32-bit pixel formats
    {MTLPixelFormatRGB10A2Unorm,           Write},
    {MTLPixelFormatRGB10A2Uint,            Write},
    {MTLPixelFormatRG11B10Float,           Write},
    {MTLPixelFormatRGB9E5Float,            Write},
    // Depth and stencil pixel formats,
    {MTLPixelFormatDepth16Unorm,           Resolve},
    {MTLPixelFormatDepth32Float,           Resolve},
    {MTLPixelFormatStencil8,               Resolve},
    {MTLPixelFormatDepth32Float_Stencil8,  Resolve},
    // Extended-range and wide-color pixel formats
    {MTLPixelFormatBGRA10_XR,              All},
    {MTLPixelFormatBGRA10_XR_sRGB,         All},
    {MTLPixelFormatBGR10_XR,               All},
    {MTLPixelFormatBGR10_XR_sRGB,          All}
});

static const FamilyFeatureMap kApple4 = extend(kApple3, {
    // Ordinary 16-bit pixel formats
    {MTLPixelFormatR16Unorm,               Resolve},
    {MTLPixelFormatR16Snorm,               Resolve},
    // Ordinary 32-bit pixel formats
    {MTLPixelFormatRG16Unorm,              Resolve},
    {MTLPixelFormatRG16Snorm,              Resolve},
    // Ordinary 64-bit pixel formats
    {MTLPixelFormatRGBA16Unorm,            Resolve},
    {MTLPixelFormatRGBA16Snorm,            Resolve},
});

static const FamilyFeatureMap kApple5 = kApple4; // No changes

// Apple6 adds Sparse to every format except for depth/stencil and YUV formats, which is done in
// bulk for everything that already has the Color feature, and then manually for compressed formats.
static const FamilyFeatureMap kApple6 = extend(kApple5, {
    // Ordinary 32-bit pixel formats (these also gain Atomic in Apple6+ from having Color)
    {MTLPixelFormatR32Uint,                Atomic},
    {MTLPixelFormatR32Sint,                Atomic},
    // Compressed pixel formats (which do not include the Color feature, but do gain Sparse)
    {MTLPixelFormatETC2_RGB8,              Sparse},
    {MTLPixelFormatETC2_RGB8_sRGB,         Sparse}},
    // Every Color-supporting pixel format also gains Sparse in this family
    {/*condition mask=*/Color,             Sparse}
);

static const FamilyFeatureMap kApple7 = extend(kApple6, {
    // Ordinary 128-bit pixel formats
    {MTLPixelFormatRGBA32Float,            MSAA},
    // Depth and stencil formats
    {MTLPixelFormatDepth16Unorm,           Sparse},
    {MTLPixelFormatDepth32Float,           Sparse},
    {MTLPixelFormatStencil8,               Sparse},
});

static const FamilyFeatureMap kApple8 = extend(kApple7, {
    // Ordinary 64-bit pixel formats
    {MTLPixelFormatRG32Uint,               Atomic},
    {MTLPixelFormatRG32Sint,               Atomic}
});

static const FamilyFeatureMap kApple9 = extend(kApple8, {
    // Ordinary 32-bit pixel formats
    {MTLPixelFormatR32Float,               Filter | Resolve},
    // Ordinary 64-bit pixel formats
    {MTLPixelFormatRG32Float,              Filter | Resolve},
    // Ordinary 128-bit pixel formats
    {MTLPixelFormatRGBA32Float,            Filter | Resolve},
    // Compressed pixel formats
    {MTLPixelFormatBC1_RGBA_,              Filter | Sparse},
    {MTLPixelFormatBC1_RGBA_sRGB_,         Filter | Sparse},
    // Depth nd stencil pixel formats
    {MTLPixelFormatDepth32Float,           Filter},
});

// Mac2 does not extend the Apple family, it is similar but has distinct values for many formats.
// Instead of defining a shared base map that both Mac2 and Apple2 extend, these two tables are
// copied in their entirety from the spec to make comparing to the PDF easier. This corresponds
// to the rightmost column of the table.
static const FamilyFeatureMap kMac2 = {
    // Ordinary 8-bit pixel foramts
    {MTLPixelFormatA8Unorm,                All},
    {MTLPixelFormatR8Unorm,                All},
    {MTLPixelFormatR8Unorm_sRGB,           NotAvailable},
    {MTLPixelFormatR8Snorm,                All},
    {MTLPixelFormatR8Uint,                 Write | Color | MSAA},
    {MTLPixelFormatR8Sint,                 Write | Color | MSAA},
    // Ordinary 16-bit pixel formats
    {MTLPixelFormatR16Unorm,               All},
    {MTLPixelFormatR16Snorm,               All},
    {MTLPixelFormatR16Uint,                Write | Color | MSAA},
    {MTLPixelFormatR16Sint,                Write | Color | MSAA},
    {MTLPixelFormatR16Float,               All},
    {MTLPixelFormatRG8Unorm,               All},
    {MTLPixelFormatRG8Unorm_sRGB,          NotAvailable},
    {MTLPixelFormatRG8Snorm,               All},
    {MTLPixelFormatRG8Uint,                Write | Color | MSAA},
    {MTLPixelFormatRG8Sint,                Write | Color | MSAA},
    // Packed 16-bit pixel formats
    {MTLPixelFormatB5G6R5Unorm,            NotAvailable},
    {MTLPixelFormatA1BGR5Unorm,            NotAvailable},
    {MTLPixelFormatABGR4Unorm,             NotAvailable},
    {MTLPixelFormatBGR5A1Unorm,            NotAvailable},
    // Ordinary 32-bit pixel formats
    {MTLPixelFormatR32Uint,                Atomic | Write | Color},
    {MTLPixelFormatR32Sint,                Atomic | Write | Color},
    {MTLPixelFormatR32Float,               All},
    {MTLPixelFormatRG16Unorm,              All},
    {MTLPixelFormatRG16Snorm,              All},
    {MTLPixelFormatRG16Uint,               Write | Color | MSAA},
    {MTLPixelFormatRG16Sint,               Write | Color | MSAA},
    {MTLPixelFormatRG16Float,              All},
    {MTLPixelFormatRGBA8Unorm,             All},
    {MTLPixelFormatRGBA8Unorm_sRGB,        Filter | Color | MSAA | Resolve | Blend},
    {MTLPixelFormatRGBA8Snorm,             All},
    {MTLPixelFormatRGBA8Uint,              Write | Color | MSAA},
    {MTLPixelFormatRGBA8Sint,              Write | Color | MSAA},
    {MTLPixelFormatBGRA8Unorm,             All},
    {MTLPixelFormatBGRA8Unorm_sRGB,        Filter | Color | MSAA | Resolve | Blend},
    // Packed 32-bit pixel formats
    {MTLPixelFormatRGB10A2Unorm,           All},
    {MTLPixelFormatBGR10A2Unorm,           All},
    {MTLPixelFormatRGB10A2Uint,            Write | Color | MSAA},
    {MTLPixelFormatRG11B10Float,           All},
    {MTLPixelFormatRGB9E5Float,            Filter},
    // Ordinary 64-bit pixel formats
    {MTLPixelFormatRG32Uint,               Write | Color | MSAA},
    {MTLPixelFormatRG32Sint,               Write | Color | MSAA},
    {MTLPixelFormatRG32Float,              All},
    {MTLPixelFormatRGBA16Unorm,            All},
    {MTLPixelFormatRGBA16Snorm,            All},
    {MTLPixelFormatRGBA16Uint,             Write | Color | MSAA},
    {MTLPixelFormatRGBA16Sint,             Write | Color | MSAA},
    {MTLPixelFormatRGBA16Float,            All},
    // Ordinary 128-bit pixel formats
    {MTLPixelFormatRGBA32Uint,             Write | Color | MSAA},
    {MTLPixelFormatRGBA32Sint,             Write | Color | MSAA},
    {MTLPixelFormatRGBA32Float,            All},
    // Compressed pixel formats (only compression types supported by Skia)
    {MTLPixelFormatETC2_RGB8,              NotAvailable},
    {MTLPixelFormatETC2_RGB8_sRGB,         NotAvailable},
    {MTLPixelFormatBC1_RGBA_,              Filter},
    {MTLPixelFormatBC1_RGBA_sRGB_,         Filter},
    // YUV pixel foramts
    {MTLPixelFormatGBGR422,                Filter},
    {MTLPixelFormatBGRG422,                Filter},
    // Depth and stencil pixel formats
    {MTLPixelFormatDepth16Unorm,           Filter | MSAA | Resolve},
    {MTLPixelFormatDepth32Float,           Filter | MSAA | Resolve},
    {MTLPixelFormatStencil8,               NotAvailable},
    {MTLPixelFormatDepth24Unorm_Stencil8_, Filter | MSAA | Resolve},
    {MTLPixelFormatDepth32Float_Stencil8,  Filter | MSAA | Resolve},
    {MTLPixelFormatX24_Stencil8_,          MSAA},
    {MTLPixelFormatX32_Stencil8,           NotAvailable},
    // Extended-range and wide-color pixel formats
    {MTLPixelFormatBGRA10_XR,              NotAvailable},
    {MTLPixelFormatBGRA10_XR_sRGB,         NotAvailable},
    {MTLPixelFormatBGR10_XR,               NotAvailable},
    {MTLPixelFormatBGR10_XR_sRGB,          NotAvailable}
};

} // anonymous namespace

SkEnumBitMask<MTLFeatureFlag> MTLPixelFormatSupport(MTLGPUFamily family, MTLPixelFormat format) {
    const FamilyFeatureMap* familyMap;
    // This assumes that family was pinned to Apple9 and this table should be updated as MtlCaps
    // detects newer families. Unfortunately the Apple family has a range of enum values lower than
    // Mac2 and the other deprecated families, so it makes it difficult to just clamp `family` here.
    switch (family) {
        case MTLGPUFamilyApple9: familyMap = &kApple9; break;
        case MTLGPUFamilyApple8: familyMap = &kApple8; break;
        case MTLGPUFamilyApple7: familyMap = &kApple7; break;
        case MTLGPUFamilyApple6: familyMap = &kApple6; break;
        case MTLGPUFamilyApple5: familyMap = &kApple5; break;
        case MTLGPUFamilyApple4: familyMap = &kApple4; break;
        case MTLGPUFamilyApple3: familyMap = &kApple3; break;
        case MTLGPUFamilyApple2: familyMap = &kApple2; break;
        case MTLGPUFamilyMac2:   familyMap = &kMac2;   break;
        default:                 familyMap = nullptr;  break;
    }

    if (familyMap) {
        // This automatically returns NotAvailable for valid MTLPixelFormats that do not have an
        // associated TextureFormat.
        const TextureFormat tf = MTLPixelFormatToTextureFormat(format);
        return familyMap->fFormatFeatures[(int) tf];
    }

    return {};
}

// *** Ground truth bidirectional map between TextureFormat and MTLPixelFormat ***

#define MTL_FORMAT_MAPPING(M) \
    M(TextureFormat::kR8,             MTLPixelFormatR8Unorm)                \
    M(TextureFormat::kR16,            MTLPixelFormatR16Unorm)               \
    M(TextureFormat::kR16F,           MTLPixelFormatR16Float)               \
    M(TextureFormat::kR32F,           MTLPixelFormatR32Float)               \
    M(TextureFormat::kA8,             MTLPixelFormatA8Unorm)                \
    M(TextureFormat::kRG8,            MTLPixelFormatRG8Unorm)               \
    M(TextureFormat::kRG16,           MTLPixelFormatRG16Unorm)              \
    M(TextureFormat::kRG16F,          MTLPixelFormatRG16Float)              \
    M(TextureFormat::kRG32F,          MTLPixelFormatRG32Float)              \
    /*TextureFormat::kRGB8,           unsupported */                        \
    /*TextureFormat::kBGR8,           unsupported */                        \
    M(TextureFormat::kB5_G6_R5,       MTLPixelFormatB5G6R5Unorm)            \
    /*TextureFormat::kR5_G6_B5,       unsupported */                        \
    /*TextureFormat::kRGB16,          unsupported */                        \
    /*TextureFormat::kRGB16F,         unsupported */                        \
    /*TextureFormat::kRGB32F,         unsupported */                        \
    /*TextureFormat::kRGB8_sRGB,      unsupported */                        \
    M(TextureFormat::kBGR10_XR,       MTLPixelFormatBGR10_XR)               \
    M(TextureFormat::kRGBA8,          MTLPixelFormatRGBA8Unorm)             \
    M(TextureFormat::kRGBA16,         MTLPixelFormatRGBA16Unorm)            \
    M(TextureFormat::kRGBA16F,        MTLPixelFormatRGBA16Float)            \
    M(TextureFormat::kRGBA32F,        MTLPixelFormatRGBA32Float)            \
    M(TextureFormat::kRGB10_A2,       MTLPixelFormatRGB10A2Unorm)           \
    /*TextureFormat::kRGBA10x6,       unsupported */                        \
    M(TextureFormat::kRGBA8_sRGB,     MTLPixelFormatRGBA8Unorm_sRGB)        \
    M(TextureFormat::kBGRA8,          MTLPixelFormatBGRA8Unorm)             \
    M(TextureFormat::kBGR10_A2,       MTLPixelFormatBGR10A2Unorm)           \
    M(TextureFormat::kBGRA8_sRGB,     MTLPixelFormatBGRA8Unorm_sRGB)        \
    M(TextureFormat::kABGR4,          MTLPixelFormatABGR4Unorm)             \
    /*TextureFormat::kARGB4,          unsupported */                        \
    M(TextureFormat::kBGRA10x6_XR,    MTLPixelFormatBGRA10_XR)              \
    M(TextureFormat::kRGB8_ETC2,      MTLPixelFormatETC2_RGB8)              \
    M(TextureFormat::kRGB8_ETC2_sRGB, MTLPixelFormatETC2_RGB8_sRGB)         \
    /*TextureFormat::kRGB8_BC1,       unsupported */                        \
    M(TextureFormat::kRGBA8_BC1,      MTLPixelFormatBC1_RGBA_)              \
    M(TextureFormat::kRGBA8_BC1_sRGB, MTLPixelFormatBC1_RGBA_sRGB_)         \
    /*TextureFormat::kYUV8_P2_420,    unsupported */                        \
    /*TextureFormat::kYUV8_P3_420,    unsupported */                        \
    /*TextureFormat::kYUV10x6_P2_420, unsupported */                        \
    /*TextureFormat::kExternal,       unsupported */                        \
    M(TextureFormat::kS8,             MTLPixelFormatStencil8)               \
    M(TextureFormat::kD16,            MTLPixelFormatDepth16Unorm)           \
    M(TextureFormat::kD32F,           MTLPixelFormatDepth32Float)           \
    M(TextureFormat::kD24_S8,         MTLPixelFormatDepth24Unorm_Stencil8_) \
    M(TextureFormat::kD32F_S8,        MTLPixelFormatDepth32Float_Stencil8)

TextureFormat MTLPixelFormatToTextureFormat(MTLPixelFormat format) {
#define M(TF, MTL) case MTL: return TF;
    switch(format) {
        MTL_FORMAT_MAPPING(M)
        default: return TextureFormat::kUnsupported;
    }
#undef M
}
MTLPixelFormat TextureFormatToMTLPixelFormat(TextureFormat format) {
#define M(TF, MTL) case TF: return MTL;
    switch(format) {
        MTL_FORMAT_MAPPING(M)
        default: return MTLPixelFormatInvalid;
    }
#undef M
}

}  // namespace skgpu::graphite
