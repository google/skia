/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/KeyContext.h"

#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"

namespace skgpu::graphite {

KeyContext::KeyContext(const Caps* caps,
                       FloatStorageManager* floatStorageManager,
                       PaintParamsKeyBuilder* paintParamsKeyBuilder,
                       PipelineDataGatherer* pipelineDataGatherer,
                       ShaderCodeDictionary* dict,
                       sk_sp<RuntimeEffectDictionary> rtEffectDict,
                       const SkColorInfo& dstColorInfo)
        : fCaps(caps)
        , fRecorder(nullptr)
        , fFloatStorageManager(floatStorageManager)
        , fPaintParamsKeyBuilder(paintParamsKeyBuilder)
        , fPipelineDataGatherer(pipelineDataGatherer)
        , fDictionary(dict)
        , fRTEffectDict(std::move(rtEffectDict))
        , fDstColorInfo(dstColorInfo) {}

KeyContext::KeyContext(skgpu::graphite::Recorder* recorder,
                       DrawContext* drawContext,
                       FloatStorageManager* floatStorageManager,
                       PaintParamsKeyBuilder* paintParamsKeyBuilder,
                       PipelineDataGatherer* pipelineDataGatherer,
                       const SkM44& local2Dev,
                       const SkColorInfo& dstColorInfo,
                       SkEnumBitMask<KeyGenFlags> initialFlags,
                       const SkColor4f& paintColor)
        : fCaps(recorder->priv().caps())
        , fRecorder(recorder)
        , fDC(drawContext)
        , fFloatStorageManager(floatStorageManager)
        , fPaintParamsKeyBuilder(paintParamsKeyBuilder)
        , fPipelineDataGatherer(pipelineDataGatherer)
        , fDictionary(recorder->priv().shaderCodeDictionary())
        , fRTEffectDict(recorder->priv().runtimeEffectDictionary())
        , fLocal2Dev(local2Dev)
        , fLocalMatrix(nullptr)
        , fDstColorInfo(dstColorInfo)
        , fKeyGenFlags(initialFlags) {\
    fPaintColor = PaintParams::Color4fPrepForDst(paintColor, fDstColorInfo).makeOpaque().premul();
    fPaintColor.fA = paintColor.fA;
}

KeyContext::KeyContext(const KeyContext& other,
                       SkEnumBitMask<KeyGenFlags> xtraFlags)
        : fCaps(other.fCaps)
        , fRecorder(other.fRecorder)
        , fDC(other.fDC)
        , fFloatStorageManager(other.fFloatStorageManager)
        , fPaintParamsKeyBuilder(other.fPaintParamsKeyBuilder)
        , fPipelineDataGatherer(other.fPipelineDataGatherer)
        , fDictionary(other.fDictionary)
        , fRTEffectDict(other.fRTEffectDict)
        , fLocal2Dev(other.fLocal2Dev)
        , fLocalMatrix(other.fLocalMatrix)
        , fDstColorInfo(other.fDstColorInfo)
        , fPaintColor(other.fPaintColor)
        , fKeyGenFlags(other.fKeyGenFlags | xtraFlags) {}

KeyContext::~KeyContext() {}

sk_sp<RuntimeEffectDictionary> KeyContext::rtEffectDict() const { return fRTEffectDict; }

KeyContext KeyContext::forRuntimeEffect(const SkRuntimeEffect* effect, int child) const {
    // Runtime effects always disable paint-color colorization of alpha-only image shaders
    SkEnumBitMask<KeyGenFlags> xtraFlags = KeyGenFlags::kDisableAlphaOnlyImageColorization;

    if (SkRuntimeEffectPriv::ChildSampleUsage(effect, child).isExplicit()) {
        // Assume explicit sampling as a proxy for either a likely data lookup (e.g. raw shader)
        // or an effect that might sample the child many times. This means it's worth using
        // eliding colorspace conversions, and we have to disable sampling optimization.
        xtraFlags |= KeyGenFlags::kEnableIdentityColorSpaceXform |
                     KeyGenFlags::kDisableSamplingOptimization;
    }

    return this->withExtraFlags(xtraFlags);
}

} // namespace skgpu::graphite
