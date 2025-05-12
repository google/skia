/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/KeyContext.h"

#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/RecorderPriv.h"

namespace skgpu::graphite {

KeyContext::KeyContext(skgpu::graphite::Recorder* recorder,
                       const SkM44& local2Dev,
                       const SkColorInfo& dstColorInfo,
                       SkEnumBitMask<KeyGenFlags> initialFlags,
                       const SkColor4f& paintColor)
        : fRecorder(recorder)
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
        , fLocal2Dev(other.fLocal2Dev)
        , fLocalMatrix(other.fLocalMatrix)
        , fDictionary(other.fDictionary)
        , fRTEffectDict(other.fRTEffectDict)
        , fDstColorInfo(other.fDstColorInfo)
        , fPaintColor(other.fPaintColor)
        , fKeyGenFlags(other.fKeyGenFlags)
        , fCaps(other.fCaps) {}

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
