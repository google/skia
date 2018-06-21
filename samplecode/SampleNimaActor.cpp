/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleNimaActor.h"

#include "SkString.h"
#include "SkVertices.h"
#include "SkPaint.h"
#include "SkFilterQuality.h"
#include "Resources.h"
#include <algorithm>

using namespace nima;

SampleActor::SampleActor(std::string baseName)
    : fTexture(nullptr)
    , fActorImages()
    , fPaint(nullptr) {
    // Load the NIMA data.
    SkString nimaSkPath = GetResourcePath(("nima/" + baseName + ".nima").c_str());
    std::string nimaPath(nimaSkPath.c_str());
    INHERITED::load(nimaPath);

    // Load the image asset.
    fTexture = GetResourceAsImage(("nima/" + baseName + ".png").c_str());

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
}

SampleActor::~SampleActor() {
}

void SampleActor::render(SkCanvas* canvas) const {
    // Render the image nodes.
    for (auto image : fActorImages) {
        image.render(this, canvas);
    }
}

SampleActorImage::SampleActorImage(ActorImage* actorImage, sk_sp<SkImage> texture, SkPaint* paint)
    : fActorImage(actorImage)
    , fTexture(texture)
    , fPaint(paint) {
}

SampleActorImage::~SampleActorImage() {
}

void SampleActorImage::render(const SampleActor* actor, SkCanvas* canvas) const {
    // Retrieve data from the image.
    uint32_t  vertexCount  = fActorImage->vertexCount();
    uint32_t  vertexStride = fActorImage->vertexStride();
    float*    vertexData   = fActorImage->vertices();
    uint32_t  indexCount   = fActorImage->triangleCount() * 3;
    uint16_t* indexData    = fActorImage->triangles();

    // Don't render if not visible.
    if (!vertexCount || fActorImage->textureIndex() < 0) {
        return;
    }

    // Split the vertex data.
    std::vector<SkPoint>  positions(vertexCount);
    std::vector<SkPoint>  texs(vertexCount);
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

        // Deform the position.
        Vec2D position(attrPosition[0], attrPosition[1]);
        if (fActorImage->connectedBoneCount() > 0) {
            position = deform(position, attrBoneIdx, attrBoneWgt);
        } else {
            position = deform(position, nullptr, nullptr);
        }

        // Set the data.
        positions[i].set(position[0], position[1]);
        texs[i].set(attrTex[0] * fTexture->width(), attrTex[1] * fTexture->height());
    }

    // Create vertices.
    sk_sp<SkVertices> vertices = SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode,
                                                      vertexCount,
                                                      positions.data(),
                                                      texs.data(),
                                                      nullptr,
                                                      indexCount,
                                                      indexData);

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
    canvas->drawVertices(vertices, blendMode, *fPaint);

    // Reset the opacity.
    fPaint->setAlpha(255);
}

Vec2D SampleActorImage::deform(const Vec2D& position, float* boneIdx, float* boneWgt) const {
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
            int index = static_cast<int>(boneIdx[i]);
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
