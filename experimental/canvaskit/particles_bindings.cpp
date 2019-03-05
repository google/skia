/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkTypes.h"
#include "SkParticleAffector.h"
#include "SkParticleDrawable.h"
#include "SkParticleEffect.h"
#include "SkParticleSerialization.h"
#include "SkRandom.h"

#include <string>

#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

EMSCRIPTEN_BINDINGS(Particles) {
    class_<SkParticleEffect>("SkParticleEffect")
        .smart_ptr<sk_sp<SkParticleEffect>>("sk_sp<SkParticleEffect>")
        .function("draw", &SkParticleEffect::draw, allow_raw_pointers())
        .function("start", select_overload<void (double, bool)>(&SkParticleEffect::start))
        .function("update", select_overload<void (double)>(&SkParticleEffect::update));

    function("MakeParticles", optional_override([](std::string json)->sk_sp<SkParticleEffect> {
        static bool didInit = false;
        if (!didInit) {
            REGISTER_REFLECTED(SkReflected);
            SkParticleAffector::RegisterAffectorTypes();
            SkParticleDrawable::RegisterDrawableTypes();
            didInit = true;
        }
        SkRandom r;
        sk_sp<SkParticleEffectParams> params(new SkParticleEffectParams());
        skjson::DOM dom(json.c_str(), json.length());
        SkFromJsonVisitor fromJson(dom.root());
        params->visitFields(&fromJson);
        return sk_sp<SkParticleEffect>(new SkParticleEffect(std::move(params), r));
    }));
    constant("particles", true);

}
