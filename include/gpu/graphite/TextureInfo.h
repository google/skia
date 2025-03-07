/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_TextureInfo_DEFINED
#define skgpu_graphite_TextureInfo_DEFINED

#include "include/core/SkString.h"
#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/private/base/SkAPI.h"
#include "include/private/base/SkAnySubclass.h"

struct SkISize;

namespace skgpu::graphite {

enum class TextureFormat : uint8_t;

/**
 * TextureInfo is a backend-agnostic wrapper around the properties of a texture, sans dimensions.
 * It is designed this way to be compilable w/o bringing in a specific backend's build files, and
 * without requiring heap allocations of virtual types.
 */
class SK_API TextureInfo {
private:
    class Data;
    friend class MtlTextureInfo;
    friend class DawnTextureInfo;
    friend class VulkanTextureInfo;

    // Size is the largest of the Data subclasses assuming a 64-bit compiler.
    inline constexpr static size_t kMaxSubclassSize = 112;
    using AnyTextureInfoData = SkAnySubclass<Data, kMaxSubclassSize>;

    // Base properties for all backend-specific properties. Clients managing textures directly
    // should use the public subclasses of Data directly, e.g. MtlTextureInfo/DawnTextureInfo.
    //
    // Each backend subclass must expose to TextureInfo[Priv]:
    //   static constexpr BackendApi kBackend;
    //   Protected isProtected() const;
    //   TextureFormat viewFormat() const;
    //   bool serialize(SkWStream*) const;
    //   bool deserialize(SkStream*);
    class Data {
    public:
        virtual ~Data() = default;

        Data(uint32_t sampleCount, skgpu::Mipmapped mipmapped)
                : fSampleCount(sampleCount)
                , fMipmapped(mipmapped) {}

        Data() = default;
        Data(const Data&) = default;

        Data& operator=(const Data&) = default;

        // NOTE: These fields are accessible via the backend-specific subclasses.
        uint32_t fSampleCount = 1;
        Mipmapped fMipmapped = Mipmapped::kNo;

    private:
        friend class TextureInfo;
        friend class TextureInfoPriv;

        virtual SkString toBackendString() const = 0;

        virtual void copyTo(AnyTextureInfoData&) const = 0;
        // Passed in TextureInfo will have data of the same backend type and subclass, and
        // base properties of Data have already been checked for equality/compatibility.
        virtual bool isCompatible(const TextureInfo& that, bool requireExact) const = 0;
    };

public:
    TextureInfo() = default;
    ~TextureInfo() = default;

    TextureInfo(const TextureInfo&);
    TextureInfo& operator=(const TextureInfo&);

    bool operator==(const TextureInfo& that) const {
        return this->isCompatible(that, /*requireExact=*/true);
    }
    bool operator!=(const TextureInfo& that) const { return !(*this == that); }

    bool isValid() const { return fData.has_value(); }
    BackendApi backend() const {
        SkASSERT(fData.has_value() || fBackend == BackendApi::kUnsupported);
        return fBackend;
    }

    uint32_t numSamples() const { return fData.has_value() ? fData->fSampleCount : 1; }
    Mipmapped mipmapped() const { return fData.has_value() ? fData->fMipmapped   : Mipmapped::kNo; }
    Protected isProtected() const { return fProtected; }

    // Return true if `that` describes a texture that is compatible with this info and can validly
    // be used to fulfill a promise image that was created with this TextureInfo.
    bool canBeFulfilledBy(const TextureInfo& that) const {
        return this->isCompatible(that, /*requireExact=*/false);
    }

    // Return a string containing the full description of this TextureInfo.
    SkString toString() const;

private:
    friend class TextureInfoPriv;

    template <typename BackendTextureData,
              std::enable_if_t<std::is_base_of_v<Data, BackendTextureData>, bool> = true>
    explicit TextureInfo(const BackendTextureData& data)
            : fBackend(BackendTextureData::kBackend)
            , fViewFormat(data.viewFormat())
            , fProtected(data.isProtected()) {
        fData.emplace<BackendTextureData>(data);
    }

    bool isCompatible(const TextureInfo& that, bool requireExact) const;

    skgpu::BackendApi  fBackend = BackendApi::kUnsupported;
    AnyTextureInfoData fData;

    // Derived properties from the backend data, cached to avoid a virtual function call
    TextureFormat fViewFormat;
    Protected fProtected = Protected::kNo;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_TextureInfo_DEFINED
