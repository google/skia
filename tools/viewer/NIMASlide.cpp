/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "NIMASlide.h"

#include "SkAnimTimer.h"
#include "SkVertices.h"
#include "SkOSPath.h"
#include "Resources.h"
#include "imgui.h"

#include <algorithm>

#include <iostream>

using namespace sk_app;
using namespace nima;

static void nimaToSkMatrix(const float* nimaData, SkMatrix& matrix) {
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

static bool vectorGetter(void* v, int index, const char** out) {
    auto vector = reinterpret_cast<std::vector<std::string>*>(v);
    *out = vector->at(index).c_str();
    return true;
}

NIMAActor::NIMAActor(const std::string& basePath)
    : fTexture(nullptr)
    , fActorImages()
    , fPaint(nullptr)
    , fAnimations() {
    // Load the NIMA data.
    std::string nimaPath((basePath + ".nima").c_str());
    INHERITED::load(nimaPath);

    // Load the image asset.
    sk_sp<SkData> imageData = SkData::MakeFromFileName((basePath + ".png").c_str());
    fTexture = SkImage::MakeFromEncoded(imageData);

    // Create the paint.
    fPaint = std::make_unique<SkPaint>();
    fPaint->setShader(fTexture->makeShader(nullptr));
    fPaint->setFilterQuality(SkFilterQuality::kLow_SkFilterQuality);

    // Load the image nodes.
    fActorImages.reserve(m_ImageNodeCount);
    for (uint32_t i = 0; i < m_ImageNodeCount; i ++) {
        fActorImages.emplace_back(m_ImageNodes[i], fTexture, fPaint.get());
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

NIMAActor::~NIMAActor() {}

void NIMAActor::render(SkCanvas* canvas, int backend) const {
    // Render the image nodes.
    for (auto image : fActorImages) {
        if (backend == 0) {
            // Render with Skia backend.
            image.renderBackend(canvas);
        } else {
            // Render with immediate backend.
            image.renderImmediate(canvas);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////

NIMAActorImage::NIMAActorImage(ActorImage* actorImage, sk_sp<SkImage> texture, SkPaint* paint)
    : fActorImage(actorImage)
    , fTexture(texture)
    , fPaint(paint)
    , fSkinned(false)
    , fPositions()
    , fTexs()
    , fAttachments()
    , fBoneIdx()
    , fBoneWgt()
    , fIndices()
    , fBones()
    , fVertices(nullptr)
    , fSkeleton(SkSkeleton::Make())
    , fRenderMode(0) {
    // Update the vertices and skeleton.
    updateVertices();
    updateSkeleton();

    // Update the vertices object.
    updateVerticesObject(false);
}

NIMAActorImage::~NIMAActorImage() {}

void NIMAActorImage::renderBackend(SkCanvas* canvas) {
    // Reset vertices if the render mode has changed.
    if (fRenderMode != 0) {
        fRenderMode = 0;
        updateVertices();
        updateVerticesObject(false);
    }

    canvas->save();

    // Update the vertex data.
    if (fActorImage->doesAnimationVertexDeform() && fActorImage->isVertexDeformDirty()) {
        updateVertices();
        updateVerticesObject(false);
        fActorImage->isVertexDeformDirty(false);
    }

    // Update the skeleton.
    updateSkeleton();

    // Draw the vertices object.
    drawVerticesObject(canvas, true);

    canvas->restore();
}

void NIMAActorImage::renderImmediate(SkCanvas* canvas) {
    // Reset vertices if the render mode has changed.
    if (fRenderMode != 1) {
        fRenderMode = 1;
        updateVertices();
        updateVerticesObject(true);
    }

    // Update the vertex data.
    if (fActorImage->doesAnimationVertexDeform() && fActorImage->isVertexDeformDirty()) {
        updateVertices();
        fActorImage->isVertexDeformDirty(false);
    }

    // Update the vertices object.
    updateVerticesObject(true);

    // Draw the vertices object.
    drawVerticesObject(canvas, false);
}

void NIMAActorImage::updateVertices() {
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
        fAttachments.clear();
        fIndices.clear();
        return;
    }

    // Split the vertex data.
    fPositions.resize(vertexCount);
    fTexs.resize(vertexCount);
    fIndices.resize(indexCount);
    if (fSkinned) {
        fAttachments.resize(vertexCount * 4);
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
                int   idx = static_cast<int>(attrBoneIdx[k]);
                float wgt = attrBoneWgt[k];
                fAttachments[i * 4 + k] = { idx, wgt };
                fBoneIdx[i * 4 + k] = idx;
                fBoneWgt[i * 4 + k] = wgt;
            }
        }
    }
    memcpy(fIndices.data(), indexData, indexCount * sizeof(uint16_t));
}

void NIMAActorImage::updateSkeleton() {
    // Set up the matrices for the first time.
    if (fBones.size() == 0) {
        int numMatrices = 1;
        if (fSkinned) {
            numMatrices = fActorImage->boneInfluenceMatricesLength() / 6;
        }
        fBones.assign(numMatrices, SkMatrix());
    }

    if (fSkinned) {
        // Update the matrices.
        float* matrixData = fActorImage->boneInfluenceMatrices();
        for (uint32_t i = 1; i < fBones.size(); i ++) {
            SkMatrix& matrix = fBones[i];
            float* data = matrixData + i * 6;
            nimaToSkMatrix(data, matrix);
        }
    }

    // Set the zero matrix to be the world transform.
    nimaToSkMatrix(fActorImage->worldTransform().values(), fBones[0]);

    // Update the skeleton.
    fSkeleton->setBones(fBones.data(),
                        fBones.size());
}

void NIMAActorImage::updateVerticesObject(bool applyDeforms) {
    std::vector<SkPoint>* usePositions = &fPositions;

    // Apply deforms if requested.
    uint32_t vertexCount = fPositions.size();
    std::vector<SkPoint> deformedPositions;
    if (applyDeforms) {
        usePositions = &deformedPositions;
        deformedPositions.reserve(vertexCount);
        for (uint32_t i = 0; i < vertexCount; i ++) {
            Vec2D nimaPoint(fPositions[i].x(), fPositions[i].y());
            int* boneIdx = nullptr;
            float* boneWgt = nullptr;
            if (fSkinned) {
                boneIdx = fBoneIdx.data() + i * 4;
                boneWgt = fBoneWgt.data() + i * 4;
            }
            nimaPoint = deform(nimaPoint, boneIdx, boneWgt);
            deformedPositions.push_back(SkPoint::Make(nimaPoint[0], nimaPoint[1]));
        }
    }

    // Update the vertices object.
    fVertices = SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode,
                                     vertexCount,
                                     usePositions->data(),
                                     fTexs.data(),
                                     nullptr,
                                     fAttachments.data(),
                                     fIndices.size(),
                                     fIndices.data());
}

void NIMAActorImage::drawVerticesObject(SkCanvas* canvas, bool useSkeleton) const {
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
    if (useSkeleton) {
        canvas->drawVertices(fVertices, fSkeleton, blendMode, *fPaint);
    } else {
        canvas->drawVertices(fVertices, blendMode, *fPaint);
    }

    // Reset the opacity.
    fPaint->setAlpha(255);
}

Vec2D NIMAActorImage::deform(const Vec2D& position, int* boneIdx, float* boneWgt) const {
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
            int index = boneIdx[i];
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

//////////////////////////////////////////////////////////////////////////////////////////////////

NIMASlide::NIMASlide(const SkString& name, const SkString& path)
    : fBasePath()
    , fActor(nullptr)
    , fPlaying(true)
    , fTime(0.0f)
    , fBackend(0)
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

    canvas->translate(500, 500);
    canvas->scale(1, -1);

    // Render the actor.
    fActor->render(canvas, fBackend);

    canvas->restore();

    // Render the GUI.
    renderGUI();
}

void NIMASlide::load(SkScalar winWidth, SkScalar winHeight) {
    resetActor();
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
                       vectorGetter,
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
    ImGui::Spacing();
    ImGui::RadioButton("Skia Backend", &fBackend, 0);
    ImGui::RadioButton("Immediate Backend", &fBackend, 1);

    ImGui::End();
}
