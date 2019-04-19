/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Sample.h"

#include "AnimTimer.h"
#include "Resources.h"
#include "nima/NimaActor.h"

#include <nima/Animation/ActorAnimationInstance.hpp>
#include <cmath>

using namespace nima;

class NimaView : public Sample {
public:
    NimaView()
        : fActor(nullptr) {
    }

protected:
    virtual bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "Nima");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onOnceBeforeDraw() override {
        // Create the actor.
        std::string nimaPath(GetResourcePath("nima/Robot.nima").c_str());
        std::string texturePath(GetResourcePath("nima/Robot.png").c_str());

        fActor = std::make_unique<NimaActor>(nimaPath, texturePath);

        // Also available: dance, jump, idle
        fActor->setAnimation("attack");
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->save();

        canvas->translate(500, 700);
        canvas->scale(1, -1);

        // Render the actor.
        fActor->render(canvas);

        canvas->restore();
    }

    bool onAnimate(const AnimTimer& timer) override {
        if (fActor) {
            float time = std::fmod(timer.secs(), fActor->duration());
            fActor->seek(time);
        }
        return true;
    }

private:
    std::unique_ptr<NimaActor>   fActor;

    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new NimaView(); )
