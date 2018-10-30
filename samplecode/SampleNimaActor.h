/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SampleNimaActor_DEFINED
#define SampleNimaActor_DEFINED

#include <nima/Actor.hpp>
#include <nima/ActorImage.hpp>
#include <nima/Vec2D.hpp>

#include "SkCanvas.h"
#include "SkData.h"
#include "SkImage.h"

class SampleActor;
class SampleActorImage;

class SampleActor : public nima::Actor {
public:
    SampleActor(std::string baseName);
    SampleActor(sk_sp<SkData> nimaBytes, sk_sp<SkData> textureBytes);
    ~SampleActor();

    void render(SkCanvas* canvas) const;
    /**
     * Updates the animation state for |t|.
     *
     * @param t   normalized [0..1] frame selector (0 -> first frame, 1 -> final frame)
     *
     */
    void seek(SkScalar t);

    void setAnimation(uint8_t index) {
        this->fAnimationIndex = index;
    }

    const std::vector<std::string>& getAnimations() const {
        return fAnimations;
    }

private:
    sk_sp<SkImage>                fTexture;
    std::vector<SampleActorImage> fActorImages;
    std::unique_ptr<SkPaint>      fPaint;
    std::vector<std::string>      fAnimations;
    uint8_t                       fAnimationIndex;

    typedef nima::Actor INHERITED;
};

class SampleActorImage {
public:
    SampleActorImage(nima::ActorImage* actorImage, sk_sp<SkImage> texture, SkPaint* paint);
    ~SampleActorImage();

    void render(const SampleActor* actor, SkCanvas* canvas) const;

    int drawOrder() const { return fActorImage->drawOrder(); }

private:
    nima::Vec2D deform(const nima::Vec2D& position, float* boneIdx, float* boneWgt) const;

private:
    nima::ActorImage* fActorImage;
    sk_sp<SkImage>    fTexture;
    SkPaint*          fPaint;
};

#endif
