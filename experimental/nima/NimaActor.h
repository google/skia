/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef NimaActor_DEFINED
#define NimaActor_DEFINED

#include <nima/Actor.hpp>
#include <nima/ActorImage.hpp>
#include <nima/Vec2D.hpp>

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"

#include <string>

class NimaActor;
class NimaActorImage;

enum RenderFlags {
    kImmediate_RenderFlag = 0x1,
    kCache_RenderFlag     = 0x2,
    kBounds_RenderFlag    = 0x4,
};

/** \class NimaActor
    NimaActor acts as a breidge between Skia and a nima::Actor object.
    The Actor object essentially is a set of bones and textures.

    NimaActor knows how to draw itself (the Actor) to an SkCanvas
    at various time stamps.

    NimaActor is also aware of the different animation types the
    Actor has and coordinates switching between them. For example,
    an animation might have an "idle" and a "jump" animation it can
    switch between.
*/
class NimaActor : public nima::Actor {
public:

    NimaActor(std::string nimaPath, std::string texturePath);
    NimaActor(sk_sp<SkData> nimaBytes, sk_sp<SkData> textureBytes);

    ~NimaActor() = default;

    /**
     * Render draws itself to the given canvas, at whatever
     * the current time position is (see seek).
     */
    void render(SkCanvas* canvas, uint32_t renderFlags = 0);
    /**
     * Updates the animation state to be at time t.
     * This does not re-draw anything, another call to render() is required.
     *
     * @param t - number of second in (modulo total duration)
     *
     */
    void seek(SkScalar t);

    /**
     * Returns the duration of the current Actor's animation in seconds.
     */
    SkScalar duration() const;

    /**
     * Sets the animation type based on the index given. The default
     * animation index is 0. If index is invalid, nothing changes.
     */
    void setAnimation(uint8_t index);

    /**
     * Sets the animation type to be one that matches the provided
     * name. If the name does not match any of the existing animation
     * types, nothing changes.
     */
    void setAnimation(std::string name);

    /**
     * Returns all possible animation names. Use with setAnimation().
     */
    const std::vector<std::string>& getAnimationNames() const {
        return fAnimationNames;
    }

private:
    void init();
    sk_sp<SkImage>                fTexture;
    std::vector<NimaActorImage>   fActorImages;
    std::unique_ptr<SkPaint>      fPaint;
    std::vector<std::string>      fAnimationNames;
    nima::ActorAnimationInstance* fAnimationInstance;
    uint8_t                       fAnimationIndex;

    typedef nima::Actor INHERITED;
};

// A wrapper class that handles rendering of ActorImages (renderable components NIMA Actors).
class NimaActorImage {
public:
    NimaActorImage(nima::ActorImage* actorImage, SkImage* texture, SkPaint* paint);
    ~NimaActorImage() = default;

    void render(SkCanvas* canvas, uint32_t renderFlags);

    int drawOrder() const { return fActorImage->drawOrder(); }

private:
    nima::ActorImage* fActorImage;
    SkImage*          fTexture;
    SkPaint*          fPaint;

    bool                                 fSkinned;
    std::vector<SkPoint>                 fPositions;
    std::vector<SkPoint>                 fTexs;
    std::vector<SkVertices::BoneIndices> fBoneIdx;
    std::vector<SkVertices::BoneWeights> fBoneWgt;
    std::vector<uint16_t>                fIndices;

    std::vector<SkVertices::Bone> fBones;
    sk_sp<SkVertices>             fVertices;

    uint32_t fRenderFlags;

    void updateVertices(bool isVolatile);
    void updateBones();
    void drawVerticesObject(SkVertices* vertices, SkCanvas* canvas, bool useBones) const;
};

#endif
