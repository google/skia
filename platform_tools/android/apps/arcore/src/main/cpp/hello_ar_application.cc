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

#include "hello_ar_application.h"
#include <gtx/string_cast.hpp>

#include "anchor_wrapper.h"
#include "plane_renderer.h"
#include "pending_anchor.h"
#include "util.h"
#include "SkCanvas.h"
#include "GrContext.h"
#include "gl/GrGLTypes.h"
#include "SkSurface.h"
#include "SkTypeface.h"
#include "SkFontStyle.h"
#include "GrBackendSurface.h"
#include "SkMatrix44.h"
#include "SkMatrix.h"
#include "SkTextBlob.h"
#include "glm.h"
#include "SkPoint3.h"
#include "Sk3D.h"
#include <math.h>       /* acos */
#include "SkShaper.h"
#include "Skottie.h"
#include "SkAnimTimer.h"
#include "Resources.h"
#include "SkStream.h"

namespace hello_ar {
    namespace {
        constexpr size_t kMaxNumberOfAndroidsToRender = 3;
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
        andy_renderer_.InitializeGlContent(asset_manager_, "models/andy.obj",
                                           "models/andy.png");
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
            ArSession_setDisplayGeometry(ar_session_, display_rotation, width, height);;
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

    void draw_skia(SkCanvas *canvas, const SkMatrix44 &matrix, skottie::Animation *anim) {
        auto proc = [canvas, anim](SkColor c, const SkMatrix44 &matrix) {
            SkPaint p;
            p.setColor(c);
            SkRect r = {0, 0, 1, 1};
            canvas->save();
            canvas->concat(matrix);
            anim->render(canvas, &r);
            canvas->restore();
        };

        SkMatrix44 tmp(SkMatrix44::kIdentity_Constructor);

        proc(0x400000FF, matrix);
        tmp.setTranslate(0, 0, 1);
        proc(0xC00000FF, matrix * tmp);
        tmp.setRotateAboutUnit(1, 0, 0, SK_ScalarPI / 2);
        proc(0x4000FF00, matrix * tmp);
        tmp.postTranslate(0, 1, 0);
        proc(0xC000FF00, matrix * tmp);
        tmp.setRotateAboutUnit(0, 1, 0, -SK_ScalarPI / 2);
        proc(0x40FF0000, matrix * tmp);
        tmp.postTranslate(1, 0, 0);
        proc(0xC0FF0000, matrix * tmp);
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

        skProj = util::glmMatToSkMat(projection_mat);
        skView = util::glmMatToSkMat(view_mat);
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
                        skModel = iter->second;
                    }
                        break;
                    case 1: {
                        auto iter = anchor_skmat4_camera_aligned_map_.find(obj_iter);
                        skModel = iter->second;
                    }
                        break;
                    case 2: {
                        auto iter = anchor_skmat4_snap_aligned_map_.find(obj_iter);
                        skModel = iter->second;
                    }
                        break;
                    default: {
                        auto iter = anchor_skmat4_axis_aligned_map_.find(obj_iter);
                        skModel = iter->second;
                    }
                        break;
                }
                models.push_back(skModel);
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

            }
            canvas->flush();
        }
    }

    glm::vec3 ProjectOntoPlane(glm::vec3 in, glm::vec3 normal) {
        float dot = util::dot(in, normal);
        float multiplier = dot / (util::magnitude(normal) * util::magnitude(normal));
        glm::vec3 out = in - multiplier * normal;
        return out;
    }

    void LogOrientation(float rotationDirection, float angleRad, char *type) {
        LOGI("Plane orientation: %s", type);
        LOGI("Cross dotted with zDir:", rotationDirection);
        if (rotationDirection == -1) {
            LOGI("Counter Clockwise %.6f degrees rotation: ", glm::degrees(angleRad));
        } else {
            LOGI("Clockwise %.6f degrees rotation: ", glm::degrees(angleRad));
        }
    }

    glm::mat4 GetCameraRotationMatrix(float cameraOutRaw[]) {
        glm::mat4 cameraRotation(1);
        glm::quat cameraQuat = glm::quat(cameraOutRaw[0], cameraOutRaw[1], cameraOutRaw[2],
                                         cameraOutRaw[3]);
        cameraRotation = glm::toMat4(cameraQuat);
        glm::vec4 temp = cameraRotation[0];
        cameraRotation[0] = cameraRotation[2];
        cameraRotation[2] = temp;
        return cameraRotation;
    }

    bool HelloArApplication::OnTouchedFirst(float x, float y, int drawMode) {
        LOGI("Entered OnTouchedFirst");
        if (pendingAnchor != nullptr) {
            delete pendingAnchor;
        }
        SkPoint p = SkPoint::Make(x,y);
        PendingAnchor* currentAnchor = new PendingAnchor(p);
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

                    //Position of anchor
                    glm::vec4 pendingAnchorPos(out_hit_raw[4], out_hit_raw[5], out_hit_raw[6], 1);
                    currentAnchor->SetAnchorPos(pendingAnchorPos);
                    currentAnchor->SetContainingPlane(ar_plane);

                    //Check if plane contains approx the same anchor
                    auto planeAnchors = plane_anchors_map_.find(ar_plane);
                    if (planeAnchors != plane_anchors_map_.end()) {
                        //other anchors existed on this plane
                        std::vector<ArAnchor*> anchors = planeAnchors->second;
                        for(ArAnchor* const& anchor: anchors) {
                            //Get anchor's pose
                            ArPose *anchor_pose = nullptr;
                            ArPose_create(ar_session_, nullptr, &anchor_pose);
                            ArAnchor_getPose(ar_session_, anchor, anchor_pose);
                            float out_anchor_raw[] = {0, 0, 0, 0, 0, 0, 0};
                            ArPose_getPoseRaw(ar_session_, anchor_pose, out_anchor_raw);
                            ArPose_destroy(anchor_pose);
                            glm::vec4 oldAnchorPos(out_anchor_raw[4], out_anchor_raw[5], out_anchor_raw[6], 1);
                            oldAnchorPos = oldAnchorPos - pendingAnchorPos;
                            float distance = util::magnitude(glm::vec3(oldAnchorPos));
                            if (distance < 0.1f) {
                                LOGI("Anchor Touch: Editing old anchor!");
                                editAnchor = true;
                                currentAnchor->SetArAnchor(anchor);
                                currentAnchor->SetAnchorPos(oldAnchorPos);
                                break;
                            }
                        }
                    }

                    LOGI("Anchor Touch: Adding new anchor!");
                    float wanted_raw_pose[] = {0, 0, 0, 0,
                                               out_hit_raw[4], out_hit_raw[5], out_hit_raw[6]};
                    ArPose_create(ar_session_, wanted_raw_pose, &created_out_pose);
                    ar_hit_result = ar_hit;
                    out_pose = created_out_pose;
                    ArPose_destroy(hit_pose);
                    hitPlane = ar_plane;
                    break;
                }
            }

            if (editAnchor) {
                currentAnchor->SetEditMode(true);
                pendingAnchor = currentAnchor;
                return editAnchor;
            }

            if (ar_hit_result) {
                ArAnchor *anchor = nullptr;

                if (ArSession_acquireNewAnchor(ar_session_, out_pose, &anchor) != AR_SUCCESS) {
                    LOGE("HelloArApplication::OnTouched ArHitResult_acquireNewAnchor error");
                    return editAnchor;
                }

                currentAnchor->SetEditMode(false);
                currentAnchor->SetArAnchor(anchor);

                ArHitResult_destroy(ar_hit_result);
                ArHitResultList_destroy(hit_result_list);
                ArPose_destroy(out_pose);
                hit_result_list = nullptr;
                pendingAnchor = currentAnchor;
                return editAnchor;
            }

            delete pendingAnchor;
            pendingAnchor = nullptr;
            return editAnchor;
        }
    }

    void GetCameraInfo(ArSession* arSession, ArFrame* arFrame, glm::vec3& cameraPos, glm::mat4& cameraRotation) {
        //Acquire camera
        ArCamera *ar_camera;
        ArFrame_acquireCamera(arSession, arFrame, &ar_camera);

        //Get camera pose
        ArPose *camera_pose = nullptr;
        ArPose_create(arSession, nullptr, &camera_pose);
        ArCamera_getDisplayOrientedPose(arSession, ar_camera, camera_pose);

        //Get camera raw info
        float outCameraRaw[] = {0, 0, 0, 0, 0, 0, 0};
        ArPose_getPoseRaw(arSession, camera_pose, outCameraRaw);
        ArPose_destroy(camera_pose);

        //Write to out variables
        cameraPos = glm::vec3(outCameraRaw[4], outCameraRaw[5], outCameraRaw[6]);
        cameraRotation = GetCameraRotationMatrix(outCameraRaw);

        //Release camera
        ArCamera_release(ar_camera);
    }

    void HelloArApplication::OnTouchedFinal(int type) {
        LOGI("Entered OnTouchedFinal");
        DrawableType drawableType = (DrawableType) type;

        glm::mat4 matrixAxisAligned(1);
        glm::mat4 matrixCameraAligned(1);
        glm::mat4 matrixSnapAligned(1);

        SkPoint touchXY = pendingAnchor->GetTouchLocation();
        ArPlane* containingPlane = pendingAnchor->GetContainingPlane();
        glm::vec4 pendingAnchorPos = pendingAnchor->GetAnchorPos();
        ArAnchor* actualAnchor = pendingAnchor->GetArAnchor();

        //Plane model matrix
        ArPose *plane_pose = nullptr;
        ArPose_create(ar_session_, nullptr, &plane_pose);
        ArPlane_getCenterPose(ar_session_, containingPlane, plane_pose);
        glm::mat4 planeModel(1);
        util::GetTransformMatrixFromPose(ar_session_, plane_pose, &planeModel);
        ArPose_destroy(plane_pose);

        //Brings Skia world to ARCore world
        glm::mat4 initRotation(1);
        initRotation = glm::rotate(initRotation, SK_ScalarPI / 2, glm::vec3(1, 0, 0));

        //SNAP SETTINGS
        matrixSnapAligned = planeModel * initRotation;

        //ALIGNED SETTINGS
        planeModel[3] = pendingAnchorPos;
        matrixAxisAligned = planeModel * initRotation;


        //CAMERA SETTINGS
        glm::mat4 backToOrigin(1);
        backToOrigin = glm::translate(backToOrigin, -glm::vec3(pendingAnchorPos));
        glm::mat4 backToPlane(1);
        backToPlane = glm::translate(backToPlane, glm::vec3(pendingAnchorPos));

        //Axes of Skia object: start with XYZ, totate to get X(-Z)Y, paste on plane, go back to origin --> plane orientation but on origin
        glm::vec3 objX = glm::normalize(glm::vec3(
                backToOrigin * planeModel * initRotation *
                glm::vec4(1, 0, 0, 1))); //X still X
        glm::vec3 objY = glm::normalize(glm::vec3(
                backToOrigin * planeModel * initRotation *
                glm::vec4(0, 1, 0, 1))); //Y is now Z
        glm::vec3 objZ = glm::normalize(glm::vec3(
                backToOrigin * planeModel * initRotation *
                glm::vec4(0, 0, 1, 1))); //Z is now Y


        //Camera info
        glm::vec3 cameraPos;
        glm::mat4 cameraRotation;
        GetCameraInfo(ar_session_, ar_frame_, cameraPos, cameraRotation);
        glm::vec3 hitPos(pendingAnchorPos);


        ArPlaneType planeType = AR_PLANE_VERTICAL;
        ArPlane_getType(ar_session_, containingPlane, &planeType);
        if (planeType == ArPlaneType::AR_PLANE_VERTICAL) {
            //drawing hit vector at actual location
            //camera rotation
            glm::vec3 xCamera = glm::vec3(glm::vec4(1, 0, 0, 1) * cameraRotation);
            glm::vec3 yCamera = glm::vec3(glm::vec4(0, 1, 0, 1) * cameraRotation);
            glm::vec3 zCamera = glm::vec3(glm::vec4(0, 0, -1, 1) * cameraRotation);
            begins.push_back(glm::vec3(0, 0, 0));
            ends.push_back(xCamera * 0.1f);
            ends.push_back(yCamera * 0.1f);
            ends.push_back(zCamera * 0.1f);
            glm::vec3 cameraYProj = -ProjectOntoPlane(yCamera, objZ);
            float angleRad = util::angleRad(objY, cameraYProj);
            glm::vec3 cross = glm::normalize(glm::cross(objY, cameraYProj));
            float dot = util::dot(cross, objZ);
            LogOrientation(dot, angleRad, "Vertical/Wall");
            glm::mat4 flip(1);
            flip = glm::rotate(flip, SK_ScalarPI, dot * objZ);
            glm::mat4 rotateTowardsCamera(1);
            rotateTowardsCamera = glm::rotate(rotateTowardsCamera, angleRad, dot * objZ);

            matrixCameraAligned =
                    backToPlane * flip * rotateTowardsCamera * backToOrigin *
                    planeModel * initRotation;
        } else {
            glm::vec3 hitLook(hitPos - cameraPos);

            //drawing hit vector at actual location
            glm::vec3 hitLookProj = -ProjectOntoPlane(hitLook, objZ);
            float angleRad = util::angleRad(objY, hitLookProj);
            glm::vec3 cross = glm::normalize(glm::cross(objY, hitLookProj));
            float dot = util::dot(cross, objZ);


            glm::mat4 rotateTowardsCamera(1);
            rotateTowardsCamera = glm::rotate(rotateTowardsCamera, angleRad,
                                              dot * objZ);

            if (planeType == ArPlaneType::AR_PLANE_HORIZONTAL_DOWNWARD_FACING) {
                //ceiling
                LogOrientation(dot, angleRad, "Ceiling");
                glm::mat4 flip(1);
                flip = glm::rotate(flip, SK_ScalarPI, dot * objZ);
                matrixCameraAligned =
                        backToPlane * flip * rotateTowardsCamera * backToOrigin *
                        planeModel * initRotation;
            } else {
                //floor or tabletop
                LogOrientation(dot, angleRad, "Floor");
                matrixCameraAligned =
                        backToPlane * rotateTowardsCamera * backToOrigin * planeModel *
                        initRotation;
            }
        }

        anchor_skmat4_axis_aligned_map_.insert(
                {actualAnchor, util::glmMatToSkMat(matrixAxisAligned)});
        anchor_skmat4_camera_aligned_map_.insert(
                {actualAnchor, util::glmMatToSkMat(matrixCameraAligned)});
        anchor_skmat4_snap_aligned_map_.insert(
                {actualAnchor, util::glmMatToSkMat(matrixSnapAligned)});

        auto planeAnchors = plane_anchors_map_.find(containingPlane);
        if (planeAnchors != plane_anchors_map_.end()) {
            //other anchors existed on this plane
            std::vector<ArAnchor*> anchors = planeAnchors->second;
            anchors.push_back(actualAnchor);
            plane_anchors_map_.insert({containingPlane, anchors});
        } else {
            std::vector<ArAnchor*> anchors;
            anchors.push_back(actualAnchor);
            plane_anchors_map_.insert({containingPlane, anchors});
        }

        ArTrackingState tracking_state = AR_TRACKING_STATE_STOPPED;
        ArAnchor_getTrackingState(ar_session_, actualAnchor, &tracking_state);
        if (tracking_state != AR_TRACKING_STATE_TRACKING) {
            anchor_skmat4_axis_aligned_map_.erase(actualAnchor);
            anchor_skmat4_camera_aligned_map_.erase(actualAnchor);
            anchor_skmat4_snap_aligned_map_.erase(actualAnchor);
            ArAnchor_release(actualAnchor);
            return;
        }

        if (tracked_obj_set_.size() >= kMaxNumberOfAndroidsToRender) {
            ArAnchor_release(tracked_obj_set_[0]);
            anchor_skmat4_axis_aligned_map_.erase(tracked_obj_set_[0]);
            anchor_skmat4_camera_aligned_map_.erase(tracked_obj_set_[0]);
            anchor_skmat4_snap_aligned_map_.erase(tracked_obj_set_[0]);
            tracked_obj_set_.erase(tracked_obj_set_.begin());
        }

        tracked_obj_set_.push_back(actualAnchor);
    }

    void HelloArApplication::OnTouched(float x, float y, int drawMode) {
        LOGI("Touch event drawMode: %d", drawMode);
        begins.clear();
        ends.clear();
        if (ar_frame_ != nullptr && ar_session_ != nullptr) {
            bool editMode = false;
            ArHitResultList *hit_result_list = nullptr;
            ArHitResultList_create(ar_session_, &hit_result_list);
            CHECK(hit_result_list);
            ArFrame_hitTest(ar_session_, ar_frame_, x, y, hit_result_list);

            int32_t hit_result_list_size = 0;
            ArHitResultList_getSize(ar_session_, hit_result_list,
                                    &hit_result_list_size);

            // The hitTest method sorts the resulting list by distance from the camera,
            // increasing.  The first hit result will usually be the most relevant when
            // responding to user input.

            ArHitResult *ar_hit_result = nullptr;
            ArPose *out_pose = nullptr;

            glm::mat4 matrixAxisAligned(1);
            glm::mat4 matrixCameraAligned(1);
            glm::mat4 matrixSnapAligned(1);

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
                        continue;
                    }

                    //Create plane pose from trackable
                    ArPose *plane_pose = nullptr;
                    ArPose_create(ar_session_, nullptr, &plane_pose);
                    ArPlane_getCenterPose(ar_session_, ar_plane, plane_pose);

                    //Create camera display pose
                    ArPose *camera_display_pose = nullptr;
                    ArPose_create(ar_session_, nullptr, &camera_display_pose);
                    ArCamera_getDisplayOrientedPose(ar_session_, ar_camera, camera_display_pose);
                    ArCamera_release(ar_camera);

                    //Raw pose of camera
                    float out_camera_raw[] = {0, 0, 0, 0, 0, 0, 0};
                    ArPose_getPoseRaw(ar_session_, camera_pose, out_camera_raw);

                    //Raw pose of camera
                    float out_camera_display_raw[] = {0, 0, 0, 0, 0, 0, 0};
                    ArPose_getPoseRaw(ar_session_, camera_display_pose, out_camera_display_raw);

                    //Raw pose of hit location
                    float out_hit_raw[] = {0, 0, 0, 0, 0, 0, 0};
                    ArPose_getPoseRaw(ar_session_, hit_pose, out_hit_raw);

                    //Raw pose of plane
                    float out_plane_raw[] = {0, 0, 0, 0, 0, 0, 0};
                    ArPose_getPoseRaw(ar_session_, plane_pose, out_plane_raw);

                    //Position of anchor
                    glm::vec4 anchorPos(out_hit_raw[4], out_hit_raw[5], out_hit_raw[6], 1);

                    auto planeAnchors = plane_anchors_map_.find(ar_plane);
                    if (planeAnchors != plane_anchors_map_.end()) {
                        //other anchors existed on this plane
                        std::vector<ArAnchor *> anchors = planeAnchors->second;
                        for (ArAnchor *const &anchor: anchors) {
                            //Get anchor's pose
                            ArPose *anchor_pose = nullptr;
                            ArPose_create(ar_session_, nullptr, &anchor_pose);
                            ArAnchor_getPose(ar_session_, anchor, anchor_pose);
                            float out_anchor_raw[] = {0, 0, 0, 0, 0, 0, 0};
                            ArPose_getPoseRaw(ar_session_, anchor_pose, out_anchor_raw);
                            glm::vec3 oldAnchor(out_anchor_raw[4], out_anchor_raw[5],
                                                out_anchor_raw[6]);
                            oldAnchor = oldAnchor - glm::vec3(anchorPos);
                            float distance = util::magnitude(oldAnchor);
                            if (distance < 0.1f) {
                                LOGI("Anchor Touch: Editing old anchor!");
                                break;
                            }
                        }
                    }

                    LOGI("Anchor Touch: Adding new anchor!");

                    //Plane model matrix
                    glm::mat4 planeModel(1);
                    util::GetTransformMatrixFromPose(ar_session_, plane_pose, &planeModel);

                    //Brings Skia world to ARCore world
                    glm::mat4 initRotation(1);
                    initRotation = glm::rotate(initRotation, SK_ScalarPI / 2, glm::vec3(1, 0, 0));

                    //SNAP SETTINGS
                    matrixSnapAligned = planeModel * initRotation;


                    //ALIGNED SETTINGS
                    planeModel[3] = anchorPos;
                    matrixAxisAligned = planeModel * initRotation;


                    //CAMERA SETTINGS
                    glm::mat4 backToOrigin(1);
                    backToOrigin = glm::translate(backToOrigin, -glm::vec3(anchorPos));
                    glm::mat4 backToPlane(1);
                    backToPlane = glm::translate(backToPlane, glm::vec3(anchorPos));

                    //Axes of Skia object: start with XYZ, totate to get X(-Z)Y, paste on plane, go back to origin --> plane orientation but on origin
                    glm::vec3 objX = glm::normalize(glm::vec3(
                            backToOrigin * planeModel * initRotation *
                            glm::vec4(1, 0, 0, 1))); //X still X
                    glm::vec3 objY = glm::normalize(glm::vec3(
                            backToOrigin * planeModel * initRotation *
                            glm::vec4(0, 1, 0, 1))); //Y is now Z
                    glm::vec3 objZ = glm::normalize(glm::vec3(
                            backToOrigin * planeModel * initRotation *
                            glm::vec4(0, 0, 1, 1))); //Z is now Y

                    glm::vec3 cameraPos(out_camera_raw[4], out_camera_raw[5], out_camera_raw[6]);
                    glm::vec3 hitPos(out_hit_raw[4], out_hit_raw[5], out_hit_raw[6]);

                    ArPlaneType planeType = AR_PLANE_VERTICAL;
                    ArPlane_getType(ar_session_, ar_plane, &planeType);
                    if (planeType == ArPlaneType::AR_PLANE_VERTICAL) {
                        //drawing hit vector at actual location
                        //camera rotation
                        glm::mat4 cameraRotation = GetCameraRotationMatrix(out_camera_display_raw);
                        glm::vec3 xCamera = glm::vec3(glm::vec4(1, 0, 0, 1) * cameraRotation);
                        glm::vec3 yCamera = glm::vec3(glm::vec4(0, 1, 0, 1) * cameraRotation);
                        glm::vec3 zCamera = glm::vec3(glm::vec4(0, 0, -1, 1) * cameraRotation);
                        begins.push_back(glm::vec3(0, 0, 0));
                        ends.push_back(xCamera * 0.1f);
                        ends.push_back(yCamera * 0.1f);
                        ends.push_back(zCamera * 0.1f);
                        glm::vec3 cameraYProj = -ProjectOntoPlane(yCamera, objZ);
                        float angleRad = util::angleRad(objY, cameraYProj);
                        glm::vec3 cross = glm::normalize(glm::cross(objY, cameraYProj));
                        float dot = util::dot(cross, objZ);
                        LogOrientation(dot, angleRad, "Vertical/Wall");
                        glm::mat4 flip(1);
                        flip = glm::rotate(flip, SK_ScalarPI, dot * objZ);
                        glm::mat4 rotateTowardsCamera(1);
                        rotateTowardsCamera = glm::rotate(rotateTowardsCamera, angleRad,
                                                          dot * objZ);

                        matrixCameraAligned =
                                backToPlane * flip * rotateTowardsCamera * backToOrigin *
                                planeModel * initRotation;
                    } else {
                        glm::vec3 hitLook(hitPos - cameraPos);

                        //drawing hit vector at actual location
                        glm::vec3 hitLookProj = -ProjectOntoPlane(hitLook, objZ);
                        float angleRad = util::angleRad(objY, hitLookProj);
                        glm::vec3 cross = glm::normalize(glm::cross(objY, hitLookProj));
                        float dot = util::dot(cross, objZ);


                        glm::mat4 rotateTowardsCamera(1);
                        rotateTowardsCamera = glm::rotate(rotateTowardsCamera, angleRad,
                                                          dot * objZ);

                        if (planeType == ArPlaneType::AR_PLANE_HORIZONTAL_DOWNWARD_FACING) {
                            //ceiling
                            LogOrientation(dot, angleRad, "Ceiling");
                            glm::mat4 flip(1);
                            flip = glm::rotate(flip, SK_ScalarPI, dot * objZ);
                            matrixCameraAligned =
                                    backToPlane * flip * rotateTowardsCamera * backToOrigin *
                                    planeModel * initRotation;
                        } else {
                            //floor or tabletop
                            LogOrientation(dot, angleRad, "Floor");
                            matrixCameraAligned =
                                    backToPlane * rotateTowardsCamera * backToOrigin * planeModel *
                                    initRotation;
                        }
                    }

                    float wanted_raw_pose[] = {out_plane_raw[0], out_plane_raw[1], out_plane_raw[2],
                                               out_plane_raw[3],
                                               out_hit_raw[4], out_hit_raw[5], out_hit_raw[6]};
                    ArPose_create(ar_session_, wanted_raw_pose, &created_out_pose);

                    ar_hit_result = ar_hit;
                    out_pose = created_out_pose;
                    ArPose_destroy(hit_pose);
                    ArPose_destroy(camera_pose);
                    ArPose_destroy(plane_pose);
                    hitPlane = ar_plane;
                    break;
                }
            }

            if (ar_hit_result && !editMode) {
                // Note that the application is responsible for releasing the anchor
                // pointer after using it. Call ArAnchor_release(anchor) to release.
                ArAnchor *anchor = nullptr;

                if (ArSession_acquireNewAnchor(ar_session_, out_pose, &anchor) != AR_SUCCESS) {
                    LOGE("HelloArApplication::OnTouched ArHitResult_acquireNewAnchor error");
                    return;
                }

                anchor_skmat4_axis_aligned_map_.insert(
                        {anchor, util::glmMatToSkMat(matrixAxisAligned)});
                anchor_skmat4_camera_aligned_map_.insert(
                        {anchor, util::glmMatToSkMat(matrixCameraAligned)});
                anchor_skmat4_snap_aligned_map_.insert(
                        {anchor, util::glmMatToSkMat(matrixSnapAligned)});

                auto planeAnchors = plane_anchors_map_.find(hitPlane);
                if (planeAnchors != plane_anchors_map_.end()) {
                    //other anchors existed on this plane
                    std::vector<ArAnchor *> anchors = planeAnchors->second;
                    anchors.push_back(anchor);
                    plane_anchors_map_.insert({hitPlane, anchors});
                } else {
                    std::vector<ArAnchor *> anchors;
                    anchors.push_back(anchor);
                    plane_anchors_map_.insert({hitPlane, anchors});
                }

                ArTrackingState tracking_state = AR_TRACKING_STATE_STOPPED;
                ArAnchor_getTrackingState(ar_session_, anchor, &tracking_state);
                if (tracking_state != AR_TRACKING_STATE_TRACKING) {
                    anchor_skmat4_axis_aligned_map_.erase(anchor);
                    anchor_skmat4_camera_aligned_map_.erase(anchor);
                    anchor_skmat4_snap_aligned_map_.erase(anchor);
                    ArAnchor_release(anchor);
                    return;
                }

                if (tracked_obj_set_.size() >= kMaxNumberOfAndroidsToRender) {
                    ArAnchor_release(tracked_obj_set_[0]);
                    anchor_skmat4_axis_aligned_map_.erase(tracked_obj_set_[0]);
                    anchor_skmat4_camera_aligned_map_.erase(tracked_obj_set_[0]);
                    anchor_skmat4_snap_aligned_map_.erase(tracked_obj_set_[0]);
                    tracked_obj_set_.erase(tracked_obj_set_.begin());
                }

                tracked_obj_set_.push_back(anchor);

                ArHitResult_destroy(ar_hit_result);
                ArPose_destroy(out_pose);
                ArHitResultList_destroy(hit_result_list);
                hit_result_list = nullptr;
            }
        }
    }

}  // namespace hello_ar
