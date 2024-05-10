/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/KeyContext.h"

#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/RecorderPriv.h"

namespace skgpu::graphite {

KeyContext::KeyContext(skgpu::graphite::Recorder* recorder,
                       const SkM44& local2Dev,
                       const SkColorInfo& dstColorInfo,
                       OptimizeSampling optimizeSampling,
                       const SkColor4f& paintColor,
                       sk_sp<TextureProxy> dstTexture,
                       SkIPoint dstOffset)
        : fRecorder(recorder)
        , fLocal2Dev(local2Dev)
        , fLocalMatrix(nullptr)
        , fDstColorInfo(dstColorInfo)
        , fOptimizeSampling(optimizeSampling)
        , fCaps(recorder->priv().caps())
        , fDstTexture(std::move(dstTexture))
        , fDstOffset(dstOffset) {
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
        , fScope(other.fScope)
        , fOptimizeSampling(other.fOptimizeSampling)
        , fCaps(other.fCaps)
        , fDstTexture(other.fDstTexture)
        , fDstOffset(other.fDstOffset) {}

} // namespace skgpu::graphite
