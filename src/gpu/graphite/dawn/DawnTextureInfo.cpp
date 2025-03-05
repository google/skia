/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/gpu/graphite/dawn/DawnGraphiteTypes.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/dawn/DawnGraphiteUtils.h"

#include <cstdint>

namespace skgpu::graphite {

DawnTextureInfo::DawnTextureInfo(WGPUTexture texture)
        : DawnTextureInfo(
                wgpuTextureGetSampleCount(texture),
                wgpuTextureGetMipLevelCount(texture) > 1 ? Mipmapped::kYes : Mipmapped::kNo,
                /*format=*/static_cast<wgpu::TextureFormat>(wgpuTextureGetFormat(texture)),
                /*viewFormat=*/static_cast<wgpu::TextureFormat>(wgpuTextureGetFormat(texture)),
                static_cast<wgpu::TextureUsage>(wgpuTextureGetUsage(texture)),
                wgpu::TextureAspect::All,
                /*slice=*/0) {}

TextureFormat DawnTextureInfo::viewFormat() const {
#if !defined(__EMSCRIPTEN__)
    if (fYcbcrVkDescriptor.externalFormat != 0) {
        return TextureFormat::kExternal;
    }
#endif
    return DawnFormatToTextureFormat(this->getViewFormat());
}

SkString DawnTextureInfo::toBackendString() const {
    return SkStringPrintf("wgpuFormat=%u,usage=0x%08X,aspect=0x%08X,slice=%u",
                          static_cast<unsigned int>(fFormat),
                          static_cast<unsigned int>(fUsage),
                          static_cast<unsigned int>(fAspect),
                          fSlice);
}

bool DawnTextureInfo::isCompatible(const TextureInfo& that, bool requireExact) const {
    const auto& dt = TextureInfoPriv::Get<DawnTextureInfo>(that);

    // The usages may match or the usage passed in may be a superset of the usage stored within. The
    // YCbCrInfo must be equal. The aspect should either match the plane aspect or should be All.
    return this->getViewFormat() == dt.getViewFormat() &&
            (fUsage & dt.fUsage) == fUsage &&
#if !defined(__EMSCRIPTEN__)
            DawnDescriptorsAreEquivalent(fYcbcrVkDescriptor, dt.fYcbcrVkDescriptor) &&
#endif
            (fAspect == dt.fAspect || (!requireExact && fAspect == wgpu::TextureAspect::All));
}

bool DawnTextureInfo::serialize(SkWStream* stream) const {

    if (!stream->write32(static_cast<uint32_t>(fFormat)))     { return false; }
    if (!stream->write32(static_cast<uint32_t>(fViewFormat))) { return false; }
    if (!stream->write64(static_cast<uint64_t>(fUsage)))      { return false; }
    if (!stream->write32(static_cast<uint32_t>(fAspect)))     { return false; }
    if (!stream->write32(fSlice))                             { return false; }

#if !defined(__EMSCRIPTEN__)
    bool hasYCbCr = DawnDescriptorIsValid(fYcbcrVkDescriptor);
    if (!stream->writeBool(hasYCbCr)) { return false; }

    if (hasYCbCr) {
        SkASSERT(SkTFitsIn<uint8_t>(fYcbcrVkDescriptor.vkChromaFilter));

        // Except for the last three members, Dawn stores these all as uint32_ts although the
        // values stored within them will usually need far fewer bits.
        if (!stream->write32(fYcbcrVkDescriptor.vkFormat))                { return false; }
        if (!stream->write32(fYcbcrVkDescriptor.vkYCbCrModel))            { return false; }
        if (!stream->write32(fYcbcrVkDescriptor.vkYCbCrRange))            { return false; }
        if (!stream->write32(fYcbcrVkDescriptor.vkComponentSwizzleRed))   { return false; }
        if (!stream->write32(fYcbcrVkDescriptor.vkComponentSwizzleGreen)) { return false; }
        if (!stream->write32(fYcbcrVkDescriptor.vkComponentSwizzleBlue))  { return false; }
        if (!stream->write32(fYcbcrVkDescriptor.vkComponentSwizzleAlpha)) { return false; }
        if (!stream->write32(fYcbcrVkDescriptor.vkXChromaOffset))         { return false; }
        if (!stream->write32(fYcbcrVkDescriptor.vkYChromaOffset))         { return false; }
        if (!stream->write32(static_cast<uint32_t>(fYcbcrVkDescriptor.vkChromaFilter))) {
            return false;
        }
        if (!stream->writeBool(fYcbcrVkDescriptor.forceExplicitReconstruction)) {
            return false;
        }
        if (!stream->write64(fYcbcrVkDescriptor.externalFormat))          { return false; }
    }
#endif

    return true;
}

bool DawnTextureInfo::deserialize(SkStream* stream) {
    uint32_t tmp32;

    if (!stream->readU32(&tmp32)) {
        return false;
    }
    // TODO(robertphillips): add validity checks to deserialized values
    fFormat = static_cast<wgpu::TextureFormat>(tmp32);

    if (!stream->readU32(&tmp32)) {
        return false;
    }
    fViewFormat = static_cast<wgpu::TextureFormat>(tmp32);

    uint64_t tmp64;
    if (!stream->readU64(&tmp64)) {
        return false;
    }
    fUsage = static_cast<wgpu::TextureUsage>(tmp64);

    if (!stream->readU32(&tmp32)) {
        return false;
    }
    fAspect = static_cast<wgpu::TextureAspect>(tmp32);

    if (!stream->readU32(&fSlice)) {
        return false;
    }

#if !defined(__EMSCRIPTEN__)
    bool tmpBool;
    if (!stream->readBool(&tmpBool)) {
        return false;
    }

    if (/* hasYbCr */ tmpBool) {
        if (!stream->readU32(&fYcbcrVkDescriptor.vkFormat))                { return false; }
        if (!stream->readU32(&fYcbcrVkDescriptor.vkYCbCrModel))            { return false; }
        if (!stream->readU32(&fYcbcrVkDescriptor.vkYCbCrRange))            { return false; }
        if (!stream->readU32(&fYcbcrVkDescriptor.vkComponentSwizzleRed))   { return false; }
        if (!stream->readU32(&fYcbcrVkDescriptor.vkComponentSwizzleGreen)) { return false; }
        if (!stream->readU32(&fYcbcrVkDescriptor.vkComponentSwizzleBlue))  { return false; }
        if (!stream->readU32(&fYcbcrVkDescriptor.vkComponentSwizzleAlpha)) { return false; }
        if (!stream->readU32(&fYcbcrVkDescriptor.vkXChromaOffset))         { return false; }
        if (!stream->readU32(&fYcbcrVkDescriptor.vkYChromaOffset))         { return false; }

        if (!stream->readU32(&tmp32))                                      { return false; }
        fYcbcrVkDescriptor.vkChromaFilter = static_cast<wgpu::FilterMode>(tmp32);
        if (!stream->readBool(&tmpBool))                                  { return false; }
        fYcbcrVkDescriptor.forceExplicitReconstruction = tmpBool;

        if (!stream->readU64(&fYcbcrVkDescriptor.externalFormat))          { return false; }
    }
#endif

    return true;
}

namespace TextureInfos {

TextureInfo MakeDawn(const DawnTextureInfo& dawnInfo) {
    return TextureInfoPriv::Make(dawnInfo);
}

bool GetDawnTextureInfo(const TextureInfo& info, DawnTextureInfo* out) {
    return TextureInfoPriv::Copy(info, out);
}

}  // namespace TextureInfos

}  // namespace skgpu::graphite
