/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ResourceTypes_DEFINED
#define skgpu_graphite_ResourceTypes_DEFINED

#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTileMode.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkMathPriv.h"

namespace skgpu::graphite {

class Buffer;

// This declaration of the DepthStencilFlags' SkEnumBitMask ops is here bc, internally, we use
// DepthStencilFlags as bit fields but, externally (i.e., from the GraphiteTypes view), we want
// it to appear as just an enum class.
SK_MAKE_BITMASK_OPS(DepthStencilFlags)

/**
 * The strategy that a renderpass and/or pipeline use to access the current dst pixel when blending.
 */
enum class DstReadStrategy : uint8_t {
    kNoneRequired,
    kTextureCopy,
    kTextureSample,  // TODO(b/238756862): To be used once direct texture sampling is implemented
    kReadFromInput,
    kFramebufferFetch,

    kLast = kFramebufferFetch
};
inline static constexpr int kDstReadStrategyCount = (int)(DstReadStrategy::kLast) + 1;

/**
 * This enum is used to specify the load operation to be used when a RenderPass begins execution.
 */
enum class LoadOp : uint8_t {
    kLoad,
    kClear,
    kDiscard,

    kLast = kDiscard
};
inline static constexpr int kLoadOpCount = (int)(LoadOp::kLast) + 1;

/**
 * This enum is used to specify the store operation to be used when a RenderPass ends execution.
 */
enum class StoreOp : uint8_t {
    kStore,
    kDiscard,

    kLast = kDiscard
};
inline static constexpr int kStoreOpCount = (int)(StoreOp::kLast) + 1;

/**
 * What a GPU buffer will be used for
 */
enum class BufferType : int {
    kVertex,
    kIndex,
    kXferCpuToGpu,
    kXferGpuToCpu,
    kUniform,
    kStorage,
    kQuery,

    // GPU-only buffer types
    kIndirect,
    kVertexStorage,
    kIndexStorage,

    kLast = kIndexStorage,
};
static const int kBufferTypeCount = static_cast<int>(BufferType::kLast) + 1;

/**
 * Data layout requirements on host-shareable buffer contents.
 */
enum class Layout : uint8_t {
    kInvalid = 0,
    kStd140,
    kStd430,
    kMetal,
};

static constexpr const char* LayoutString(Layout layout) {
    switch(layout) {
        case Layout::kStd140:  return "std140";
        case Layout::kStd430:  return "std430";
        case Layout::kMetal:   return "metal";
        case Layout::kInvalid: return "invalid";
    }
    SkUNREACHABLE;
}

/**
 * Indicates the intended access pattern over resource memory. This is used to select the most
 * efficient memory type during resource creation based on the capabilities of the platform.
 *
 * This is only a hint and the actual memory type will be determined based on the resource type and
 * backend capabilities.
 */
enum class AccessPattern : uint8_t {
    // GPU-only memory does not need to support reads/writes from the CPU. GPU-private memory will
    // be preferred if the backend supports an efficient private memory type.
    kGpuOnly,

    // The resource needs to be CPU visible, e.g. for read-back or as a copy/upload source.
    kHostVisible,

    // Use to debug GPU only buffers.
    kGpuOnlyCopySrc,
};

/**
 * Determines whether the contents of a GPU buffer sub-allocation gets cleared to 0 before being
 * used in a GPU command submission.
 */
enum class ClearBuffer : bool {
    kNo = false,
    kYes = true,
};

/**
 * Must the contents of the Resource be preserved af a render pass or can a more efficient
 * representation be chosen when supported by hardware.
 */
enum class Discardable : bool {
    kNo = false,
    kYes = true
};

enum class Ownership : uint8_t {
    kOwned,
    kWrapped,
};

/** Uniquely identifies the type of resource that is cached with a GraphiteResourceKey. */
using ResourceType = uint32_t;

/**
 * Can the resource be held by multiple users at the same time?
 * For example, stencil buffers, pipelines, etc.
 */
enum class Shareable : uint8_t {
    kNo,      // The resource is visible in the ResourceCache once all its usage refs are dropped
    kScratch, // The resource is visible to other Recorders, but acts like kNo within a Recording
    kYes,     // The resource is always visible in the ResourceCache
};

/*
 * Struct that can be passed into bind buffer calls on the CommandBuffer. The ownership of the
 * buffer and its usage in command submission must be tracked by the caller (e.g. as with
 * buffers created by DrawBufferManager).
 */
struct BindBufferInfo {
    const Buffer* fBuffer = nullptr;
    uint32_t fOffset = 0;
    uint32_t fSize = 0;

    operator bool() const { return SkToBool(fBuffer); }

    bool operator==(const BindBufferInfo& o) const {
        return fBuffer == o.fBuffer && (!fBuffer || (fOffset == o.fOffset && fSize == o.fSize));
    }
    bool operator!=(const BindBufferInfo& o) const { return !(*this == o); }
};

struct ImmutableSamplerInfo {
    // If the sampler requires YCbCr conversion, backends can place that information here.
    // In order to fit within SamplerDesc's uint32 desc field, backends can only utilize up to
    // kMaxNumConversionInfoBits bits.
    uint32_t fNonFormatYcbcrConversionInfo = 0;
    // fFormat represents known OR external format numerical representation.
    uint64_t fFormat = 0;
};


/**
 * Struct used to describe how a Texture/TextureProxy/TextureProxyView is sampled.
 */
struct SamplerDesc {
    static_assert(kSkTileModeCount <= 4 && kSkFilterModeCount <= 2 && kSkMipmapModeCount <= 4);

    constexpr SamplerDesc(const SkSamplingOptions& samplingOptions, SkTileMode tileMode)
            : SamplerDesc(samplingOptions, {tileMode, tileMode}) {}

    constexpr SamplerDesc(const SkSamplingOptions& samplingOptions,
                          const std::pair<SkTileMode, SkTileMode> tileModes,
                          const ImmutableSamplerInfo info = {})
            : fDesc((static_cast<int>(tileModes.first)            << kTileModeXShift           ) |
                    (static_cast<int>(tileModes.second)           << kTileModeYShift           ) |
                    (static_cast<int>(samplingOptions.filter)     << kFilterModeShift          ) |
                    (static_cast<int>(samplingOptions.mipmap)     << kMipmapModeShift          ) |
                    (info.fNonFormatYcbcrConversionInfo           << kImmutableSamplerInfoShift) )
            , fFormat(info.fFormat)
            , fExternalFormatMostSignificantBits(info.fFormat >> 32) {

        // Cubic sampling is handled in a shader, with the actual texture sampled by with NN,
        // but that is what a cubic SkSamplingOptions is set to if you ignore 'cubic', which let's
        // us simplify how we construct SamplerDec's from the options passed to high-level draws.
        SkASSERT(!samplingOptions.useCubic || (samplingOptions.filter == SkFilterMode::kNearest &&
                                               samplingOptions.mipmap == SkMipmapMode::kNone));

        // TODO: Add aniso value when used.

        // Assert that fYcbcrConversionInfo does not exceed kMaxNumConversionInfoBits such that
        // the conversion information can fit within an uint32.
        SkASSERT(info.fNonFormatYcbcrConversionInfo >> kMaxNumConversionInfoBits == 0);
    }
    constexpr SamplerDesc() = default;
    constexpr SamplerDesc(const SamplerDesc&) = default;

#if defined(GPU_TEST_UTILS)
    constexpr SamplerDesc(uint32_t desc, uint32_t format, uint32_t extFormatMSB)
            : fDesc(desc)
            , fFormat(format)
            , fExternalFormatMostSignificantBits(extFormatMSB) {}
#endif

    bool operator==(const SamplerDesc& o) const {
        return o.fDesc == fDesc && o.fFormat == fFormat &&
               o.fExternalFormatMostSignificantBits == fExternalFormatMostSignificantBits;
    }

    bool operator!=(const SamplerDesc& o) const { return !(*this == o); }

    SkTileMode tileModeX()          const {
        return static_cast<SkTileMode>((fDesc >> kTileModeXShift) & 0b11);
    }
    SkTileMode tileModeY()          const {
        return static_cast<SkTileMode>((fDesc >> kTileModeYShift) & 0b11);
    }
    SkFilterMode filterMode()       const {
        return static_cast<SkFilterMode>((fDesc >> kFilterModeShift) & 0b01);
    }
    SkMipmapMode mipmap()           const {
        return static_cast<SkMipmapMode>((fDesc >> kMipmapModeShift) & 0b11);
    }
    uint32_t   desc()               const { return fDesc;                                        }
    uint32_t   format()             const { return fFormat;                                      }
    uint32_t   externalFormatMSBs() const { return fExternalFormatMostSignificantBits;           }
    bool       isImmutable()        const { return (fDesc >> kImmutableSamplerInfoShift) != 0;   }
    bool       usesExternalFormat() const { return (fDesc >> kImmutableSamplerInfoShift) & 0b1;  }

    // NOTE: returns the HW sampling options to use, so a bicubic SkSamplingOptions will become
    // nearest-neighbor sampling in HW.
    SkSamplingOptions samplingOptions() const {
        // TODO: Add support for anisotropic filtering
        SkFilterMode filter = static_cast<SkFilterMode>((fDesc >> kFilterModeShift) & 0b01);
        SkMipmapMode mipmap = static_cast<SkMipmapMode>((fDesc >> kMipmapModeShift) & 0b11);
        return SkSamplingOptions(filter, mipmap);
    }

    ImmutableSamplerInfo immutableSamplerInfo() const {
        return {this->desc() >> kImmutableSamplerInfoShift,
                ((uint64_t) this->externalFormatMSBs() << 32) | (uint64_t) this->format()};
    }

    SkSpan<const uint32_t> asSpan() const {
        // Span length depends upon whether the sampler is immutable and if it uses a known format
        return {&fDesc, 1 + this->isImmutable() + this->usesExternalFormat()};
    }

    // These are public such that backends can bitshift data in order to determine whatever
    // sampler qualities they need from fDesc.
    static constexpr int kNumTileModeBits   = SkNextLog2_portable(int(SkTileMode::kLastTileMode)+1);
    static constexpr int kNumFilterModeBits = SkNextLog2_portable(int(SkFilterMode::kLast)+1);
    static constexpr int kNumMipmapModeBits = SkNextLog2_portable(int(SkMipmapMode::kLast)+1);
    static constexpr int kMaxNumConversionInfoBits =
            32 - kNumFilterModeBits - kNumMipmapModeBits - kNumTileModeBits;

    static constexpr int kTileModeXShift            = 0;
    static constexpr int kTileModeYShift            = kTileModeXShift  + kNumTileModeBits;
    static constexpr int kFilterModeShift           = kTileModeYShift  + kNumTileModeBits;
    static constexpr int kMipmapModeShift           = kFilterModeShift + kNumFilterModeBits;
    static constexpr int kImmutableSamplerInfoShift = kMipmapModeShift + kNumMipmapModeBits;

    // Only relevant when using immutable samplers. Otherwise, can be ignored. The number of uint32s
    // required to represent all relevant sampler desc information depends upon whether we are using
    // a known or external format.
    static constexpr int kInt32sNeededKnownFormat = 2;
    static constexpr int kInt32sNeededExternalFormat = 3;

private:
    // Note: The order of these member attributes matters to keep unique object representation
    // such that SkGoodHash can be used to hash SamplerDesc objects.
    uint32_t fDesc = 0;

    // Data fields populated by backend Caps which store texture format information (needed for
    // YCbCr sampling). Only relevant when using immutable samplers. Otherwise, can be ignored.
    // Known formats only require a uint32, but external formats can be up to a uint64. We store
    // this as two separate uint32s such that has_unique_object_representation can be true, allowing
    // this structure to be easily hashed using SkGoodHash. So, external formats can be represented
    // with (fExternalFormatMostSignificantBits << 32) | fFormat.
    uint32_t fFormat = 0;
    uint32_t fExternalFormatMostSignificantBits = 0;
};

};  // namespace skgpu::graphite

#endif // skgpu_graphite_ResourceTypes_DEFINED
