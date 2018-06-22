/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef NIMASlide_DEFINED
#define NIMASlide_DEFINED

#include "Slide.h"

#include "SkSkeleton.h"
#include "SkCanvas.h"
#include <nima/Actor.hpp>
#include <nima/ActorImage.hpp>
#include <nima/Animation/ActorAnimationInstance.hpp>
#include <nima/Vec2D.hpp>

class NIMAActor;
class NIMAActorImage;

class NIMAActor : public nima::Actor {
public:
    NIMAActor(const std::string& basePath);
    ~NIMAActor();

    void render(SkCanvas* canvas, int backend) const;

    const std::vector<std::string>& getAnimations() const { return fAnimations; }

private:
    sk_sp<SkImage>              fTexture;
    std::vector<NIMAActorImage> fActorImages;
    std::unique_ptr<SkPaint>    fPaint;
    std::vector<std::string>    fAnimations;

    typedef nima::Actor INHERITED;
};

class NIMAActorImage {
public:
    NIMAActorImage(nima::ActorImage* actorImage, sk_sp<SkImage> texture, SkPaint* paint);
    ~NIMAActorImage();

    void renderBackend(SkCanvas* canvas);
    void renderImmediate(SkCanvas* canvas);

    int drawOrder() const { return fActorImage->drawOrder(); }

private:
    void updateVertices();
    void updateSkeleton();

    void updateVerticesObject(bool applyDeforms);
    void drawVerticesObject(SkCanvas* canvas, bool useSkeleton) const;

    nima::Vec2D deform(const nima::Vec2D& position, int* boneIdx, float* boneWgt) const;

private:
    nima::ActorImage* fActorImage;
    sk_sp<SkImage>    fTexture;
    SkPaint*          fPaint;

    bool                                 fSkinned;
    std::vector<SkPoint>                 fPositions;
    std::vector<SkPoint>                 fTexs;
    std::vector<SkSkeleton::Attachment> fAttachments;
    std::vector<int>                     fBoneIdx;
    std::vector<float>                   fBoneWgt;
    std::vector<uint16_t>                fIndices;

    std::vector<SkMatrix> fBones;
    sk_sp<SkVertices>     fVertices;
    sk_sp<SkSkeleton>     fSkeleton;

    int fRenderMode;
};

class NIMASlide : public Slide {
public:
    NIMASlide(const SkString& name, const SkString& path);
    ~NIMASlide() override;

    void draw(SkCanvas* canvas) override;
    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;
    bool animate(const SkAnimTimer& timer) override;

    bool onChar(SkUnichar c) override;
    bool onMouse(SkScalar x, SkScalar y, sk_app::Window::InputState state,
                 uint32_t modifiers) override;

private:
    void resetActor();

    void renderGUI();

private:
    std::string                fBasePath;
    std::unique_ptr<NIMAActor> fActor;

    bool fPlaying;
    float fTime;
    int fBackend;

    nima::ActorAnimationInstance* fAnimation;
    int                           fAnimationIndex;
};

#endif
