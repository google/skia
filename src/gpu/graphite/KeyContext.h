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
#include "src/base/SkEnumBitMask.h"
#include "src/core/SkColorData.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/gpu/graphite/TextureProxy.h"

class SkRuntimeEffect;

namespace skgpu::graphite {

class Caps;
enum class DstReadStrategy : uint8_t;
class Recorder;
class RuntimeEffectDictionary;
class ShaderCodeDictionary;

enum class KeyGenFlags : uint8_t {
    kDefault = 0b0,
    // By default, linear sampling can be optimized to nearest when it's visually equivalent.
    // This flag disables this behavior.
    kDisableSamplingOptimization       = 0b001,
    // By default, identity color conversions map to ColorSpaceTransformPremul as a reasonably
    // performant baseline that avoids shader combinatorics. However, in certain contexts (such as
    // image filters or runtime effects) that sample an image many times *and* perform up front
    // work to ensure there doesn't need to be any color conversion, skipping color space conversion
    // in the shader produces meaningful performance improvements.
    kEnableIdentityColorSpaceXform     = 0b010,
    // By default, alpha-only image shaders are colorized by the paint's color. In the context of
    // a runtime effect this is disabled.
    kDisableAlphaOnlyImageColorization = 0b100,
};
SK_MAKE_BITMASK_OPS(KeyGenFlags)

// The key context must always be able to provide a valid ShaderCodeDictionary and
// SkRuntimeEffectDictionary. Depending on the calling context it can also supply a
// backend-specific resource providing object (e.g., a Recorder).
class KeyContext {
public:
    // Constructor for the pre-compile code path (i.e., no Recorder)
    KeyContext(const Caps* caps,
               ShaderCodeDictionary* dict,
               RuntimeEffectDictionary* rtEffectDict,
               const SkColorInfo& dstColorInfo)
            : fDictionary(dict)
            , fRTEffectDict(rtEffectDict)
            , fDstColorInfo(dstColorInfo)
            , fCaps(caps) {}

    // Constructor for the ExtractPaintData code path (i.e., with a Recorder)
    KeyContext(Recorder*,
               const SkM44& local2Dev,
               const SkColorInfo& dstColorInfo,
               SkEnumBitMask<KeyGenFlags> initialFlags,
               const SkColor4f& paintColor);

    KeyContext(const KeyContext&);

    Recorder* recorder() const { return fRecorder; }

    const Caps* caps() const { return fCaps; }

    const SkM44& local2Dev() const { return fLocal2Dev; }
    const SkMatrix* localMatrix() const { return fLocalMatrix; }

    ShaderCodeDictionary* dict() const { return fDictionary; }
    RuntimeEffectDictionary* rtEffectDict() const { return fRTEffectDict; }

    const SkColorInfo& dstColorInfo() const { return fDstColorInfo; }

    const SkPMColor4f& paintColor() const { return fPaintColor; }

    SkEnumBitMask<KeyGenFlags> flags() const { return fKeyGenFlags; }

protected:
    Recorder* fRecorder = nullptr;
    SkM44 fLocal2Dev;
    SkMatrix* fLocalMatrix = nullptr;
    ShaderCodeDictionary* fDictionary;
    RuntimeEffectDictionary* fRTEffectDict;
    SkColorInfo fDstColorInfo;
    // Although stored as premul the paint color is actually comprised of an opaque RGB portion
    // and a separate alpha portion. The two portions will never be used together but are stored
    // together to reduce the number of uniforms.
    SkPMColor4f fPaintColor = SK_PMColor4fBLACK;
    SkEnumBitMask<KeyGenFlags> fKeyGenFlags = KeyGenFlags::kDefault;

private:
    const Caps* fCaps = nullptr;
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

class KeyContextWithColorInfo : public KeyContext {
public:
    KeyContextWithColorInfo(const KeyContext& other, const SkColorInfo& info) : KeyContext(other) {
        // We want to keep fPaintColor's alpha value but replace the RGB with values in the new
        // color space
        SkPMColor4f tmp = fPaintColor;
        tmp.fA = 1.0f;
        SkColorSpaceXformSteps(fDstColorInfo, info).apply(tmp.vec());
        fPaintColor.fR = tmp.fR;
        fPaintColor.fG = tmp.fG;
        fPaintColor.fB = tmp.fB;
        fDstColorInfo = info;
    }

private:
    KeyContextWithColorInfo(const KeyContextWithColorInfo&) = delete;
    KeyContextWithColorInfo& operator=(const KeyContextWithColorInfo&) = delete;
};

// The key generation flags vary in the scope of a SkRuntimeEffect per child based on how the RE's
// SkSL invokes each child.
class KeyContextForRuntimeEffect : public KeyContext {
public:
    KeyContextForRuntimeEffect(const KeyContext& other, const SkRuntimeEffect* effect, int child);


private:
    KeyContextForRuntimeEffect(const KeyContextForRuntimeEffect&) = delete;
    KeyContextForRuntimeEffect& operator=(const KeyContextForRuntimeEffect&) = delete;
};

class KeyContextWithCoordClamp : public KeyContext {
public:
    KeyContextWithCoordClamp(const KeyContext& other) : KeyContext(other) {
        // Subtleties in clamping implmentation can lead to texture samples at non pixel aligned
        // coordinates, particularly if clamped to non-texel centers.
        fKeyGenFlags |= KeyGenFlags::kDisableSamplingOptimization;
    }

private:
    KeyContextWithCoordClamp(const KeyContextWithCoordClamp&) = delete;
    KeyContextWithCoordClamp& operator=(const KeyContextWithCoordClamp&) = delete;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_KeyContext_DEFINED
