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
            : fFloatStorageManager(floatStorageManager)
            , fPaintParamsKeyBuilder(paintParamsKeyBuilder)
            , fPipelineDataGatherer(pipelineDataGatherer)
            , fDictionary(dict)
            , fRTEffectDict(std::move(rtEffectDict))
            , fDstColorInfo(dstColorInfo)
            , fCaps(caps) {}

KeyContext::KeyContext(skgpu::graphite::Recorder* recorder,
                       DrawContext* drawContext,
                       FloatStorageManager* floatStorageManager,
                       PaintParamsKeyBuilder* paintParamsKeyBuilder,
                       PipelineDataGatherer* pipelineDataGatherer,
                       const SkM44& local2Dev,
                       const SkColorInfo& dstColorInfo,
                       SkEnumBitMask<KeyGenFlags> initialFlags,
                       const SkColor4f& paintColor)
        : fRecorder(recorder)
        , fDC(drawContext)
        , fFloatStorageManager(floatStorageManager)
        , fPaintParamsKeyBuilder(paintParamsKeyBuilder)
        , fPipelineDataGatherer(pipelineDataGatherer)
        , fLocal2Dev(local2Dev)
        , fLocalMatrix(nullptr)
        , fDstColorInfo(dstColorInfo)
        , fKeyGenFlags(initialFlags)
        , fCaps(recorder->priv().caps()) {
    fDictionary = fRecorder->priv().shaderCodeDictionary();
    fRTEffectDict = fRecorder->priv().runtimeEffectDictionary();
    fPaintColor = PaintParams::Color4fPrepForDst(paintColor, fDstColorInfo).makeOpaque().premul();
    fPaintColor.fA = paintColor.fA;
}

KeyContext::KeyContext(const KeyContext& other)
        : fRecorder(other.fRecorder)
        , fDC(other.fDC)
        , fFloatStorageManager(other.fFloatStorageManager)
        , fPaintParamsKeyBuilder(other.fPaintParamsKeyBuilder)
        , fPipelineDataGatherer(other.fPipelineDataGatherer)
        , fLocal2Dev(other.fLocal2Dev)
        , fLocalMatrix(other.fLocalMatrix)
        , fDictionary(other.fDictionary)
        , fRTEffectDict(other.fRTEffectDict)
        , fDstColorInfo(other.fDstColorInfo)
        , fPaintColor(other.fPaintColor)
        , fKeyGenFlags(other.fKeyGenFlags)
        , fCaps(other.fCaps) {}

KeyContext::~KeyContext() {}

sk_sp<RuntimeEffectDictionary> KeyContext::rtEffectDict() const { return fRTEffectDict; }

KeyContextForRuntimeEffect::KeyContextForRuntimeEffect(const KeyContext& other,
                                                       const SkRuntimeEffect* effect,
                                                       int child)
        : KeyContext(other) {
    // Runtime effects always disable paint-color colorization of alpha-only image shaders
    fKeyGenFlags |= KeyGenFlags::kDisableAlphaOnlyImageColorization;

    if (SkRuntimeEffectPriv::ChildSampleUsage(effect, child).isExplicit()) {
        // Assume explicit sampling as a proxy for either a likely data lookup (e.g. raw shader)
        // or an effect that might sample the child many times. This means it's worth using
        // eliding colorspace conversions, and we have to disable sampling optimization.
        fKeyGenFlags |= KeyGenFlags::kEnableIdentityColorSpaceXform |
                        KeyGenFlags::kDisableSamplingOptimization;

    }
}

} // namespace skgpu::graphite
