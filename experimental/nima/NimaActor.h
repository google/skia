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

#include "SkCanvas.h"
#include "SkData.h"
#include "SkImage.h"

#include <string>

class NimaActor;
class NimaActorImage;

class NimaActor : public nima::Actor {
public:
    NimaActor(std::string nimaPath, std::string texturePath);
    NimaActor(sk_sp<SkData> nimaBytes, sk_sp<SkData> textureBytes);
    ~NimaActor() = default;

    void render(SkCanvas* canvas, uint32_t renderFlags);

    void render(SkCanvas* canvas) {
        this->render(canvas, 0);
    }
    /**
     * Updates the animation state to be at time t.
     *
     * @param t - number of second in (modulo total duration)
     *
     */
    void seek(SkScalar t);

    void setAnimation(uint8_t index) {
        if (index < fAnimationNames.size()) {
            fAnimationIndex = index;
            fAnimationInstance = this->animationInstance(fAnimationNames[fAnimationIndex]);
        }
    }

    void setAnimation(std::string name) {
        for (size_t i = 0; i < fAnimationNames.size(); i++)
        {
            std::string aName = fAnimationNames[i];
            if (aName == name)
            {
                setAnimation(i);
                return;
            }
        }
    }

    const std::vector<std::string>& getAnimationNames() const {
        return fAnimationNames;
    }

    /**
     * Returns the duration of the current Actor's animation in seconds.
     */
    SkScalar duration() const {
        if (fAnimationInstance) {
            return fAnimationInstance->duration();
        }
        return 0.0f;
    }

private:
    sk_sp<SkImage>                fTexture;
    std::vector<NimaActorImage>   fActorImages;
    std::unique_ptr<SkPaint>      fPaint;
    std::vector<std::string>      fAnimationNames;
    nima::ActorAnimationInstance* fAnimationInstance;
    uint8_t                       fAnimationIndex;

    typedef nima::Actor INHERITED;
};

enum RenderFlags {
    kImmediate_RenderFlag = 0x1,
    kCache_RenderFlag     = 0x2,
    kBounds_RenderFlag    = 0x4,
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
