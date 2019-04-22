/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/nima/NimaActor.h"

#include "include/core/SkData.h"
#include "include/core/SkFilterQuality.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkString.h"
#include "include/core/SkVertices.h"

#include <algorithm>
#include <cmath>

NimaActor::NimaActor(std::string nimaPath, std::string texturePath)
    : fTexture(nullptr)
    , fActorImages()
    , fPaint(nullptr)
    , fAnimationNames()
    , fAnimationInstance(nullptr) {
    // Load the NIMA data.
    INHERITED::load(nimaPath);

    // Load the image asset.
    fTexture = SkImage::MakeFromEncoded(SkData::MakeFromFileName(texturePath.c_str()));

    this->init();
}

NimaActor::NimaActor(sk_sp<SkData> nimaBytes, sk_sp<SkData> textureBytes)
    : fTexture(nullptr)
    , fActorImages()
    , fPaint(nullptr)
    , fAnimationNames()
    , fAnimationInstance(nullptr) {
    // Load the NIMA data.
    INHERITED::load(const_cast<uint8_t*>(nimaBytes->bytes()), nimaBytes->size());

    // Load the image asset.
    fTexture = SkImage::MakeFromEncoded(textureBytes);

    this->init();
}

void NimaActor::init() {
    // Create the paint.
    fPaint = std::make_unique<SkPaint>();
    fPaint->setShader(fTexture->makeShader(nullptr));
    fPaint->setFilterQuality(SkFilterQuality::kLow_SkFilterQuality);

    // Load the image nodes.
    fActorImages.reserve(m_ImageNodeCount);
    for (uint32_t i = 0; i < m_ImageNodeCount; i ++) {
        fActorImages.emplace_back(m_ImageNodes[i], fTexture.get(), fPaint.get());
    }

    // Sort the image nodes.
    std::sort(fActorImages.begin(), fActorImages.end(), [](auto a, auto b) {
        return a.drawOrder() < b.drawOrder();
    });

    // Get the list of animations.
    fAnimationNames.reserve(m_AnimationsCount);
    for (uint32_t i = 0; i < m_AnimationsCount; i++) {
        fAnimationNames.push_back(m_Animations[i].name());
    }
    this->setAnimation(0);
}

SkScalar NimaActor::duration() const {
    if (fAnimationInstance) {
        return fAnimationInstance->duration();
    }
    return 0.0f;
}

void NimaActor::setAnimation(uint8_t index) {
    if (index < fAnimationNames.size()) {
        fAnimationIndex = index;
        fAnimationInstance = this->animationInstance(fAnimationNames[fAnimationIndex]);
    }
}

void NimaActor::setAnimation(std::string name) {
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

void NimaActor::render(SkCanvas* canvas, uint32_t renderFlags) {
    // Render the image nodes.
    for (auto& image : fActorImages) {
        image.render(canvas, renderFlags);
    }
}

void NimaActor::seek(SkScalar t) {
    // Apply the animation.
    if (fAnimationInstance) {
        t = std::fmod(t, fAnimationInstance->max());
        fAnimationInstance->time(t);
        fAnimationInstance->apply(1.0f);
    }
}

// ===================================================================================

NimaActorImage::NimaActorImage(nima::ActorImage* actorImage, SkImage* texture, SkPaint* paint)
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

void NimaActorImage::render(SkCanvas* canvas, uint32_t renderFlags) {
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

void NimaActorImage::updateVertices(bool isVolatile) {
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

void NimaActorImage::updateBones() {
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

void NimaActorImage::drawVerticesObject(SkVertices* vertices, SkCanvas* canvas, bool useBones) const {
    // Determine the blend mode.
    SkBlendMode blendMode;
    switch (fActorImage->blendMode()) {
        case nima::BlendMode::Off: {
            blendMode = SkBlendMode::kSrc;
            break;
        }
        case nima::BlendMode::Normal: {
            blendMode = SkBlendMode::kSrcOver;
            break;
        }
        case nima::BlendMode::Additive: {
            blendMode = SkBlendMode::kPlus;
            break;
        }
        case nima::BlendMode::Multiply: {
            blendMode = SkBlendMode::kMultiply;
            break;
        }
        case nima::BlendMode::Screen: {
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
