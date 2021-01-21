/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#if !defined(SK_BUILD_FOR_GOOGLE3)  // Google3 doesn't build particles module

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "modules/particles/include/SkParticleEffect.h"
#include "modules/particles/include/SkParticleSerialization.h"
#include "modules/skresources/include/SkResources.h"
#include "src/sksl/SkSLVMGenerator.h"
#include "tools/Resources.h"

struct UniformValue {
    const char*        fName;
    std::vector<float> fData;
};

class ParticlesGM : public skiagm::GM {
public:
    ParticlesGM(const char* name,
                double startTime,
                SkISize size,
                SkPoint origin,
                std::vector<UniformValue> uniforms = {})
            : GM(SK_ColorBLACK)
            , fName(name)
            , fStartTime(startTime)
            , fSize(size)
            , fOrigin(origin)
            , fUniforms(std::move(uniforms)) {}

    SkString onShortName() override { return SkStringPrintf("particles_%s", fName); }
    SkISize onISize() override { return fSize; }

    void onOnceBeforeDraw() override {
        SkParticleEffect::RegisterParticleTypes();

        auto jsonData = GetResourceAsData(SkStringPrintf("particles/%s.json", fName).c_str());
        skjson::DOM dom(static_cast<const char*>(jsonData->data()), jsonData->size());
        SkFromJsonVisitor fromJson(dom.root());

        auto resourceProvider = skresources::FileResourceProvider::Make(GetResourcePath());
        auto effectParams = sk_make_sp<SkParticleEffectParams>();
        effectParams->visitFields(&fromJson);
        effectParams->prepare(resourceProvider.get());

        fEffect = sk_make_sp<SkParticleEffect>(effectParams);
        for (const auto& val : fUniforms) {
            SkAssertResult(fEffect->setUniform(val.fName, val.fData.data(), val.fData.size()));
        }

        fEffect->start(/*now=*/0.0, /*looping=*/true);

        // Fast-forward (in 30 fps time-slices) to the requested time
        for (double time = 0; time < fStartTime; time += 1.0 / 30) {
            fEffect->update(/*now=*/std::min(time, fStartTime));
        }
    }

    bool onAnimate(double nanos) override {
        if (fEffect) {
            fEffect->update(fStartTime + (nanos * 1E-9));
        }
        return true;
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->save();
        canvas->translate(fOrigin.fX, fOrigin.fY);
        fEffect->draw(canvas);
        canvas->restore();
    }

protected:
    const char*               fName;
    const double              fStartTime;
    const SkISize             fSize;
    const SkPoint             fOrigin;
    sk_sp<SkParticleEffect>   fEffect;
    std::vector<UniformValue> fUniforms;
};

DEF_GM(return new ParticlesGM("confetti",     1.0, {400, 400}, {200, 200});)
DEF_GM(return new ParticlesGM("cube",         1.0, {400, 400}, {200, 200});)
DEF_GM(return new ParticlesGM("curves",       4.0, {100, 200}, { 50, 190});)
DEF_GM(return new ParticlesGM("mandrill",     1.0, {250, 250}, { 25,  25});)
DEF_GM(return new ParticlesGM("spiral",       2.0, {250, 250}, {125, 125});)
DEF_GM(return new ParticlesGM("sprite_frame", 1.0, {200, 200}, {100, 100});)
DEF_GM(return new ParticlesGM("text",         1.0, {250, 110}, { 10, 100});)
DEF_GM(return new ParticlesGM("uniforms",     2.0, {250, 250}, {125, 125},
                                              {{"rate",  {2.0f}},
                                               {"spin",  {4.0f}},
                                               {"color", {0.25f, 0.75f, 0.75f}}});)

#endif  // SK_BUILD_FOR_GOOGLE3
