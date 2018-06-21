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
#include "SkImage.h"

class SampleActor;
class SampleActorImage;

class SampleActor : public nima::Actor {
public:
    SampleActor(std::string baseName);
    ~SampleActor();

    void render(SkCanvas* canvas) const;

private:
    sk_sp<SkImage>                fTexture;
    std::vector<SampleActorImage> fActorImages;
    std::unique_ptr<SkPaint>      fPaint;

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
