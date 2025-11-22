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
class DrawContext;
enum class DstReadStrategy : uint8_t;
class FloatStorageManager;
class PaintParamsKeyBuilder;
class PipelineDataGatherer;
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
    KeyContext(const Caps*,
               FloatStorageManager*,
               PaintParamsKeyBuilder*,
               PipelineDataGatherer*,
               ShaderCodeDictionary*,
               sk_sp<RuntimeEffectDictionary>,
               const SkColorInfo& dstColorInfo);

    // Constructor for the ExtractPaintData code path (i.e., with a Recorder)
    KeyContext(Recorder*,
               DrawContext*,
               FloatStorageManager*,
               PaintParamsKeyBuilder*,
               PipelineDataGatherer*,
               const SkM44& local2Dev,
               const SkColorInfo& dstColorInfo,
               SkEnumBitMask<KeyGenFlags> initialFlags,
               const SkColor4f& paintColor);

    KeyContext(const KeyContext&, SkEnumBitMask<KeyGenFlags> xtraFlags=KeyGenFlags::kDefault);
    ~KeyContext();

    // Create scoped KeyContexts that allow child effects to be processed differently.
    KeyContext withColorInfo(const SkColorInfo& info) const {
        KeyContext o = *this;
        o.fDstColorInfo = info;

        // We want to keep fPaintColor's alpha value but replace the RGB with values in the new
        // color space. By overriding the alpha type of the old and new dst color infos to be
        // kOpaque, SkColorSpaceXformSteps will leave the alpha channel alone.
        SkColorSpaceXformSteps(fDstColorInfo.colorSpace(), kOpaque_SkAlphaType,
                               info.colorSpace(),          kOpaque_SkAlphaType)
                .apply(o.fPaintColor.vec());
        SkASSERT(o.fPaintColor.fA == fPaintColor.fA);
        return o;
    }

    // The key generation flags vary in the scope of a SkRuntimeEffect per child based on how the
    // RuntimeEffect's SkSL invokes each child.
    KeyContext forRuntimeEffect(const SkRuntimeEffect* effect, int child) const;

    KeyContext withExtraFlags(SkEnumBitMask<KeyGenFlags> flags) const {
        return KeyContext(*this, flags);
    }

    Recorder* recorder() const { return fRecorder; }
    DrawContext* drawContext() const { return fDC; }

    const Caps* caps() const { return fCaps; }

    const SkM44& local2Dev() const { return fLocal2Dev; }
    const SkMatrix* localMatrix() const { return fLocalMatrix; }

    FloatStorageManager* floatStorageManager() const { return fFloatStorageManager; }
    PaintParamsKeyBuilder* paintParamsKeyBuilder() const { return fPaintParamsKeyBuilder; }
    PipelineDataGatherer* pipelineDataGatherer() const { return fPipelineDataGatherer; }
    ShaderCodeDictionary* dict() const { return fDictionary; }

    sk_sp<RuntimeEffectDictionary> rtEffectDict() const;

    const SkColorInfo& dstColorInfo() const { return fDstColorInfo; }

    const SkPMColor4f& paintColor() const { return fPaintColor; }

    SkEnumBitMask<KeyGenFlags> flags() const { return fKeyGenFlags; }

private:
    // Fields which will not change over the course of building a paint key
    const Caps* fCaps;
    Recorder* fRecorder;
    DrawContext* fDC;
    FloatStorageManager* fFloatStorageManager;
    PaintParamsKeyBuilder* fPaintParamsKeyBuilder;
    PipelineDataGatherer* fPipelineDataGatherer;
    ShaderCodeDictionary* fDictionary;
    sk_sp<RuntimeEffectDictionary> fRTEffectDict;
    SkM44 fLocal2Dev;

protected:
    // Fields that can be modified while walking a paint's effects for a key
    SkMatrix* fLocalMatrix = nullptr;
    SkColorInfo fDstColorInfo;
    // Although stored as premul the paint color is actually comprised of an opaque RGB portion
    // and a separate alpha portion. The two portions will never be used together but are stored
    // together to reduce the number of uniforms.
    SkPMColor4f fPaintColor = SK_PMColor4fBLACK;
    SkEnumBitMask<KeyGenFlags> fKeyGenFlags = KeyGenFlags::kDefault;
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
