/*
 * Copyright 2017 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <SkSurface.h>
#include <SkCanvas.h>
#include <SkVertices.h>
#include "gl/GrGLTypes.h"
#include "plane_renderer.h"
#include "util.h"
#include "SkImage.h"
#include "GrBackendSurface.h"
#include "SkPicture.h"
#include "SkStream.h"

namespace hello_ar {
    namespace {
        constexpr float kPlaneTexScale = 0.0005f;
        constexpr U8CPU kPlaneAlpha = 100;
        constexpr int32_t kPlaneColorRgbaSize = 6;
        constexpr std::array<SkColor, kPlaneColorRgbaSize> kPlaneColorRgba = {
                {SkColorSetARGB(kPlaneAlpha, 255, 0, 0), SkColorSetARGB(kPlaneAlpha, 0, 255, 0),
                SkColorSetARGB(kPlaneAlpha, 0, 0, 255), SkColorSetARGB(kPlaneAlpha, 255, 255, 0),
                SkColorSetARGB(kPlaneAlpha, 0, 255, 255), SkColorSetARGB(kPlaneAlpha, 255, 0, 255)}
        };

        inline SkColor GetRandomPlaneColor() {
            const SkColor colorRgba = kPlaneColorRgba[std::rand() % kPlaneColorRgbaSize];
            return colorRgba;
        }
    }  // namespace

    void PlaneRenderer::InitializeGlContent(AAssetManager *asset_manager) {
        //Open asset texture: get buffer + size
        AAsset* a = AAssetManager_open(asset_manager, "models/trigrid.png", AASSET_MODE_UNKNOWN);
        const void* buffer = AAsset_getBuffer(a);
        size_t size = (size_t) AAsset_getLength(a);

        sk_sp<SkData> data = SkData::MakeWithCopy(buffer, size);
        sk_sp<SkImage> image = SkImage::MakeFromEncoded(data, nullptr);

        //Set up shader
        SkMatrix textureScale;
        textureScale.setScale(kPlaneTexScale, kPlaneTexScale);
        shader_ = image->makeShader(SkShader::TileMode::kRepeat_TileMode, SkShader::TileMode::kRepeat_TileMode,
                                    &textureScale);
        //Close asset
        AAsset_close(a);
    }

    void PlaneRenderer::Draw(ArSession* arSession, ArFrame* arFrame, GrContext* context, SkSurface* surface,
                             SkMatrix44& vpv, int& planeCount) {
        ArTrackableList* planeList = nullptr;
        ArTrackableList_create(arSession, &planeList);
        CHECK(planeList != nullptr);
        ArSession_getAllTrackables(arSession, AR_TRACKABLE_PLANE, planeList);

        int32_t planeListSize = 0;
        ArTrackableList_getSize(arSession, planeList, &planeListSize);

        for (int i = 0; i < planeListSize; ++i) {
            ArTrackable* trackable = nullptr;
            ArTrackableList_acquireItem(arSession, planeList, i, &trackable);
            ArPlane *arPlane = ArAsPlane(trackable);
            ArTrackingState ouTrackingState;
            ArTrackable_getTrackingState(arSession, trackable,
                                         &ouTrackingState);

            //Check if plane is subsumed by another one. If so, skip plane
            ArPlane* subsumePlane;
            ArPlane_acquireSubsumedBy(arSession, arPlane, &subsumePlane);
            if (subsumePlane != nullptr) {
                ArTrackable_release(ArAsTrackable(subsumePlane));
                continue;
            }

            //Check if plane isn't tracking. If so, skip plane
            if (ArTrackingState::AR_TRACKING_STATE_TRACKING != ouTrackingState) {
                continue;
            }

            //Check if need to cull plane (behind camera). If so, skip plane
            /*if (CullPlane(arSession, arFrame, ar_plane)) {
                continue;
            }*/

            ArTrackingState planeTrackingState;
            ArTrackable_getTrackingState(arSession, ArAsTrackable(arPlane),
                                         &planeTrackingState);
            if (planeTrackingState == AR_TRACKING_STATE_TRACKING) {
                if (surface != nullptr) {
                    LOGI("Drawing plane");
                    SkCanvas* planeCanvas = surface->getCanvas();
                    planeCanvas->save();
                    glm::mat4 model(1);
                    util::GetPlaneModelMatrix(model, arSession, arPlane);
                    glm::mat4 initRotation(1);
                    util::SetSkiaInitialRotation(initRotation);

                    SkMatrix44 mvpv = vpv * util::GlmMatToSkMat(model) * util::GlmMatToSkMat(initRotation);
                    planeCanvas->setMatrix(mvpv);

                    SkPaint paint;
                    DrawSinglePlane(arSession, arPlane, planeCanvas, paint);
                    planeCanvas->flush();
                    planeCanvas->restore();
                    planeCount++;
                }
            }
        }
        ArTrackableList_destroy(planeList);
        planeList = nullptr;
    }

    /*********** HELPERS ********************/

    void PlaneRenderer::SetPlaneColor(ArPlane* arPlane, ArTrackable* trackable, SkColor& outColor) {
        //Pick a color
        const auto iter = plane_color_map_.find(arPlane);
        if (iter != plane_color_map_.end()) {
            outColor = iter->second;

            // If this is an already observed trackable release it
            ArTrackable_release(trackable);
        } else {
            // The first plane is always white.
            if (!firstPlaneFound) {
                firstPlaneFound = true;
                outColor = SkColorSetARGB(170, 255, 255, 255);
            } else {
                outColor = GetRandomPlaneColor();
            }
            plane_color_map_.insert({arPlane, outColor});
        }
    }

    void PlaneRenderer::UpdateForPlane(const ArSession* arSession, ArPlane* arPlane, ArTrackable* trackable) {
        positions_.clear();
        colors_.clear();
        triangles_.clear();

        //Grab polygon info
        int32_t polygon_length;
        ArPlane_getPolygonSize(arSession, arPlane, &polygon_length);
        if (polygon_length == 0) {
            LOGE("PlaneRenderer::UpdatePlane, no valid plane polygon is found");
            return;
        }
        const int32_t vertices_size = polygon_length / 2;
        std::vector<glm::vec2> raw_vertices(vertices_size);
        ArPlane_getPolygon(arSession, arPlane, glm::value_ptr(raw_vertices.front()));


        //Populate vertices
        for (int32_t i = 0; i < vertices_size; ++i) {
            positions_.push_back(SkPoint::Make(raw_vertices[i].x, raw_vertices[i].y));
        }

        //Populate colors
        SkColor color;
        SetPlaneColor(arPlane, trackable, color);
        for (int32_t i = 0; i < vertices_size; ++i) {
            colors_.push_back(color);
        }

        //Populate triangles
        const int32_t vertices_length = positions_.size();
        for (int i = 0; i < vertices_length - 1; ++i) {
            triangles_.push_back(0);
            triangles_.push_back(i);
            triangles_.push_back(i + 1);
        }
    }

    void PlaneRenderer::DrawSinglePlane(ArSession* arSession, ArPlane* arPlane, SkCanvas* canvas, SkPaint& paint) {
        UpdateForPlane(arSession, arPlane, ArAsTrackable(arPlane));

        //Populate mesh vertices
        SkPoint pos[positions_.size()];
        std::copy(positions_.begin(), positions_.end(), pos);


        //Populate colors
        SkColor col[positions_.size()];
        std::copy(colors_.begin(), colors_.end(), col);


        //Populate triangle indices
        uint16_t indices[triangles_.size()];
        std::copy(triangles_.begin(), triangles_.end(), indices);


        paint.setShader(shader_);
        paint.setAlpha(kPlaneAlpha);
        sk_sp<SkVertices> vertices = SkVertices::MakeCopy(
                SkVertices::VertexMode::kTriangleFan_VertexMode,
                positions_.size(), pos, pos, col,
                triangles_.size(), indices);
        canvas->drawVertices(vertices.get(), SkBlendMode::kDstATop, paint);
    }

    bool PlaneRenderer::CullPlane(ArSession* arSession, ArFrame* arFrame, ArPlane* arPlane) {
        glm::mat4 cameraRot(1);
        glm::vec3 camPos(1);
        util::GetCameraInfo(arSession, arFrame, camPos, cameraRot);
        glm::vec4 camZ(0, 0, 1, 1);
        camZ = camZ * cameraRot;

        glm::vec3 planePos(1);
        util::GetPlanePosition(planePos, arSession, arPlane);

        glm::vec3 directionToPlane = glm::normalize(planePos - camPos);
        glm::vec3 proj = util::ProjectOntoVector(directionToPlane, glm::vec3(camZ));

        float dot = util::Dot(glm::normalize(proj), glm::normalize(glm::vec3(camZ)));
        LOGI("Cull plane dot: %.6f", dot);
        return (dot < 0);
    }

}  // namespace hello_ar
