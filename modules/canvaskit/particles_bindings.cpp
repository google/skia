/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkTypes.h"
#include "include/utils/SkRandom.h"
#include "modules/particles/include/SkParticleEffect.h"
#include "modules/particles/include/SkParticleSerialization.h"
#include "modules/skresources/include/SkResources.h"
#include "src/sksl/SkSLByteCode.h"

#include <string>

#include "modules/canvaskit/WasmCommon.h"

#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

namespace {

class ParticleAssetProvider : public skresources::ResourceProvider {
public:
    ~ParticleAssetProvider() override = default;

    // Tried using a map, but that gave strange errors like
    // https://emscripten.org/docs/porting/guidelines/function_pointer_issues.html
    // Not entirely sure why, but perhaps the iterator in the map was
    // confusing enscripten.
    using AssetVec = std::vector<std::pair<SkString, sk_sp<SkData>>>;

    static sk_sp<ParticleAssetProvider> Make(AssetVec assets) {
        if (assets.empty()) {
            return nullptr;
        }

        return sk_sp<ParticleAssetProvider>(new ParticleAssetProvider(std::move(assets)));
    }

    sk_sp<skresources::ImageAsset> loadImageAsset(const char[] /* path */,
                                                  const char name[],
                                                  const char[] /* id */) const override {
        // For CK we ignore paths & IDs, and identify images based solely on name.
        if (auto data = this->findAsset(name)) {
            return skresources::MultiFrameImageAsset::Make(std::move(data));
        }

        return nullptr;
    }

    sk_sp<SkData> loadFont(const char name[], const char[] /* url */) const override {
        // Same as images paths, we ignore font URLs.
        return this->findAsset(name);
    }

private:
    explicit ParticleAssetProvider(AssetVec assets) : fAssets(std::move(assets)) {}

    sk_sp<SkData> findAsset(const char name[]) const {
        for (const auto& asset : fAssets) {
            if (asset.first.equals(name)) {
                return asset.second;
            }
        }

        SkDebugf("Could not find %s\n", name);
        return nullptr;
    }

    const AssetVec fAssets;
};

}

struct SimpleUniform {
    int columns;
    int rows;
    int slot; // the index into the uniforms array that this uniform begins.
};

SimpleUniform fromUniform(SkSL::ByteCode::Uniform u) {
    SimpleUniform su;
    su.columns = u.fColumns;
    su.rows = u.fRows;
    su.slot = u.fSlot;
    return su;
}

EMSCRIPTEN_BINDINGS(Particles) {
    class_<SkParticleEffect>("SkParticleEffect")
        .smart_ptr<sk_sp<SkParticleEffect>>("sk_sp<SkParticleEffect>")
        .function("draw", &SkParticleEffect::draw, allow_raw_pointers())
        .function("_effectUniformPtr", optional_override([](SkParticleEffect& self)->uintptr_t {
            return reinterpret_cast<uintptr_t>(self.effectUniforms());
        }))
        .function("_particleUniformPtr", optional_override([](SkParticleEffect& self)->uintptr_t {
            return reinterpret_cast<uintptr_t>(self.particleUniforms());
        }))
        .function("getEffectUniformCount", optional_override([](SkParticleEffect& self)->int {
            auto ec = self.effectCode();
            if (!ec) {
                return -1;
            }
            return ec->getUniformCount();
        }))
        .function("getEffectUniformFloatCount", optional_override([](SkParticleEffect& self)->int {
            auto ec = self.effectCode();
            if (!ec) {
                return -1;
            }
            return ec->getUniformSlotCount();
        }))
        .function("getEffectUniformName", optional_override([](SkParticleEffect& self, int i)->JSString {
            auto ec = self.effectCode();
            if (!ec) {
                return emscripten::val::null();
            }
            return emscripten::val(ec->getUniform(i).fName.c_str());
        }))
        .function("getEffectUniform", optional_override([](SkParticleEffect& self, int i)->SimpleUniform {
            SimpleUniform su;
            auto ec = self.effectCode();
            if (!ec) {
                return su;
            }
            su = fromUniform(ec->getUniform(i));
            return su;
        }))
        .function("getParticleUniformCount", optional_override([](SkParticleEffect& self)->int {
            auto ec = self.particleCode();
            if (!ec) {
                return -1;
            }
            return ec->getUniformCount();
        }))
        .function("getParticleUniformFloatCount", optional_override([](SkParticleEffect& self)->int {
            auto ec = self.particleCode();
            if (!ec) {
                return -1;
            }
            return ec->getUniformSlotCount();
        }))
        .function("getParticleUniformName", optional_override([](SkParticleEffect& self, int i)->JSString {
            auto ec = self.particleCode();
            if (!ec) {
                return emscripten::val::null();
            }
            return emscripten::val(ec->getUniform(i).fName.c_str());
        }))
        .function("getParticleUniform", optional_override([](SkParticleEffect& self, int i)->SimpleUniform {
            SimpleUniform su;
            auto ec = self.particleCode();
            if (!ec) {
                return su;
            }
            su = fromUniform(ec->getUniform(i));
            return su;
        }))
        .function("setPosition", select_overload<void (SkPoint)>(&SkParticleEffect::setPosition))
        .function("setRate", select_overload<void (float)>(&SkParticleEffect::setRate))
        .function("start", select_overload<void (double, bool)>(&SkParticleEffect::start))
        .function("update", select_overload<void (double)>(&SkParticleEffect::update));

    value_object<SimpleUniform>("SimpleUniform")
        .field("columns", &SimpleUniform::columns)
        .field("rows",    &SimpleUniform::rows)
        .field("slot",    &SimpleUniform::slot);

    function("_MakeParticles", optional_override([](std::string json,
                                                   size_t assetCount,
                                                   uintptr_t /* char**    */ nptr,
                                                   uintptr_t /* uint8_t** */ dptr,
                                                   uintptr_t /* size_t*   */ sptr)
                                                ->sk_sp<SkParticleEffect> {
        // See the comment in canvaskit_bindings.cpp about the use of uintptr_t
        static bool didInit = false;
        if (!didInit) {
            SkParticleEffect::RegisterParticleTypes();
            didInit = true;
        }

        const auto assetNames = reinterpret_cast<char**   >(nptr);
        const auto assetDatas = reinterpret_cast<uint8_t**>(dptr);
        const auto assetSizes = reinterpret_cast<size_t*  >(sptr);

        ParticleAssetProvider::AssetVec assets;
        assets.reserve(assetCount);

        for (size_t i = 0; i < assetCount; i++) {
            auto name  = SkString(assetNames[i]);
            auto bytes = SkData::MakeFromMalloc(assetDatas[i], assetSizes[i]);
            assets.push_back(std::make_pair(std::move(name), std::move(bytes)));
        }

        SkRandom r;
        sk_sp<SkParticleEffectParams> params(new SkParticleEffectParams());
        skjson::DOM dom(json.c_str(), json.length());
        SkFromJsonVisitor fromJson(dom.root());
        params->visitFields(&fromJson);
        params->prepare(skresources::DataURIResourceProviderProxy::Make(
                            ParticleAssetProvider::Make(std::move(assets))).get());
        return sk_sp<SkParticleEffect>(new SkParticleEffect(std::move(params), r));
    }));
    constant("particles", true);

}
