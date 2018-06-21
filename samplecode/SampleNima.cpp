/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SampleNimaActor.h"
#include "SkAnimTimer.h"
#include "SkView.h"
#include <nima/Animation/ActorAnimationInstance.hpp>
#include <cmath>

using namespace nima;

class NimaView : public SampleView {
public:
    NimaView()
        : fActor(nullptr)
        , fAnimation(nullptr) {
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Nima");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onOnceBeforeDraw() override {
        // Create the actor.
        fActor = std::make_unique<SampleActor>("Robot");

        // Get the animation.
        fAnimation = fActor->animationInstance("attack");
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->save();

        canvas->translate(500, 500);
        canvas->scale(1, -1);

        // Render the actor.
        fActor->render(canvas);

        canvas->restore();
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        // Apply the animation.
        if (fAnimation) {
            float time = std::fmod(timer.secs(), fAnimation->max());
            fAnimation->time(time);
            fAnimation->apply(1.0f);
        }
        return true;
    }

private:
    std::unique_ptr<SampleActor> fActor;
    ActorAnimationInstance*      fAnimation;

    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new NimaView; }
static SkViewRegister reg(MyFactory);
