/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef AndroidRuntimeEffectManager_DEFINED
#define AndroidRuntimeEffectManager_DEFINED

#include "include/core/SkRefCnt.h"

#include <map>
#include <string>

class SkRuntimeEffect;

namespace ui {

enum class Dataspace : int32_t {
    UNKNOWN,
    SRGB,
    BT2020,
    BT2020_ITU_PQ,
    DISPLAY_P3,
    V0_SRGB,
};

} // namespace ui

namespace shaders {

struct LinearEffect {
    ui::Dataspace inputDataspace = ui::Dataspace::SRGB;
    ui::Dataspace outputDataspace = ui::Dataspace::SRGB;
    bool undoPremultipliedAlpha = false;
    ui::Dataspace fakeOutputDataspace = ui::Dataspace::UNKNOWN;

    enum SkSLType { Shader, ColorFilter };
    SkSLType type = Shader;
};

} // namespace shaders

class RuntimeEffectManager {
public:
    RuntimeEffectManager();

    enum class KnownId {
        kBlurFilter_MixEffect,
        kEdgeExtensionEffect,
        kKawaseBlurDualFilter_HighSampleBlurEffect,
        kKawaseBlurDualFilter_LowSampleBlurEffect,
        kMouriMap_BlurEffect,
        kMouriMap_CrossTalkAndChunk16x16Effect,
        kMouriMap_Chunk8x8Effect,
        kMouriMap_TonemapEffect,
    };

    sk_sp<SkRuntimeEffect> getKnownRuntimeEffect(KnownId id);

    sk_sp<SkRuntimeEffect> getOrCreateLinearRuntimeEffect(const shaders::LinearEffect&);

private:
    sk_sp<SkRuntimeEffect> fMixEffect;
    sk_sp<SkRuntimeEffect> fEdgeExtensionEffect;
    sk_sp<SkRuntimeEffect> fKawaseHighSampleEffect;
    sk_sp<SkRuntimeEffect> fKawaseLowSampleEffect;
    sk_sp<SkRuntimeEffect> fBlurEffect;
    sk_sp<SkRuntimeEffect> fCrosstalkAndChunk16x16Effect;
    sk_sp<SkRuntimeEffect> fChunk8x8Effect;
    sk_sp<SkRuntimeEffect> fToneMapEffect;

    std::map<std::string, sk_sp<SkRuntimeEffect>> fLinearEffects;
};

#endif // AndroidRuntimeEffectManager_DEFINED
