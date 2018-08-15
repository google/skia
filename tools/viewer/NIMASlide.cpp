/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "NIMASlide.h"

#include "SkAnimTimer.h"
#include "SkOSPath.h"
#include "Resources.h"
#include "imgui.h"

#include <algorithm>
#include <cmath>

using namespace sk_app;
using namespace nima;

// ImGui expects an array of const char* when displaying a ListBox. This function is for an
// overload of ImGui::ListBox that takes a getter so that ListBox works with
// std::vector<std::string>.
static bool vector_getter(void* v, int index, const char** out) {
    auto vector = reinterpret_cast<std::vector<std::string>*>(v);
    *out = vector->at(index).c_str();
    return true;
}

// A wrapper class that handles rendering of ActorImages (renderable components NIMA Actors).
class NIMAActorImage {
public:
    NIMAActorImage(ActorImage* actorImage, SkImage* texture, SkPaint* paint)
            : fActorImage(actorImage)
            , fTexture(texture)
            , fPaint(paint)
            , fSkinned(false)
            , fPositions()
            , fTexs()
            , fBoneIdx()
            , fBoneWgt()
            , fIndices()
            , fBones()
            , fVertices(nullptr)
            , fRenderFlags(0) {
        // Update the vertices and bones.
        this->updateVertices(true);
        this->updateBones();
    }

    void render(SkCanvas* canvas, uint32_t renderFlags) {
        bool dirty = renderFlags != fRenderFlags;
        fRenderFlags = renderFlags;

        bool useImmediate = renderFlags & kImmediate_RenderFlag;
        bool useCache = renderFlags & kCache_RenderFlag;
        bool drawBounds = renderFlags & kBounds_RenderFlag;

        // Don't use the cache if drawing in immediate mode.
        useCache &= !useImmediate;

        if (fActorImage->doesAnimationVertexDeform() || dirty) {
            // These are vertices that transform beyond just bone transforms, so they must be
            // updated every frame.
            // If the render flags are dirty, reset the vertices object.
            this->updateVertices(!useCache);
        }

        // Update the bones.
        this->updateBones();

        // Deform the bones in immediate mode.
        sk_sp<SkVertices> vertices = fVertices;
        if (useImmediate) {
            vertices = fVertices->applyBones(fBones.data(), fBones.size());
        }

        // Draw the vertices object.
        this->drawVerticesObject(vertices.get(), canvas, !useImmediate);

        // Draw the bounds.
        if (drawBounds && fActorImage->renderOpacity() > 0.0f) {
            // Get the bounds.
            SkRect bounds = vertices->bounds();

            // Approximate bounds if not using immediate transforms.
            if (!useImmediate) {
                const SkRect originalBounds = fBones[0].mapRect(vertices->bounds());
                bounds = originalBounds;
                for (size_t i = 1; i < fBones.size(); i++) {
                    const SkVertices::Bone& matrix = fBones[i];
                    bounds.join(matrix.mapRect(originalBounds));
                }
            }

            // Draw the bounds.
            SkPaint paint;
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setColor(0xFFFF0000);
            canvas->drawRect(bounds, paint);
        }
    }

    int drawOrder() const { return fActorImage->drawOrder(); }

private:
    void updateVertices(bool isVolatile) {
        // Update whether the image is skinned.
        fSkinned = fActorImage->connectedBoneCount() > 0;

        // Retrieve data from the image.
        uint32_t  vertexCount  = fActorImage->vertexCount();
        uint32_t  vertexStride = fActorImage->vertexStride();
        float*    vertexData   = fActorImage->vertices();
        uint32_t  indexCount   = fActorImage->triangleCount() * 3;
        uint16_t* indexData    = fActorImage->triangles();

        // Don't render if not visible.
        if (!vertexCount || fActorImage->textureIndex() < 0) {
            fPositions.clear();
            fTexs.clear();
            fBoneIdx.clear();
            fBoneWgt.clear();
            fIndices.clear();
            return;
        }

        // Split the vertex data.
        fPositions.resize(vertexCount);
        fTexs.resize(vertexCount);
        fIndices.resize(indexCount);
        if (fSkinned) {
            fBoneIdx.resize(vertexCount * 4);
            fBoneWgt.resize(vertexCount * 4);
        }
        for (uint32_t i = 0; i < vertexCount; i ++) {
            uint32_t j = i * vertexStride;

            // Get the attributes.
            float* attrPosition = vertexData + j;
            float* attrTex      = vertexData + j + 2;
            float* attrBoneIdx  = vertexData + j + 4;
            float* attrBoneWgt  = vertexData + j + 8;

            // Get deformed positions if necessary.
            if (fActorImage->doesAnimationVertexDeform()) {
                attrPosition = fActorImage->animationDeformedVertices() + i * 2;
            }

            // Set the data.
            fPositions[i].set(attrPosition[0], attrPosition[1]);
            fTexs[i].set(attrTex[0] * fTexture->width(), attrTex[1] * fTexture->height());
            if (fSkinned) {
                for (uint32_t k = 0; k < 4; k ++) {
                    fBoneIdx[i][k] = static_cast<uint32_t>(attrBoneIdx[k]);
                    fBoneWgt[i][k] = attrBoneWgt[k];
                }
            }
        }
        memcpy(fIndices.data(), indexData, indexCount * sizeof(uint16_t));

        // Update the vertices object.
        fVertices = SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode,
                                         vertexCount,
                                         fPositions.data(),
                                         fTexs.data(),
                                         nullptr,
                                         fBoneIdx.data(),
                                         fBoneWgt.data(),
                                         fIndices.size(),
                                         fIndices.data(),
                                         isVolatile);
    }

    void updateBones() {
        // NIMA matrices are a collection of 6 floats.
        constexpr int kNIMAMatrixSize = 6;

        // Set up the matrices for the first time.
        if (fBones.size() == 0) {
            int numMatrices = 1;
            if (fSkinned) {
                numMatrices = fActorImage->boneInfluenceMatricesLength() / kNIMAMatrixSize;
            }

            // Initialize all matrices to the identity matrix.
            fBones.assign(numMatrices, {{ 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f }});
        }

        if (fSkinned) {
            // Update the matrices.
            float* matrixData = fActorImage->boneInfluenceMatrices();
            memcpy(fBones.data(), matrixData, fBones.size() * kNIMAMatrixSize * sizeof(float));
        }

        // Set the zero matrix to be the world transform.
        memcpy(fBones.data(),
               fActorImage->worldTransform().values(),
               kNIMAMatrixSize * sizeof(float));
    }

    void drawVerticesObject(SkVertices* vertices, SkCanvas* canvas, bool useBones) const {
        // Determine the blend mode.
        SkBlendMode blendMode;
        switch (fActorImage->blendMode()) {
            case BlendMode::Off: {
                blendMode = SkBlendMode::kSrc;
                break;
            }
            case BlendMode::Normal: {
                blendMode = SkBlendMode::kSrcOver;
                break;
            }
            case BlendMode::Additive: {
                blendMode = SkBlendMode::kPlus;
                break;
            }
            case BlendMode::Multiply: {
                blendMode = SkBlendMode::kMultiply;
                break;
            }
            case BlendMode::Screen: {
                blendMode = SkBlendMode::kScreen;
                break;
            }
        }

        // Set the opacity.
        fPaint->setAlpha(static_cast<U8CPU>(fActorImage->renderOpacity() * 255));

        // Draw the vertices.
        if (useBones) {
            canvas->drawVertices(vertices, fBones.data(), fBones.size(), blendMode, *fPaint);
        } else {
            canvas->drawVertices(vertices, blendMode, *fPaint);
        }

        // Reset the opacity.
        fPaint->setAlpha(255);
    }

private:
    ActorImage* fActorImage;
    SkImage*    fTexture;
    SkPaint*    fPaint;

    bool                                 fSkinned;
    std::vector<SkPoint>                 fPositions;
    std::vector<SkPoint>                 fTexs;
    std::vector<SkVertices::BoneIndices> fBoneIdx;
    std::vector<SkVertices::BoneWeights> fBoneWgt;
    std::vector<uint16_t>                fIndices;

    std::vector<SkVertices::Bone> fBones;
    sk_sp<SkVertices>             fVertices;

    uint32_t fRenderFlags;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

// Represents an Actor, or an animated character, in NIMA.
class NIMAActor : public Actor {
public:
    NIMAActor(const std::string& basePath)
        : fTexture(nullptr)
        , fActorImages()
        , fPaint()
        , fAnimations() {
        // Load the NIMA data.
        std::string nimaPath((basePath + ".nima").c_str());
        INHERITED::load(nimaPath);

        // Load the image asset.
        sk_sp<SkData> imageData = SkData::MakeFromFileName((basePath + ".png").c_str());
        fTexture = SkImage::MakeFromEncoded(imageData);

        // Create the paint.
        fPaint.setShader(fTexture->makeShader(nullptr));
        fPaint.setFilterQuality(SkFilterQuality::kLow_SkFilterQuality);

        // Load the image nodes.
        fActorImages.reserve(m_ImageNodeCount);
        for (uint32_t i = 0; i < m_ImageNodeCount; i ++) {
            fActorImages.emplace_back(m_ImageNodes[i], fTexture.get(), &fPaint);
        }

        // Sort the image nodes.
        std::sort(fActorImages.begin(), fActorImages.end(), [](auto a, auto b) {
            return a.drawOrder() < b.drawOrder();
        });

        // Get the list of animations.
        fAnimations.reserve(m_AnimationsCount);
        for (uint32_t i = 0; i < m_AnimationsCount; i ++) {
            fAnimations.push_back(m_Animations[i].name());
        }
    }

    void render(SkCanvas* canvas, uint32_t renderFlags) {
        // Render the image nodes.
        for (auto& image : fActorImages) {
            image.render(canvas, renderFlags);
        }
    }

    const std::vector<std::string>& getAnimations() const {
        return fAnimations;
    }

private:
    sk_sp<SkImage>              fTexture;
    std::vector<NIMAActorImage> fActorImages;
    SkPaint                     fPaint;
    std::vector<std::string>    fAnimations;

    typedef Actor INHERITED;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

NIMASlide::NIMASlide(const SkString& name, const SkString& path)
        : fBasePath()
        , fActor(nullptr)
        , fPlaying(true)
        , fTime(0.0f)
        , fRenderFlags(0)
        , fAnimation(nullptr)
        , fAnimationIndex(0) {
    fName = name;

    // Get the path components.
    SkString baseName = SkOSPath::Basename(path.c_str());
    baseName.resize(baseName.size() - 5);
    SkString dirName = SkOSPath::Dirname(path.c_str());
    SkString basePath = SkOSPath::Join(dirName.c_str(), baseName.c_str());

    // Save the base path.
    fBasePath = std::string(basePath.c_str());
}

NIMASlide::~NIMASlide() {}

void NIMASlide::draw(SkCanvas* canvas) {
    canvas->save();

    for (int i = 0; i < 10; i ++) {
        for (int j = 0; j < 10; j ++) {
            canvas->save();

            canvas->translate(1250 - 250 * i, 1250 - 250 * j);
            canvas->scale(0.5, -0.5);

            // Render the actor.
            fActor->render(canvas, fRenderFlags);

            canvas->restore();
        }
    }

    canvas->restore();

    // Render the GUI.
    this->renderGUI();
}

void NIMASlide::load(SkScalar winWidth, SkScalar winHeight) {
    this->resetActor();
}

void NIMASlide::unload() {
    // Discard resources.
    fAnimation = nullptr;
    fActor.reset(nullptr);
}

bool NIMASlide::animate(const SkAnimTimer& timer) {
    // Apply the animation.
    if (fAnimation) {
        if (fPlaying) {
            fTime = std::fmod(timer.secs(), fAnimation->max());
        }
        fAnimation->time(fTime);
        fAnimation->apply(1.0f);
    }
    return true;
}

bool NIMASlide::onChar(SkUnichar c) {
    return false;
}

bool NIMASlide::onMouse(SkScalar x, SkScalar y, Window::InputState state, uint32_t modifiers) {
    return false;
}

void NIMASlide::resetActor() {
    // Create the actor.
    fActor = std::make_unique<NIMAActor>(fBasePath);

    // Get the animation.
    fAnimation = fActor->animationInstance(fActor->getAnimations()[fAnimationIndex]);
}

void NIMASlide::renderGUI() {
    ImGui::SetNextWindowSize(ImVec2(300, 0));
    ImGui::Begin("NIMA");

    // List of animations.
    auto animations = const_cast<std::vector<std::string>&>(fActor->getAnimations());
    ImGui::PushItemWidth(-1);
    if (ImGui::ListBox("Animations",
                       &fAnimationIndex,
                       vector_getter,
                       reinterpret_cast<void*>(&animations),
                       animations.size(),
                       5)) {
        resetActor();
    }

    // Playback control.
    ImGui::Spacing();
    if (ImGui::Button("Play")) {
        fPlaying = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Pause")) {
        fPlaying = false;
    }

    // Time slider.
    ImGui::PushItemWidth(-1);
    ImGui::SliderFloat("Time", &fTime, 0.0f, fAnimation->max(), "Time: %.3f");

    // Backend control.
    int useImmediate = SkToBool(fRenderFlags & kImmediate_RenderFlag);
    ImGui::Spacing();
    ImGui::RadioButton("Skia Backend", &useImmediate, 0);
    ImGui::RadioButton("Immediate Backend", &useImmediate, 1);
    if (useImmediate) {
        fRenderFlags |= kImmediate_RenderFlag;
    } else {
        fRenderFlags &= ~kImmediate_RenderFlag;
    }

    // Cache control.
    bool useCache = SkToBool(fRenderFlags & kCache_RenderFlag);
    ImGui::Spacing();
    ImGui::Checkbox("Cache Vertices", &useCache);
    if (useCache) {
        fRenderFlags |= kCache_RenderFlag;
    } else {
        fRenderFlags &= ~kCache_RenderFlag;
    }

    // Bounding box toggle.
    bool drawBounds = SkToBool(fRenderFlags & kBounds_RenderFlag);
    ImGui::Spacing();
    ImGui::Checkbox("Draw Bounds", &drawBounds);
    if (drawBounds) {
        fRenderFlags |= kBounds_RenderFlag;
    } else {
        fRenderFlags &= ~kBounds_RenderFlag;
    }

    ImGui::End();
}
