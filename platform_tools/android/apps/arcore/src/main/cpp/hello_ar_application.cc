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

#include "platform_tools/android/apps/arcore/src/main/cpp/hello_ar_application.h"
#include <gtx/string_cast.hpp>

#include <math.h> /* acos */
#include "include/core/SkCanvas.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkMatrix44.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "include/utils/Sk3D.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skshaper/include/SkShaper.h"
#include "platform_tools/android/apps/arcore/src/main/cpp/anchor_wrapper.h"
#include "platform_tools/android/apps/arcore/src/main/cpp/glm.h"
#include "platform_tools/android/apps/arcore/src/main/cpp/pending_anchor.h"
#include "platform_tools/android/apps/arcore/src/main/cpp/plane_renderer.h"
#include "platform_tools/android/apps/arcore/src/main/cpp/util.h"
#include "tools/Resources.h"
#include "tools/timer/AnimTimer.h"

namespace hello_ar {
    namespace {
        constexpr size_t kMaxNumberOfAndroidsToRender = 1;
        constexpr int32_t kPlaneColorRgbaSize = 16;

        const glm::vec3 kWhite = {255, 255, 255};

        constexpr std::array<uint32_t, kPlaneColorRgbaSize> kPlaneColorRgba = {
                {0xFFFFFFFF, 0xF44336FF, 0xE91E63FF, 0x9C27B0FF, 0x673AB7FF, 0x3F51B5FF,
                        0x2196F3FF, 0x03A9F4FF, 0x00BCD4FF, 0x009688FF, 0x4CAF50FF, 0x8BC34AFF,
                        0xCDDC39FF, 0xFFEB3BFF, 0xFFC107FF, 0xFF9800FF}};

        inline glm::vec3 GetRandomPlaneColor() {
            const int32_t colorRgba = kPlaneColorRgba[std::rand() % kPlaneColorRgbaSize];
            return glm::vec3(((colorRgba >> 24) & 0xff) / 255.0f,
                             ((colorRgba >> 16) & 0xff) / 255.0f,
                             ((colorRgba >> 8) & 0xff) / 255.0f);
        }
    }  // namespace

    HelloArApplication::HelloArApplication(AAssetManager *asset_manager)
            : asset_manager_(asset_manager) {
        LOGI("OnCreate()");
    }

    HelloArApplication::~HelloArApplication() {
        if (ar_session_ != nullptr) {
            ArSession_destroy(ar_session_);
            ArFrame_destroy(ar_frame_);
        }
    }

    void HelloArApplication::OnPause() {
        LOGI("OnPause()");
        if (ar_session_ != nullptr) {
            ArSession_pause(ar_session_);
        }
    }

    void HelloArApplication::OnResume(void *env, void *context, void *activity) {
        LOGI("OnResume()");

        if (ar_session_ == nullptr) {
            ArInstallStatus install_status;
            // If install was not yet requested, that means that we are resuming the
            // activity first time because of explicit user interaction (such as
            // launching the application)
            bool user_requested_install = !install_requested_;

            // === ATTENTION!  ATTENTION!  ATTENTION! ===
            // This method can and will fail in user-facing situations.  Your
            // application must handle these cases at least somewhat gracefully.  See
            // HelloAR Java sample code for reasonable behavior.
            CHECK(ArCoreApk_requestInstall(env, activity, user_requested_install,
                                           &install_status) == AR_SUCCESS);

            switch (install_status) {
                case AR_INSTALL_STATUS_INSTALLED:
                    break;
                case AR_INSTALL_STATUS_INSTALL_REQUESTED:
                    install_requested_ = true;
                    return;
            }

            // === ATTENTION!  ATTENTION!  ATTENTION! ===
            // This method can and will fail in user-facing situations.  Your
            // application must handle these cases at least somewhat gracefully.  See
            // HelloAR Java sample code for reasonable behavior.
            CHECK(ArSession_create(env, context, &ar_session_) == AR_SUCCESS);
            CHECK(ar_session_);

            ArFrame_create(ar_session_, &ar_frame_);
            CHECK(ar_frame_);

            ArSession_setDisplayGeometry(ar_session_, display_rotation_, width_,
                                         height_);
        }

        const ArStatus status = ArSession_resume(ar_session_);
        CHECK(status == AR_SUCCESS);
    }

    void HelloArApplication::OnSurfaceCreated() {
        LOGI("OnSurfaceCreated()");

        background_renderer_.InitializeGlContent();
        point_cloud_renderer_.InitializeGlContent();
        plane_renderer_.InitializeGlContent(asset_manager_);
    }

    void HelloArApplication::OnDisplayGeometryChanged(int display_rotation,
                                                      int width, int height) {
        LOGI("OnSurfaceChanged(%d, %d)", width, height);
        glViewport(0, 0, width, height);
        display_rotation_ = display_rotation;
        width_ = width;
        height_ = height;

        if (ar_session_ != nullptr) {
            ArSession_setDisplayGeometry(ar_session_, display_rotation, width, height);
        }
    }

    void HelloArApplication::OnObjectRotationChanged(int rotation) {
        LOGI("OnObjectRotationChanged(%d)", rotation);
        currentObjectRotation = rotation;
    }

    void HelloArApplication::OnAction(float value) {
        LOGI("OnAction(%.6f)", value);
        currentValue = value;
    }

    void DrawText(SkCanvas *canvas, SkPaint *paint, const char text[]) {
        float spacing = 0.05;
        for (int i = 0; i < sizeof(text) / sizeof(text[0]); i++) {
            const char letter[] = {text[i]};
            size_t byteLength = strlen(static_cast<const char *>(letter));
            canvas->drawText(letter, byteLength, spacing * i, 0, *paint);
        }
    }

    void DrawAxes(SkCanvas *canvas, SkMatrix44 m) {
        SkPaint p;
        p.setStrokeWidth(10);
        SkPoint3 src[4] = {
                {0,   0,   0},
                {0.2, 0,   0},
                {0,   0.2, 0},
                {0,   0,   0.2},
        };
        SkPoint dst[4];
        Sk3MapPts(dst, m, src, 4);

        const char str[] = "XYZ";
        p.setColor(SK_ColorRED);
        canvas->drawLine(dst[0], dst[1], p);

        p.setColor(SK_ColorGREEN);
        canvas->drawLine(dst[0], dst[2], p);

        p.setColor(SK_ColorBLUE);
        canvas->drawLine(dst[0], dst[3], p);
    }

    void DrawVector(SkCanvas *canvas, SkMatrix44 m, glm::vec3 begin, glm::vec3 end, SkColor c) {
        SkPaint p;
        p.setStrokeWidth(15);
        SkPoint3 src[2] = {
                {begin.x, begin.y, begin.z},
                {end.x,   end.y,   end.z}
        };
        SkPoint dst[2];
        Sk3MapPts(dst, m, src, 2);

        const char str[] = "XYZ";
        p.setColor(c);
        canvas->drawLine(dst[0], dst[1], p);
    }

    void DrawBoundingBox(SkCanvas* canvas) {
        SkPaint paint;
        paint.setColor(SK_ColorYELLOW);
        SkIRect bounds = canvas->getDeviceClipBounds();
        SkRect b = SkRect::Make(bounds);

        canvas->drawRect(b, paint);
    }

    void HelloArApplication::OnDrawFrame() {
        grContext = GrContext::MakeGL();

        GrBackendRenderTarget target;
        sk_sp<SkSurface> surface = nullptr;
        GrGLFramebufferInfo framebuffer_info;
        framebuffer_info.fFBOID = 0;
        framebuffer_info.fFormat = 0x8058;


        glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if (ar_session_ == nullptr) return;

        ArSession_setCameraTextureName(ar_session_,
                                       background_renderer_.GetTextureId());

        // Update session to get current frame and render camera background.
        if (ArSession_update(ar_session_, ar_frame_) != AR_SUCCESS) {
            LOGE("HelloArApplication::OnDrawFrame ArSession_update error");
        }

        // GET CAMERA INFO
        ArCamera *ar_camera;
        ArFrame_acquireCamera(ar_session_, ar_frame_, &ar_camera);

        glm::mat4 view_mat;
        glm::mat4 projection_mat;
        ArCamera_getViewMatrix(ar_session_, ar_camera, glm::value_ptr(view_mat));
        ArCamera_getProjectionMatrix(ar_session_, ar_camera,
                /*near=*/0.1f, /*far=*/100.f,
                                     glm::value_ptr(projection_mat));

        ArTrackingState camera_tracking_state;
        ArCamera_getTrackingState(ar_session_, ar_camera, &camera_tracking_state);
        ArCamera_release(ar_camera);

        background_renderer_.Draw(ar_session_, ar_frame_);

        // If the camera isn't tracking don't bother rendering other objects.
        if (camera_tracking_state != AR_TRACKING_STATE_TRACKING) {
            return;
        }

        // Get light estimation value.
        ArLightEstimate *ar_light_estimate;
        ArLightEstimateState ar_light_estimate_state;
        ArLightEstimate_create(ar_session_, &ar_light_estimate);

        ArFrame_getLightEstimate(ar_session_, ar_frame_, ar_light_estimate);
        ArLightEstimate_getState(ar_session_, ar_light_estimate,
                                 &ar_light_estimate_state);

        // Set light intensity to default. Intensity value ranges from 0.0f to 1.0f.
        // The first three components are color scaling factors.
        // The last one is the average pixel intensity in gamma space.
        float color_correction[4] = {1.f, 1.f, 1.f, 1.f};
        if (ar_light_estimate_state == AR_LIGHT_ESTIMATE_STATE_VALID) {
            ArLightEstimate_getColorCorrection(ar_session_, ar_light_estimate,
                                               color_correction);
        }

        ArLightEstimate_destroy(ar_light_estimate);
        ar_light_estimate = nullptr;
        SkMatrix44 skProj;
        SkMatrix44 skView;
        SkMatrix skViewport;

        skProj = util::GlmMatToSkMat(projection_mat);
        skView = util::GlmMatToSkMat(view_mat);
        skViewport.setScale(width_ / 2, -height_ / 2);
        skViewport.postTranslate(width_ / 2, height_ / 2);
        target = GrBackendRenderTarget(width_, height_, 0, 0, framebuffer_info);
        surface = SkSurface::MakeFromBackendRenderTarget(grContext.get(),
                                                         target,
                                                         kBottomLeft_GrSurfaceOrigin,
                                                         kRGBA_8888_SkColorType,
                                                         nullptr, nullptr);

        // Render Andy objects.
        std::vector<SkMatrix44> models;
        //glm::mat4 model_mat(1.0f);
        for (const auto &obj_iter : tracked_obj_set_) {
            ArTrackingState tracking_state = AR_TRACKING_STATE_STOPPED;
            ArAnchor_getTrackingState(ar_session_, obj_iter, &tracking_state);
            if (tracking_state == AR_TRACKING_STATE_TRACKING) {
                // Render object only if the tracking state is AR_TRACKING_STATE_TRACKING.
                //util::GetTransformMatrixFromAnchor(ar_session_, obj_iter, &model_mat);
                //DRAW ANDY
                //andy_renderer_.Draw(glm::mat4(1), glm::mat4(1), model_mat, color_correction);

                //PREPARE SKIA MATS

                SkMatrix44 skModel;

                switch (currentObjectRotation) {
                    case 0: {
                        auto iter = anchor_skmat4_axis_aligned_map_.find(obj_iter);
                        if (iter != anchor_skmat4_axis_aligned_map_.end()) {
                            skModel = iter->second;
                            models.push_back(skModel);
                        }
                    }
                        break;
                    case 1: {
                        auto iter = anchor_skmat4_camera_aligned_map_.find(obj_iter);
                        if (iter != anchor_skmat4_camera_aligned_map_.end()) {
                            skModel = iter->second;
                            models.push_back(skModel);
                        }
                    }
                        break;
                    case 2: {
                        auto iter = anchor_skmat4_snap_aligned_map_.find(obj_iter);
                        if (iter != anchor_skmat4_snap_aligned_map_.end()) {
                            skModel = iter->second;
                            models.push_back(skModel);
                        }
                    }
                        break;
                    default: {
                        auto iter = anchor_skmat4_axis_aligned_map_.find(obj_iter);
                        if (iter != anchor_skmat4_axis_aligned_map_.end()) {
                            skModel = iter->second;
                            models.push_back(skModel);
                        }
                    }
                        break;
                }

            }
        }

        // Update and render planes.
        ArTrackableList *plane_list = nullptr;
        ArTrackableList_create(ar_session_, &plane_list);
        CHECK(plane_list != nullptr);

        ArTrackableType plane_tracked_type = AR_TRACKABLE_PLANE;
        ArSession_getAllTrackables(ar_session_, plane_tracked_type, plane_list);

        int32_t plane_list_size = 0;
        ArTrackableList_getSize(ar_session_, plane_list, &plane_list_size);
        plane_count_ = plane_list_size;

        for (int i = 0; i < plane_list_size; ++i) {
            ArTrackable *ar_trackable = nullptr;
            ArTrackableList_acquireItem(ar_session_, plane_list, i, &ar_trackable);
            ArPlane *ar_plane = ArAsPlane(ar_trackable);
            ArTrackingState out_tracking_state;
            ArTrackable_getTrackingState(ar_session_, ar_trackable,
                                         &out_tracking_state);

            ArPlane *subsume_plane;
            ArPlane_acquireSubsumedBy(ar_session_, ar_plane, &subsume_plane);
            if (subsume_plane != nullptr) {
                ArTrackable_release(ArAsTrackable(subsume_plane));
                continue;
            }

            if (ArTrackingState::AR_TRACKING_STATE_TRACKING != out_tracking_state) {
                continue;
            }

            ArTrackingState plane_tracking_state;
            ArTrackable_getTrackingState(ar_session_, ArAsTrackable(ar_plane),
                                         &plane_tracking_state);
            if (plane_tracking_state == AR_TRACKING_STATE_TRACKING) {
                const auto iter = plane_color_map_.find(ar_plane);
                glm::vec3 color;
                if (iter != plane_color_map_.end()) {
                    color = iter->second;

                    // If this is an already observed trackable release it so it doesn't
                    // leave aof placing objects on surfaces (n additional reference dangling.
                    ArTrackable_release(ar_trackable);
                } else {
                    // The first plane is always white.
                    if (!first_plane_has_been_found_) {
                        first_plane_has_been_found_ = true;
                        color = kWhite;
                    } else {
                        color = GetRandomPlaneColor();
                    }
                    plane_color_map_.insert({ar_plane, color});
                }

                plane_renderer_.Draw(projection_mat, view_mat, ar_session_, ar_plane,
                                     color);
            }
        }

        ArTrackableList_destroy(plane_list);
        plane_list = nullptr;

        // Update and render point cloud.
        ArPointCloud *ar_point_cloud = nullptr;
        ArStatus point_cloud_status =
                ArFrame_acquirePointCloud(ar_session_, ar_frame_, &ar_point_cloud);
        if (point_cloud_status == AR_SUCCESS) {
            point_cloud_renderer_.Draw(projection_mat * view_mat, ar_session_,
                                       ar_point_cloud);
            ArPointCloud_release(ar_point_cloud);
        }
        SkMatrix44 i = SkMatrix44::kIdentity_Constructor;

        if (surface != nullptr) {
            SkCanvas *canvas = surface->getCanvas();
            SkAutoCanvasRestore acr(canvas, true);
            SkMatrix44 vpv = skViewport * skProj * skView;
            for(SkMatrix44 skModel: models) {
                SkMatrix44 i = SkMatrix44::kIdentity_Constructor;
                canvas->setMatrix(i);
                SkMatrix44 mvpv = skViewport * skProj * skView * skModel;

                //Draw XYZ axes
                DrawAxes(canvas, mvpv);
                //Drawing camera orientation
            /*	DrawVector(canvas, vpv, begins[0], ends[0], SK_ColorMAGENTA);
                DrawVector(canvas, vpv, begins[0], ends[1], SK_ColorYELLOW);
                DrawVector(canvas, vpv, begins[0], ends[2], SK_ColorCYAN);*/

                canvas->concat(mvpv);
                SkPaint paint;

                //Draw Circle
                paint.setColor(0x80700000);
                canvas->drawCircle(0, 0, 0.1, paint);

                //Draw Text
                paint.setColor(SK_ColorBLUE);
                if (currentValue != 0) {
                    paint.setTextSize(currentValue);
                } else {
                    paint.setTextSize(0.1);
                }

                paint.setAntiAlias(true);
                const char text[] = "SkAR";
                size_t byteLength = strlen(static_cast<const char *>(text));
                SkShaper shaper(nullptr);
                SkTextBlobBuilder builder;
                SkPoint p = SkPoint::Make(0, 0);
                shaper.shape(&builder, paint, text, byteLength, true, p, 10);
                canvas->drawTextBlob(builder.make(), 0, 0, paint);

                //DrawBoundingBox(canvas);
            }
            canvas->flush();
        }
    }


    bool HelloArApplication::OnTouchedFirst(float x, float y, int drawMode) {
        LOGI("Entered OnTouchedFirst");
        if (pendingAnchor != nullptr) {
            delete pendingAnchor;
        }
        SkPoint p = SkPoint::Make(x,y);
        pendingAnchor = new PendingAnchor(p);
        bool editAnchor = false;

        if (ar_frame_ != nullptr && ar_session_ != nullptr) {
            ArHitResultList *hit_result_list = nullptr;
            ArHitResultList_create(ar_session_, &hit_result_list);
            CHECK(hit_result_list);
            ArFrame_hitTest(ar_session_, ar_frame_, x, y, hit_result_list);

            int32_t hit_result_list_size = 0;
            ArHitResultList_getSize(ar_session_, hit_result_list, &hit_result_list_size);
            ArHitResult *ar_hit_result = nullptr;
            ArPose *out_pose = nullptr;
            ArPlane* hitPlane = nullptr;
            for (int32_t i = 0; i < hit_result_list_size; ++i) {
                ArHitResult *ar_hit = nullptr;
                ArPose *created_out_pose = nullptr;
                ArHitResult_create(ar_session_, &ar_hit);
                ArHitResultList_getItem(ar_session_, hit_result_list, i, ar_hit);

                if (ar_hit == nullptr) {
                    LOGE("HelloArApplication::OnTouched ArHitResultList_getItem error");
                    return editAnchor;
                }

                ArTrackable *ar_trackable = nullptr;
                ArHitResult_acquireTrackable(ar_session_, ar_hit, &ar_trackable);
                ArTrackableType ar_trackable_type = AR_TRACKABLE_NOT_VALID;
                ArTrackable_getType(ar_session_, ar_trackable, &ar_trackable_type);
                // Creates an anchor if a plane or an oriented point was hit.
                if (AR_TRACKABLE_PLANE == ar_trackable_type) {
                    ArPose *hit_pose = nullptr;
                    ArPose_create(ar_session_, nullptr, &hit_pose);
                    ArHitResult_getHitPose(ar_session_, ar_hit, hit_pose);
                    int32_t in_polygon = 0;
                    ArPlane *ar_plane = ArAsPlane(ar_trackable);
                    ArPlane_isPoseInPolygon(ar_session_, ar_plane, hit_pose, &in_polygon);

                    {
                        // Use hit pose and camera pose to check if hittest is from the
                        // back of the plane, if it is, no need to create the anchor.
                        ArPose *camera_pose = nullptr;
                        ArPose_create(ar_session_, nullptr, &camera_pose);
                        ArCamera *ar_camera;
                        ArFrame_acquireCamera(ar_session_, ar_frame_, &ar_camera);
                        ArCamera_getPose(ar_session_, ar_camera, camera_pose);
                        float normal_distance_to_plane = util::CalculateDistanceToPlane(
                                ar_session_, *hit_pose, *camera_pose);

                        if (!in_polygon || normal_distance_to_plane < 0) {
                            ArPose_destroy(camera_pose);
                            continue;
                        }
                        ArPose_destroy(camera_pose);
                        ArCamera_release(ar_camera);
                    }

                    //Raw pose of hit location
                    float out_hit_raw[] = {0, 0, 0, 0, 0, 0, 0};
                    ArPose_getPoseRaw(ar_session_, hit_pose, out_hit_raw);
                    ArPose_destroy(hit_pose);

                    //Position of anchor
                    glm::vec4 pendingAnchorPos(out_hit_raw[4], out_hit_raw[5], out_hit_raw[6], 1);
                    pendingAnchor->SetContainingPlane(ar_plane);

                    //Check if plane contains approx the same anchor
                    auto planeAnchors = plane_anchors_map_.find(ar_plane);
                    if (planeAnchors != plane_anchors_map_.end()) {
                        //other anchors existed on this plane
                        std::vector<ArAnchor*> anchors = planeAnchors->second;
                        int i = 0;
                        LOGI("Size of anchor list: %d", (int) anchors.size());
                        for(ArAnchor* const& anchor: anchors) {
                            //Get anchor's pose
                            i++;
                            LOGI("CHECKING: Anchor #%d", i);
                            ArPose *anchor_pose = nullptr;
                            ArPose_create(ar_session_, nullptr, &anchor_pose);
                            ArAnchor_getPose(ar_session_, anchor, anchor_pose);
                            float out_anchor_raw[] = {0, 0, 0, 0, 0, 0, 0};
                            ArPose_getPoseRaw(ar_session_, anchor_pose, out_anchor_raw);
                            ArPose_destroy(anchor_pose);
                            glm::vec4 oldAnchorPos(out_anchor_raw[4], out_anchor_raw[5], out_anchor_raw[6], 1);
                            oldAnchorPos = oldAnchorPos - pendingAnchorPos;
                            float distance = util::Magnitude(glm::vec3(oldAnchorPos));
                            if (distance < 0.1f) {
                                LOGI("TouchFirst: Editing old anchor!");
                                editAnchor = true;
                                pendingAnchor->SetArAnchor(anchor);
                                pendingAnchor->SetEditMode(true);

                                ArHitResult_destroy(ar_hit);
                                ArHitResultList_destroy(hit_result_list);
                                LOGI("TouchFirst: Edit %d", editAnchor);
                                return editAnchor;
                            }
                        }
                    }

                    //actual hit result, and containing plane
                    ar_hit_result = ar_hit;
                    hitPlane = ar_plane;

                    //new anchor pos
                    float wanted_raw_pose[] = {0, 0, 0, 0, out_hit_raw[4], out_hit_raw[5], out_hit_raw[6]};
                    ArPose_create(ar_session_, wanted_raw_pose, &created_out_pose);
                    out_pose = created_out_pose;
                    break;
                }
            }


            if (ar_hit_result) {
                LOGI("TouchFirst: Adding new anchor!");
                ArAnchor *anchor = nullptr;
                pendingAnchor->SetEditMode(false);

                if (ArSession_acquireNewAnchor(ar_session_, out_pose, &anchor) != AR_SUCCESS) {
                    LOGE("HelloArApplication::OnTouched ArHitResult_acquireNewAnchor error");
                    LOGI("TouchFirst: Failed to acquire new anchor");
                    delete hitPlane;
                    delete pendingAnchor;
                    pendingAnchor = nullptr;
                    LOGI("TouchFirst: Edit %d", editAnchor);
                    return editAnchor;
                }
                pendingAnchor->SetArAnchor(anchor);

                ArHitResult_destroy(ar_hit_result);
                ArHitResultList_destroy(hit_result_list);
                ArPose_destroy(out_pose);
                hit_result_list = nullptr;
                LOGI("TouchFirst: Edit %d", editAnchor);
                return editAnchor;
            }

            LOGI("TouchFirst: didn't hit anything");
            delete hitPlane;
            delete pendingAnchor;
            pendingAnchor = nullptr;
            LOGI("TouchFirst: Edit %d", editAnchor);
            return editAnchor;
        }
    }

    void HelloArApplication::AddAnchor(ArAnchor* anchor, ArPlane* containingPlane) {
        //delete anchor from matrices maps
        //releasing the anchor if it is not tracking anymore
        ArTrackingState tracking_state = AR_TRACKING_STATE_STOPPED;
        ArAnchor_getTrackingState(ar_session_, anchor, &tracking_state);
        if (tracking_state != AR_TRACKING_STATE_TRACKING) {
            RemoveAnchor(anchor);
            return;
        }

        //releasing the first anchor if we exceeded maximum number of objects to be rendered
        if (tracked_obj_set_.size() >= kMaxNumberOfAndroidsToRender) {
            RemoveAnchor(tracked_obj_set_[0]);
        }

        //updating the containing plane with a new anchor
        auto planeAnchors = plane_anchors_map_.find(containingPlane);
        if (planeAnchors != plane_anchors_map_.end()) {
            //other anchors existed on this plane
            LOGI("TouchFinal: ADDING TO OLD ANCHORS");
            std::vector<ArAnchor*> anchors = planeAnchors->second;
            anchors.push_back(anchor);
            plane_anchors_map_[containingPlane] = anchors;
            anchor_plane_map_.insert({anchor, containingPlane});
        } else {
            LOGI("TouchFinal: NEW SET OF ANCHORS");
            std::vector<ArAnchor*> anchors;
            anchors.push_back(anchor);
            plane_anchors_map_.insert({containingPlane, anchors});
            anchor_plane_map_.insert({anchor, containingPlane});
        }

        tracked_obj_set_.push_back(anchor);
    }

    void HelloArApplication::OnTouchTranslate(float x, float y) {
        LOGI("Entered On Edit Touched");
        ArAnchor *anchor = pendingAnchor->GetArAnchor();
        glm::mat4 matrix = util::SkMatToGlmMat(
                anchor_skmat4_axis_aligned_map_.find(anchor)->second);

        if (ar_frame_ != nullptr && ar_session_ != nullptr) {
            ArHitResultList *hit_result_list = nullptr;
            ArHitResultList_create(ar_session_, &hit_result_list);
            CHECK(hit_result_list);
            ArFrame_hitTest(ar_session_, ar_frame_, x, y, hit_result_list);

            int32_t hit_result_list_size = 0;
            ArHitResultList_getSize(ar_session_, hit_result_list, &hit_result_list_size);
            ArHitResult *ar_hit_result = nullptr;
            ArPose *out_pose = nullptr;
            ArPlane *hitPlane = nullptr;
            for (int32_t i = 0; i < hit_result_list_size; ++i) {
                ArHitResult *ar_hit = nullptr;
                ArPose *created_out_pose = nullptr;
                ArHitResult_create(ar_session_, &ar_hit);
                ArHitResultList_getItem(ar_session_, hit_result_list, i, ar_hit);

                if (ar_hit == nullptr) {
                    LOGE("HelloArApplication::OnTouched ArHitResultList_getItem error");
                    return;
                }

                ArTrackable *ar_trackable = nullptr;
                ArHitResult_acquireTrackable(ar_session_, ar_hit, &ar_trackable);
                ArTrackableType ar_trackable_type = AR_TRACKABLE_NOT_VALID;
                ArTrackable_getType(ar_session_, ar_trackable, &ar_trackable_type);
                // Creates an anchor if a plane or an oriented point was hit.
                if (AR_TRACKABLE_PLANE == ar_trackable_type) {
                    ArPose *hit_pose = nullptr;
                    ArPose_create(ar_session_, nullptr, &hit_pose);
                    ArHitResult_getHitPose(ar_session_, ar_hit, hit_pose);
                    int32_t in_polygon = 0;
                    ArPlane *ar_plane = ArAsPlane(ar_trackable);
                    ArPlane_isPoseInPolygon(ar_session_, ar_plane, hit_pose, &in_polygon);

                    {
                        // Use hit pose and camera pose to check if hittest is from the
                        // back of the plane, if it is, no need to create the anchor.
                        ArPose *camera_pose = nullptr;
                        ArPose_create(ar_session_, nullptr, &camera_pose);
                        ArCamera *ar_camera;
                        ArFrame_acquireCamera(ar_session_, ar_frame_, &ar_camera);
                        ArCamera_getPose(ar_session_, ar_camera, camera_pose);
                        float normal_distance_to_plane = util::CalculateDistanceToPlane(
                                ar_session_, *hit_pose, *camera_pose);

                        if (!in_polygon || normal_distance_to_plane < 0) {
                            ArPose_destroy(camera_pose);
                            continue;
                        }
                        ArPose_destroy(camera_pose);
                        ArCamera_release(ar_camera);
                    }

                    //Raw pose of hit location
                    float out_hit_raw[] = {0, 0, 0, 0, 0, 0, 0};
                    ArPose_getPoseRaw(ar_session_, hit_pose, out_hit_raw);
                    ArPose_destroy(hit_pose);

                    //Translate by new amount
                    glm::vec4 newPos(out_hit_raw[4], out_hit_raw[5], out_hit_raw[6], 1);
                    glm::vec4 oldPos = pendingAnchor->GetAnchorPos(ar_session_);
                    glm::vec3 movement = glm::vec3(newPos - oldPos);


                    //CAMERA SETTINGS
                    glm::mat4 backToOrigin(1);
                    backToOrigin = glm::translate(backToOrigin, -glm::vec3(oldPos));
                    glm::mat4 backToPlane(1);
                    backToPlane = glm::translate(backToPlane, glm::vec3(oldPos));

                    //Axes of Skia object: start with XYZ, totate to get X(-Z)Y, paste on plane, go back to origin --> plane orientation but on origin
                    glm::vec3 objX = glm::normalize(glm::vec3(
                            backToOrigin * matrix *
                            glm::vec4(1, 0, 0, 1))); //X still X
                    glm::vec3 objY = glm::normalize(glm::vec3(
                            backToOrigin * matrix *
                            glm::vec4(0, 1, 0, 1))); //Y is now Z
                    glm::vec3 objZ = glm::normalize(glm::vec3(
                            backToOrigin * matrix *
                            glm::vec4(0, 0, 1, 1))); //Z is now Y


                    glm::mat4 translate(1);
                    translate = glm::translate(translate, movement);
                    matrix = translate * matrix;
                    RemoveAnchor(anchor);



                    //new anchor pos
                    float wanted_raw_pose[] = {0, 0, 0, 0, out_hit_raw[4], out_hit_raw[5],
                                               out_hit_raw[6]};
                    ArPose_create(ar_session_, wanted_raw_pose, &created_out_pose);
                    out_pose = created_out_pose;
                    ar_hit_result = ar_hit;
                    break;
                }
            }

            if (ar_hit_result) {
                LOGI("TouchFirst: Adding new anchor!");
                ArAnchor *anchor = nullptr;
                pendingAnchor->SetEditMode(false);

                if (ArSession_acquireNewAnchor(ar_session_, out_pose, &anchor) != AR_SUCCESS) {
                    LOGE("HelloArApplication::OnTouched ArHitResult_acquireNewAnchor error");
                    LOGI("TouchFirst: Failed to acquire new anchor");
                    delete hitPlane;
                    delete pendingAnchor;
                    pendingAnchor = nullptr;
                    return;
                }
                pendingAnchor->SetArAnchor(anchor);
                anchor_skmat4_axis_aligned_map_[anchor] = util::GlmMatToSkMat(matrix);

                //Add anchor
                AddAnchor(anchor, pendingAnchor->GetContainingPlane());


                ArHitResult_destroy(ar_hit_result);
                ArHitResultList_destroy(hit_result_list);
                ArPose_destroy(out_pose);
                hit_result_list = nullptr;
                return;
            }
        }
    }

    void HelloArApplication::RemoveAnchor(ArAnchor* anchor) {
        //delete anchor from matrices maps
        anchor_skmat4_axis_aligned_map_.erase(anchor);
        anchor_skmat4_camera_aligned_map_.erase(anchor);
        anchor_skmat4_snap_aligned_map_.erase(anchor);

        auto containingPlaneIter = anchor_plane_map_.find(anchor);
        if (containingPlaneIter != anchor_plane_map_.end()) {
            ArPlane*  containingPlane = containingPlaneIter->second;
            auto planeAnchors = plane_anchors_map_.find(containingPlane);
            if (planeAnchors != plane_anchors_map_.end()) {
                //delete this anchor from the list of anchors associated with its plane
                std::vector<ArAnchor*> anchors = planeAnchors->second;
                anchors.erase(std::remove(anchors.begin(), anchors.end(), anchor), anchors.end());
                plane_anchors_map_[planeAnchors->first] = anchors;

                //delete anchor from map of anchor to plane
                anchor_plane_map_.erase(anchor);
            }
        }
        //delete anchor from list of tracked objects
        tracked_obj_set_.erase(std::remove(tracked_obj_set_.begin(), tracked_obj_set_.end(), anchor), tracked_obj_set_.end());
        ArAnchor_release(anchor);
    }

    void HelloArApplication::UpdateMatrixMaps(ArAnchor* anchorKey, glm::mat4 aaMat, glm::mat4 caMat, glm::mat4 snapMat) {
        anchor_skmat4_axis_aligned_map_.insert({anchorKey, util::GlmMatToSkMat(aaMat)});
        anchor_skmat4_camera_aligned_map_.insert({anchorKey, util::GlmMatToSkMat(caMat)});
        anchor_skmat4_snap_aligned_map_.insert({anchorKey, util::GlmMatToSkMat(snapMat)});
    }

    void SetSkiaInitialRotation(glm::mat4& initRotation) {
        initRotation = glm::rotate(initRotation, SK_ScalarPI / 2, glm::vec3(1, 0, 0));
    }

    void SetSkiaObjectAxes(glm::vec3& x, glm::vec3& y, glm::vec3& z, glm::mat4 transform) {
        x = glm::normalize(glm::vec3(transform * glm::vec4(1, 0, 0, 1))); //X still X
        y = glm::normalize(glm::vec3(transform  * glm::vec4(0, 1, 0, 1))); //Y is now Z
        z = glm::normalize(glm::vec3(transform  * glm::vec4(0, 0, 1, 1))); //Z is now Y
    }

    void SetCameraAlignedRotation(glm::mat4& rotateTowardsCamera, float& rotationDirection, const glm::vec3& toProject, const glm::vec3& skiaY, const glm::vec3& skiaZ) {
        glm::vec3 hitLookProj = -util::ProjectOntoPlane(toProject, skiaZ);
        float angleRad = util::AngleRad(skiaY, hitLookProj);
        glm::vec3 cross = glm::normalize(glm::cross(skiaY, hitLookProj));

        //outs
        rotationDirection = util::Dot(cross, skiaZ);
        rotateTowardsCamera = glm::rotate(rotateTowardsCamera, angleRad, rotationDirection * skiaZ);
    }

    struct CameraAlignmentInfo {
        glm::vec3& skiaY, skiaZ;
        glm::mat4& preRot, postRot;

        CameraAlignmentInfo(glm::vec3& skiaY, glm::vec3& skiaZ, glm::mat4 preRot, glm::mat4 postRot)
                : skiaY(skiaY), skiaZ(skiaZ), preRot(preRot), postRot(postRot) {}
    };

    void SetCameraAlignedVertical(glm::mat4& caMat, const glm::mat4& camRot, const CameraAlignmentInfo& camAlignInfo) {
        //Camera axes
        glm::vec3 xCamera = glm::vec3(glm::vec4(1, 0, 0, 1) * camRot);
        glm::vec3 yCamera = glm::vec3(glm::vec4(0, 1, 0, 1) * camRot);
        glm::vec3 zCamera = glm::vec3(glm::vec4(0, 0, -1, 1) * camRot);

        //Get matrix that rotates object from plane towards the wanted angle
        glm::mat4 rotateTowardsCamera(1);
        float rotationDirection = 1;
        SetCameraAlignedRotation(rotateTowardsCamera, rotationDirection, yCamera, camAlignInfo.skiaY, camAlignInfo.skiaZ);

        //LogOrientation(dot, angleRad, "Vertical/Wall");
        glm::mat4 flip(1);
        flip = glm::rotate(flip, SK_ScalarPI, rotationDirection * camAlignInfo.skiaZ);
        caMat = camAlignInfo.postRot * flip * rotateTowardsCamera * camAlignInfo.preRot;
    }

    void SetCameraAlignedHorizontal(glm::mat4& caMat, ArPlaneType planeType, const glm::vec3 hitLook, const CameraAlignmentInfo& camAlignInfo) {
        //Ceiling or Floor: follow hit location
        //Get matrix that rotates object from plane towards the wanted angle
        glm::mat4 rotateTowardsCamera(1);
        float rotationDirection = 1;
        SetCameraAlignedRotation(rotateTowardsCamera, rotationDirection, hitLook, camAlignInfo.skiaY, camAlignInfo.skiaZ);

        if (planeType == ArPlaneType::AR_PLANE_HORIZONTAL_DOWNWARD_FACING) {
            //ceiling
            //LogOrientation(dot, angleRad, "Ceiling");
            glm::mat4 flip(1);
            flip = glm::rotate(flip, SK_ScalarPI, rotationDirection * camAlignInfo.skiaZ);
            caMat = camAlignInfo.postRot * flip * rotateTowardsCamera * camAlignInfo.preRot;
        } else {
            //floor or tabletop
            //LogOrientation(dot, angleRad, "Floor");
            caMat = camAlignInfo.postRot * rotateTowardsCamera * camAlignInfo.preRot;
        }
    }



    void HelloArApplication::SetCameraAlignedMatrix(glm::mat4& caMat, glm::vec3 hitPos, glm::mat4& planeModel, const glm::mat4& initRotation) {
        //Translation matrices: from plane to origin, and from origin to plane
        glm::mat4 backToOrigin(1);
        backToOrigin = glm::translate(backToOrigin, -hitPos);
        glm::mat4 backToPlane(1);
        backToPlane = glm::translate(backToPlane, hitPos);

        //Axes of Skia object: start with XYZ, totate to get X(-Z)Y, paste on plane, go back to origin --> plane orientation but on origin
        glm::vec3 skiaX, skiaY, skiaZ;
        SetSkiaObjectAxes(skiaX, skiaY, skiaZ, backToOrigin * planeModel * initRotation);

        //Get camera position & rotation
        glm::vec3 cameraPos;
        glm::mat4 cameraRotationMatrix;
        util::GetCameraInfo(ar_session_, ar_frame_, cameraPos, cameraRotationMatrix);

        //Set matrix depending on type of surface
        ArPlaneType planeType = AR_PLANE_VERTICAL;
        ArPlane_getType(ar_session_, pendingAnchor->GetContainingPlane(), &planeType);

        //Set CamerAlignmentInfo
        CameraAlignmentInfo camAlignInfo(skiaY, skiaZ, backToOrigin * planeModel * initRotation, backToPlane);

        if (planeType == ArPlaneType::AR_PLANE_VERTICAL) {
            //Wall: follow phone orientation
            SetCameraAlignedVertical(caMat, cameraRotationMatrix, camAlignInfo);
        } else {
            //Ceiling or Floor: follow hit location
            glm::vec3 hitLook(hitPos - cameraPos);
            SetCameraAlignedHorizontal(caMat, planeType, hitLook, camAlignInfo);
        }
    }


    void HelloArApplication::SetModelMatrices(glm::mat4& aaMat, glm::mat4& caMat, glm::mat4& snapMat, const glm::mat4& planeModel) {
        //Brings Skia world to ARCore world
        glm::mat4 initRotation(1);
        SetSkiaInitialRotation(initRotation);

        //Copy plane model for editing
        glm::mat4 copyPlaneModel(planeModel);

        //Set snap matrix
        //snapMat = copyPlaneModel * initRotation;

        //Set axis-aligned matrix
        glm::vec4 anchorPos = pendingAnchor->GetAnchorPos(ar_session_);
        copyPlaneModel[3] = anchorPos;
        aaMat = planeModel * initRotation;

        //Set camera-aligned matrix
        //SetCameraAlignedMatrix(caMat, glm::vec3(anchorPos), copyPlaneModel, initRotation);
    }

    void GetPlaneModelMatrix(glm::mat4& planeModel, ArSession* arSession, ArPlane* arPlane) {
        ArPose *plane_pose = nullptr;
        ArPose_create(arSession, nullptr, &plane_pose);
        ArPlane_getCenterPose(arSession, arPlane, plane_pose);
        util::GetTransformMatrixFromPose(arSession, plane_pose, &planeModel);
        ArPose_destroy(plane_pose);
    }

    void HelloArApplication::OnTouchedFinal(int type) {
        LOGI("Entered OnTouchedFinal");
        if (pendingAnchor == nullptr) {
            LOGI("WARNING: Entered OnTouchedFinal but no pending anchor..");
            return;
        }

        if (pendingAnchor->GetEditMode()) {
            LOGI("WARNING: Editing old anchor in OnTouchedFinal!");
        }

        //Get necessary pending anchor info
        ArPlane* containingPlane = pendingAnchor->GetContainingPlane();
        glm::vec4 pendingAnchorPos = pendingAnchor->GetAnchorPos(ar_session_);
        ArAnchor* actualAnchor = pendingAnchor->GetArAnchor();

        //Plane model matrix
        glm::mat4 planeModel(1);
        GetPlaneModelMatrix(planeModel, ar_session_, containingPlane);

        //Setup skia object model matrices
        glm::mat4 matrixAxisAligned(1);
        glm::mat4 matrixCameraAligned(1);
        glm::mat4 matrixSnapAligned(1);
        SetModelMatrices(matrixAxisAligned, matrixCameraAligned, matrixSnapAligned, planeModel);

        //Update anchor -> model matrix datastructures
        UpdateMatrixMaps(actualAnchor, matrixAxisAligned, matrixCameraAligned, matrixSnapAligned);

        //Add anchor to aux datastructures
        AddAnchor(actualAnchor, containingPlane);
    }

}  // namespace hello_ar
