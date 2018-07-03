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

// NIMA stores its matrices as 6 floats to represent translation and scale. This function takes
// that format and converts it into a 3x3 matrix representation.
static void nima_to_skmatrix(const float* nimaData, SkMatrix& matrix) {
    matrix[0] = nimaData[0];
    matrix[1] = nimaData[2];
    matrix[2] = nimaData[4];
    matrix[3] = nimaData[1];
    matrix[4] = nimaData[3];
    matrix[5] = nimaData[5];
    matrix[6] = 0.0f;
    matrix[7] = 0.0f;
    matrix[8] = 1.0f;
}

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
            , fRenderMode(kBackend_RenderMode) {
        // Update the vertices and bones.
        this->updateVertices();
        this->updateBones();

        // Update the vertices object.
        this->updateVerticesObject(false, false);
    }

    void renderBackend(SkCanvas* canvas) {
        // Reset vertices if the render mode has changed.
        if (fRenderMode != kBackend_RenderMode) {
            fRenderMode = kBackend_RenderMode;
            this->updateVertices();
            this->updateVerticesObject(false, false);
        }

        // Update the vertex data.
        if (fActorImage->doesAnimationVertexDeform()) {
            this->updateVertices();
            this->updateVerticesObject(false, true);
        }

        // Update the bones.
        this->updateBones();

        // Draw the vertices object.
        this->drawVerticesObject(canvas, true);
    }

    void renderImmediate(SkCanvas* canvas) {
        // Reset vertices if the render mode has changed.
        if (fRenderMode != kImmediate_RenderMode) {
            fRenderMode = kImmediate_RenderMode;
            this->updateVertices();
            this->updateVerticesObject(true, true);
        }

        // Update the vertex data.
        if (fActorImage->doesAnimationVertexDeform() && fActorImage->isVertexDeformDirty()) {
            this->updateVertices();
            fActorImage->isVertexDeformDirty(false);
        }

        // Update the vertices object.
        this->updateVerticesObject(true, true);

        // Draw the vertices object.
        this->drawVerticesObject(canvas, false);
    }

    int drawOrder() const { return fActorImage->drawOrder(); }

private:
    void updateVertices() {
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
                    fBoneIdx[i].indices[k] = static_cast<uint32_t>(attrBoneIdx[k]);
                    fBoneWgt[i].weights[k] = attrBoneWgt[k];
                }
            }
        }
        memcpy(fIndices.data(), indexData, indexCount * sizeof(uint16_t));
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
            fBones.assign(numMatrices, SkMatrix());
        }

        if (fSkinned) {
            // Update the matrices.
            float* matrixData = fActorImage->boneInfluenceMatrices();
            for (uint32_t i = 1; i < fBones.size(); i ++) {
                SkMatrix& matrix = fBones[i];
                float* data = matrixData + i * kNIMAMatrixSize;
                nima_to_skmatrix(data, matrix);
            }
        }

        // Set the zero matrix to be the world transform.
        nima_to_skmatrix(fActorImage->worldTransform().values(), fBones[0]);
    }

    void updateVerticesObject(bool applyDeforms, bool isVolatile) {
        std::vector<SkPoint>* positions = &fPositions;

        // Apply deforms if requested.
        uint32_t vertexCount = fPositions.size();
        std::vector<SkPoint> deformedPositions;
        if (applyDeforms) {
            positions = &deformedPositions;
            deformedPositions.reserve(vertexCount);
            for (uint32_t i = 0; i < vertexCount; i ++) {
                Vec2D nimaPoint(fPositions[i].x(), fPositions[i].y());
                uint32_t* boneIdx = nullptr;
                float* boneWgt = nullptr;
                if (fSkinned) {
                    boneIdx = fBoneIdx[i].indices;
                    boneWgt = fBoneWgt[i].weights;
                }
                nimaPoint = this->deform(nimaPoint, boneIdx, boneWgt);
                deformedPositions.push_back(SkPoint::Make(nimaPoint[0], nimaPoint[1]));
            }
        }

        // Update the vertices object.
        fVertices = SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode,
                                         vertexCount,
                                         positions->data(),
                                         fTexs.data(),
                                         nullptr,
                                         fBoneIdx.data(),
                                         fBoneWgt.data(),
                                         fIndices.size(),
                                         fIndices.data(),
                                         isVolatile);
    }

    void drawVerticesObject(SkCanvas* canvas, bool useBones) const {
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
            canvas->drawVertices(fVertices, fBones.data(), fBones.size(), blendMode, *fPaint);
        } else {
            canvas->drawVertices(fVertices, blendMode, *fPaint);
        }

        // Reset the opacity.
        fPaint->setAlpha(255);
    }

    Vec2D deform(const Vec2D& position, uint32_t* boneIdx, float* boneWgt) const {
        float px = position[0], py = position[1];
        float px2 = px, py2 = py;
        float influence[6] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

        // Apply the world transform.
        Mat2D worldTransform = fActorImage->worldTransform();
        px2 = worldTransform[0] * px + worldTransform[2] * py + worldTransform[4];
        py2 = worldTransform[1] * px + worldTransform[3] * py + worldTransform[5];

        // Apply deformations based on bone offsets.
        if (boneIdx && boneWgt) {
            float* matrices = fActorImage->boneInfluenceMatrices();

            for (uint32_t i = 0; i < 4; i ++) {
                uint32_t index = boneIdx[i];
                float weight = boneWgt[i];
                for (int j = 0; j < 6; j ++) {
                    influence[j] += matrices[index * 6 + j] * weight;
                }
            }

            px = influence[0] * px2 + influence[2] * py2 + influence[4];
            py = influence[1] * px2 + influence[3] * py2 + influence[5];
        } else {
            px = px2;
            py = py2;
        }

        // Return the transformed position.
        return Vec2D(px, py);
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

    std::vector<SkMatrix> fBones;
    sk_sp<SkVertices>     fVertices;

    RenderMode fRenderMode;
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

    void render(SkCanvas* canvas, RenderMode renderMode) {
        // Render the image nodes.
        for (auto& image : fActorImages) {
            switch (renderMode) {
            case kBackend_RenderMode: {
                // Render with Skia backend.
                image.renderBackend(canvas);
                break;
            }
            case kImmediate_RenderMode: {
                // Render with immediate backend.
                image.renderImmediate(canvas);
                break;
            }
            }
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
        , fRenderMode(kBackend_RenderMode)
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
            fActor->render(canvas, fRenderMode);

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
    ImGui::SetNextWindowSize(ImVec2(300, 220));
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
    int renderMode = fRenderMode;
    ImGui::Spacing();
    ImGui::RadioButton("Skia Backend", &renderMode, 0);
    ImGui::RadioButton("Immediate Backend", &renderMode, 1);
    if (renderMode == 0) {
        fRenderMode = kBackend_RenderMode;
    } else {
        fRenderMode = kImmediate_RenderMode;
    }

    ImGui::End();
}
