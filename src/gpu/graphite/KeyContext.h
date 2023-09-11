/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_KeyContext_DEFINED
#define skgpu_graphite_KeyContext_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/private/SkColorData.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

class Caps;
enum class DstReadRequirement;
class Recorder;
class RuntimeEffectDictionary;
class ShaderCodeDictionary;

// The key context must always be able to provide a valid ShaderCodeDictionary and
// SkRuntimeEffectDictionary. Depending on the calling context it can also supply a
// backend-specific resource providing object (e.g., a Recorder).
class KeyContext {
public:
    // Constructor for the pre-compile code path (i.e., no Recorder)
    KeyContext(const Caps* caps,
               ShaderCodeDictionary* dict,
               RuntimeEffectDictionary* rtEffectDict,
               const SkColorInfo& dstColorInfo,
               sk_sp<TextureProxy> dstTexture,
               SkIPoint dstOffset)
            : fDictionary(dict)
            , fRTEffectDict(rtEffectDict)
            , fDstColorInfo(dstColorInfo)
            , fCaps(caps)
            , fDstTexture(std::move(dstTexture))
            , fDstOffset(dstOffset) {}

    // Constructor for the ExtractPaintData code path (i.e., with a Recorder)
    KeyContext(Recorder*,
               const SkM44& local2Dev,
               const SkColorInfo&,
               const SkColor4f& paintColor,
               sk_sp<TextureProxy> dstTexture,
               SkIPoint dstOffset);

    KeyContext(const KeyContext&);

    Recorder* recorder() const { return fRecorder; }

    const Caps* caps() const { return fCaps; }

    const SkM44& local2Dev() const { return fLocal2Dev; }
    const SkMatrix* localMatrix() const { return fLocalMatrix; }

    ShaderCodeDictionary* dict() const { return fDictionary; }
    RuntimeEffectDictionary* rtEffectDict() const { return fRTEffectDict; }

    const SkColorInfo& dstColorInfo() const { return fDstColorInfo; }

    // Proxy to the destination texture, if it needs to be read from, or null otherwise.
    sk_sp<TextureProxy> dstTexture() const { return fDstTexture; }
    // Offset within dstTexture to the top-left corner of the area that needs to be read.
    SkIPoint dstOffset() const { return fDstOffset; }

    const SkPMColor4f& paintColor() const { return fPaintColor; }

protected:
    Recorder* fRecorder = nullptr;
    SkM44 fLocal2Dev;
    SkMatrix* fLocalMatrix = nullptr;
    ShaderCodeDictionary* fDictionary;
    RuntimeEffectDictionary* fRTEffectDict;
    SkColorInfo fDstColorInfo;
    SkPMColor4f fPaintColor = SK_PMColor4fBLACK;

private:
    const Caps* fCaps = nullptr;
    sk_sp<TextureProxy> fDstTexture;
    SkIPoint fDstOffset;
};

class KeyContextWithLocalMatrix : public KeyContext {
public:
    KeyContextWithLocalMatrix(const KeyContext& other, const SkMatrix& childLM)
            : KeyContext(other) {
        if (fLocalMatrix) {
            fStorage = SkMatrix::Concat(childLM, *fLocalMatrix);
        } else {
            fStorage = childLM;
        }

        fLocalMatrix = &fStorage;
    }

private:
    KeyContextWithLocalMatrix(const KeyContextWithLocalMatrix&) = delete;
    KeyContextWithLocalMatrix& operator=(const KeyContextWithLocalMatrix&) = delete;

    SkMatrix fStorage;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_KeyContext_DEFINED
