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
#include <math.h>
#include "SkShaper.h"
#include "Skottie.h"
#include "SkAnimTimer.h"
#include "Resources.h"
#include "SkStream.h"

namespace hello_ar {
    namespace {
        constexpr size_t kMaxNumberOfAndroidsToRender = 1;
        constexpr int32_t kPlaneColorRgbaSize = 16;
        const float rotationSpeed = 2.0f;

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
            ArAnchor_getTrackingState(ar_session_, obj_iter->GetArAnchor(), &tracking_state);
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

                //Draw XYZ axes of Skia object
                DrawAxes(canvas, mvpv);

                //Setup canvas & paint
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

    /************* OnTouch functions *************************************************/

    bool HelloArApplication::OnTouchedFirst(float x, float y, int drawMode) {
        LOGI("Entered OnTouchedFirst");
        ReleasePendingAnchor();
        SkPoint p = SkPoint::Make(x,y);
        pendingAnchor = new PendingAnchor(p);
        pendingAnchor->SetAnchorWrapper(nullptr);
        bool editAnchor = false;

        if (ar_frame_ != nullptr && ar_session_ != nullptr) {
            //Perform hit test & get # of hit objects
            ArHitResultList* hitResultList = nullptr;
            int32_t hitResultListSize = 0;
            util::GetHitTestInfo(ar_session_, ar_frame_, x, y, hitResultListSize, &hitResultList);

            //Outputs of hit test: a hit result, the pose at that location, and the concerned plane
            ArHitResult* hitResult = nullptr;
            ArPose* hitPose = nullptr;
            ArPlane* hitPlane = nullptr;

            //Traverse the list of hit results
            for (auto i = 0; i < hitResultListSize; ++i) {
                bool traversalResult = TraverseHitResultList(hitResultList, i, &hitPose, &hitResult, &hitPlane);
                if (traversalResult) {
                    //if traversal returned true --> continue traversal
                    continue;
                } else if (pendingAnchor->GetEditMode()) {
                    //else, traversal returned false & editing anchor
                    return true;
                } else {
                    //else, traversal returned false & added new anchor
                    break;
                }
            }

            //If something hit: a new anchor must be created
            if (hitResult) {
                LOGI("OnTouchedFirst: adding new anchor");
                ArAnchor* newAnchor = nullptr;

                if (ArSession_acquireNewAnchor(ar_session_, hitPose, &newAnchor) == AR_SUCCESS) {
                    AnchorWrapper* newAnchorW = new AnchorWrapper(newAnchor);
                    pendingAnchor->SetAnchorWrapper(newAnchorW);
                    util::ReleaseHitTraversal(hitResult, hitResultList, hitPose);
                    LOGI("TouchFirst: Edit %d", editAnchor);
                    return editAnchor;
                } else {
                    LOGE("HelloArApplication::OnTouchedFirst ArHitResult_acquireNewAnchor error");
                }
            }

            //Release: result, resultList, hitPose, trackable, pendingAnchor
            util::ReleaseHitTraversal(hitResult, hitResultList, hitPose);
            ArTrackable* arTrackable = ArAsTrackable(hitPlane);
            ArTrackable_release(arTrackable);
            ReleasePendingAnchor();
            LOGI("TouchFirst: Edit %d", editAnchor);
            return false;
        }
        return false; //nothing happened
    }

    void HelloArApplication::OnTouchTranslate(float x, float y) {
        LOGI("Entered OnTouchedTranslate");
        if (!CheckPendingAnchorOnEdit()) {
            return;
        }

        ArAnchor* anchor = pendingAnchor->GetArAnchor();
        AnchorWrapper* anchorW = pendingAnchor->GetAnchorWrapper();
        glm::mat4 aaMat = util::SkMatToGlmMat(
                anchor_skmat4_axis_aligned_map_.find(anchorW)->second);
        glm::mat4 caMat = util::SkMatToGlmMat(
                anchor_skmat4_camera_aligned_map_.find(anchorW)->second);
        glm::mat4 snapMat = util::SkMatToGlmMat(
                anchor_skmat4_snap_aligned_map_.find(anchorW)->second);

        if (ar_frame_ != nullptr && ar_session_ != nullptr) {
            //Perform hit test & get # of hit objects
            ArHitResultList* hitResultList = nullptr;
            int32_t hitResultListSize = 0;
            util::GetHitTestInfo(ar_session_, ar_frame_, x, y, hitResultListSize, &hitResultList);

            //Outputs of hit test: a hit result, the pose at that location, and the concerned plane
            ArHitResult* hitResult = nullptr;
            ArPose* hitPose = nullptr;
            ArPlane* hitPlane = nullptr;

            //Prepare translation matrix
            glm::mat4 translate(1);
            for (auto i = 0; i < hitResultListSize; ++i) {
                bool traversalResult = TraverseHitResultListOnTranslate(hitResultList, i, &hitPose,
                                                                        &hitResult, &hitPlane, translate);
                if (traversalResult) {
                    //if traversal returned true --> continue traversal
                    continue;
                } else {
                    aaMat = translate * aaMat;
                    caMat = translate * caMat;
                    snapMat = snapMat; //untranslatable
                    break;
                }
            }

            //Translated to new place: modify the anchor wrapper
            if (hitResult) {
                ArAnchor* newAnchor = nullptr;
                if (ArSession_acquireNewAnchor(ar_session_, hitPose, &newAnchor) == AR_SUCCESS) {
                    //Update anchor wrapper after translate
                    AnchorWrapper* newAnchorW = new AnchorWrapper(newAnchor);
                    ComputeAnchorWrapperPostTranslate(anchorW, newAnchorW, newAnchor);

                    //Update anchor -> model matrix datastructures
                    AddModelMatrices(newAnchorW, aaMat, caMat, snapMat);

                    //Add anchor to aux datastructures
                    AddAnchorWrapper(newAnchorW, pendingAnchor->GetContainingPlane());

                    util::ReleaseHitTraversal(hitResult, hitResultList, hitPose);
                    return;
                } else {
                    LOGE("HelloArApplication::OnTouchedTranslate ArHitResult_acquireNewAnchor error");
                }
            }

            //Release: result, resultList, hitPose, trackable, pendingAnchor
            util::ReleaseHitTraversal(hitResult, hitResultList, hitPose);
            ArTrackable* arTrackable = ArAsTrackable(hitPlane);
            ArTrackable_release(arTrackable);
            ReleasePendingAnchor();
            return;
        }
        return; //nothing happened
    }

    void HelloArApplication::OnTouchScale(float scale) {
        if (!CheckPendingAnchorOnEdit()) {
            return;
        }

        AnchorWrapper* anchorW = pendingAnchor->GetAnchorWrapper();
        glm::vec4 anchorPos = pendingAnchor->GetAnchorPos(ar_session_);
        glm::mat4 aaMat = util::SkMatToGlmMat(
                anchor_skmat4_axis_aligned_map_.find(anchorW)->second);
        glm::mat4 caMat = util::SkMatToGlmMat(
                anchor_skmat4_camera_aligned_map_.find(anchorW)->second);
        glm::mat4 snapMat = util::SkMatToGlmMat(
                anchor_skmat4_snap_aligned_map_.find(anchorW)->second);

        glm::mat4 backToOrigin(1);
        backToOrigin = glm::translate(backToOrigin, -glm::vec3(anchorPos));
        glm::mat4 backToPlane(1);
        backToPlane = glm::translate(backToPlane, glm::vec3(anchorPos));

        glm::mat4 scaleMat(1);
        scaleMat = glm::scale(scaleMat, glm::vec3(scale, scale, scale));
        UpdateModelMatrices(anchorW, backToPlane * scaleMat * backToOrigin * aaMat,
                            backToPlane * scaleMat * backToOrigin * caMat,
                            backToPlane * scaleMat * backToOrigin * snapMat);
    }

    void HelloArApplication::OnTouchRotate(float angle) {
        if (!CheckPendingAnchorOnEdit()) {
            return;
        }

        AnchorWrapper* anchorW = pendingAnchor->GetAnchorWrapper();
        glm::vec4 anchorPos = pendingAnchor->GetAnchorPos(ar_session_);
        glm::mat4 aaMat = util::SkMatToGlmMat(
                anchor_skmat4_axis_aligned_map_.find(anchorW)->second);
        glm::mat4 caMat = util::SkMatToGlmMat(
                anchor_skmat4_camera_aligned_map_.find(anchorW)->second);
        glm::mat4 snapMat = util::SkMatToGlmMat(
                anchor_skmat4_snap_aligned_map_.find(anchorW)->second);

        glm::mat4 backToOrigin(1);
        backToOrigin = glm::translate(backToOrigin, -glm::vec3(anchorPos));
        glm::mat4 backToPlane(1);
        backToPlane = glm::translate(backToPlane, glm::vec3(anchorPos));

        glm::mat4 rotate(1);
        float angleRad = glm::radians(angle) * rotationSpeed;
        rotate = glm::rotate(rotate, angleRad,
                             -pendingAnchor->GetAnchorWrapper()->GetMatrixInfo()->skiaAxes[2]);
        UpdateModelMatrices(anchorW, backToPlane * rotate * backToOrigin * aaMat,
                            backToPlane * rotate * backToOrigin * caMat,
                            backToPlane * rotate * backToOrigin * snapMat);
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
        AnchorWrapper* anchorW = pendingAnchor->GetAnchorWrapper();

        //Compute necessary information to be packaged for all matrix computations
        util::MatrixComputationInfo* matInfo = GetMatrixComputationInfo(containingPlane);
        anchorW->SetMatrixInfo(matInfo);

        //Setup skia object model matrices
        glm::mat4 matrixAxisAligned(1);
        glm::mat4 matrixCameraAligned(1);
        glm::mat4 matrixSnapAligned(1);
        SetModelMatrices(matrixAxisAligned, matrixCameraAligned, matrixSnapAligned, matInfo);

        //Update anchor -> model matrix datastructures
        AddModelMatrices(anchorW, matrixAxisAligned, matrixCameraAligned, matrixSnapAligned);

        //Add anchor to aux datastructures
        AddAnchorWrapper(anchorW, containingPlane);
    }

    /******************* ANCHOR MANAGEMENT ***********************************************/

    void HelloArApplication::AddAnchorWrapper(AnchorWrapper* anchorW, ArPlane* containingPlane) {
        //delete anchor from matrices maps
        //releasing the anchor if it is not tracking anymore
        ArTrackingState tracking_state = AR_TRACKING_STATE_STOPPED;
        ArAnchor_getTrackingState(ar_session_, anchorW->GetArAnchor(), &tracking_state);
        if (tracking_state != AR_TRACKING_STATE_TRACKING) {
            RemoveAnchorWrapper(anchorW);
            return;
        }

        //releasing the first anchor if we exceeded maximum number of objects to be rendered
        if (tracked_obj_set_.size() >= kMaxNumberOfAndroidsToRender) {
            RemoveAnchorWrapper(tracked_obj_set_[0]);
        }

        //updating the containing plane with a new anchor
        auto planeAnchors = plane_anchors_map_.find(containingPlane);
        if (planeAnchors != plane_anchors_map_.end()) {
            //other anchors existed on this plane
            LOGI("TouchFinal: ADDING TO OLD ANCHORS");
            std::vector<AnchorWrapper*> anchorWrappers = planeAnchors->second;
            anchorWrappers.push_back(anchorW);
            plane_anchors_map_[containingPlane] = anchorWrappers;
        } else {
            LOGI("TouchFinal: NEW SET OF ANCHORS");
            std::vector<AnchorWrapper*> anchorWrappers;
            anchorWrappers.push_back(anchorW);
            plane_anchors_map_.insert({containingPlane, anchorWrappers});
        }

        auto anchorPlane = anchor_plane_map_.find(anchorW);
        if (anchorPlane != anchor_plane_map_.end()) {
            anchor_plane_map_[anchorW] =containingPlane;
        } else {
            anchor_plane_map_.insert({anchorW, containingPlane});
        }

        tracked_obj_set_.push_back(anchorW);
    }

    void HelloArApplication::RemoveAnchorWrapper(AnchorWrapper* anchorW) {
        //delete anchor from matrices maps
        anchor_skmat4_axis_aligned_map_.erase(anchorW);
        anchor_skmat4_camera_aligned_map_.erase(anchorW);
        anchor_skmat4_snap_aligned_map_.erase(anchorW);

        auto containingPlaneIter = anchor_plane_map_.find(anchorW);
        if (containingPlaneIter != anchor_plane_map_.end()) {
            ArPlane*  containingPlane = containingPlaneIter->second;
            auto planeAnchors = plane_anchors_map_.find(containingPlane);
            if (planeAnchors != plane_anchors_map_.end()) {
                //delete this anchor from the list of anchors associated with its plane
                std::vector<AnchorWrapper*> anchorWrappers = planeAnchors->second;
                anchorWrappers.erase(std::remove(anchorWrappers.begin(),
                                                 anchorWrappers.end(), anchorW),
                                                 anchorWrappers.end());
                plane_anchors_map_[planeAnchors->first] = anchorWrappers;

                //delete anchor from map of anchor to plane
                anchor_plane_map_.erase(anchorW);
            }
        }

        //delete anchor from list of tracked objects
        tracked_obj_set_.erase(std::remove(tracked_obj_set_.begin(), tracked_obj_set_.end(),
                                           anchorW), tracked_obj_set_.end());
        ArAnchor_release(anchorW->GetArAnchor());
    }

    void HelloArApplication::ComputeAnchorWrapperPostTranslate(AnchorWrapper* oldWrapper,
                                                               AnchorWrapper* newWrapper,
                                                               ArAnchor* newAnchor) {
        //Remove old wrapper from state of app
        RemoveAnchorWrapper(oldWrapper);

        //Update pending anchor
        pendingAnchor->SetAnchorWrapper(newWrapper);
        pendingAnchor->SetEditMode(true);

        //Set new anchor fields
        newWrapper->SetArAnchor(newAnchor);

        //Prepare new matrix info
        util::MatrixComputationInfo* oldMatInfo = oldWrapper->GetMatrixInfo();
        glm::vec4 newAnchorPos = oldWrapper->GetAnchorPos(ar_session_);
        glm::mat4 newBackToOrigin(1);
        newBackToOrigin = glm::translate(newBackToOrigin, -glm::vec3(newAnchorPos));
        glm::mat4 newBackToPlane(1);
        newBackToPlane = glm::translate(newBackToPlane, glm::vec3(newAnchorPos));
        glm::mat4 newHitModel(oldMatInfo->planeModel);
        newHitModel[3] = newAnchorPos;
        util::MatrixComputationInfo* newMatInfo =
                new util::MatrixComputationInfo(oldMatInfo->skiaAxes, oldMatInfo->initRotation,
                                                newBackToOrigin, newBackToPlane,
                                                oldMatInfo->planeModel, newHitModel);
        delete oldWrapper;

        //Set new anchor fields
        newWrapper->SetArAnchor(newAnchor);
        newWrapper->SetMatrixInfo(newMatInfo);
    }

    bool HelloArApplication::CheckPendingAnchorOnEdit() {
        LOGI("Editing anchor");

        if (pendingAnchor == nullptr) {
            LOGI("WARNING: Entered edit with null pending anchor!");
            return false;
        }

        AnchorWrapper* anchorW = pendingAnchor->GetAnchorWrapper();
        auto iter = anchor_plane_map_.find(anchorW);
        if (iter == anchor_plane_map_.end()) {
            LOGI("WARNING: Entered edit with no finalized anchor!");
            return false;
        }

        if (!pendingAnchor->GetEditMode()) {
            LOGI("WARNING: Entered edit with no edit mode!");
            return false;
        }

        return true;
    }

    void HelloArApplication::ReleasePendingAnchor() {
        if (pendingAnchor != nullptr) {
            delete pendingAnchor;
            pendingAnchor = nullptr;
        }
    }

    /************************ Model Matrix functions ***************/

    void HelloArApplication::SetModelMatrices(glm::mat4& aaMat, glm::mat4& caMat, glm::mat4& snapMat,
                                              util::MatrixComputationInfo* info) {
        //Set snap matrix
        snapMat = info->planeModel * info->initRotation;

        //Set axis-aligned matrix
        aaMat = info->hitModel * info->initRotation;

        //Set camera-aligned matrix
        SetCameraAlignedMatrix(caMat, glm::vec3(info->hitModel[3]), info);
    }

    // Inserts new key-value pair associated with an anchor & its corresponding model matrix
    void HelloArApplication::AddModelMatrices(AnchorWrapper* anchorWrapperKey, glm::mat4 aaMat,
                                              glm::mat4 caMat, glm::mat4 snapMat) {
        anchor_skmat4_axis_aligned_map_.insert({anchorWrapperKey, util::GlmMatToSkMat(aaMat)});
        anchor_skmat4_camera_aligned_map_.insert({anchorWrapperKey, util::GlmMatToSkMat(caMat)});
        anchor_skmat4_snap_aligned_map_.insert({anchorWrapperKey, util::GlmMatToSkMat(snapMat)});
    }

    // Updates a pre-existing anchor wrapper key in the model matrix maps
    void HelloArApplication::UpdateModelMatrices(AnchorWrapper* anchorWrapperKey, glm::mat4 aaMat,
                                                 glm::mat4 caMat, glm::mat4 snapMat) {
        anchor_skmat4_axis_aligned_map_[anchorWrapperKey] = util::GlmMatToSkMat(aaMat);
        anchor_skmat4_camera_aligned_map_[anchorWrapperKey] = util::GlmMatToSkMat(caMat);
        anchor_skmat4_snap_aligned_map_[anchorWrapperKey] = util::GlmMatToSkMat(snapMat);
    }

    // Constructs the model matrix associated with the camera-aligned orientation mode
    void HelloArApplication::SetCameraAlignedMatrix(glm::mat4& caMat, glm::vec3 hitPos,
                                                    const util::MatrixComputationInfo* info) {
        //Get camera position & rotation
        glm::vec3 cameraPos;
        glm::mat4 cameraRotationMatrix;
        util::GetCameraInfo(ar_session_, ar_frame_, cameraPos, cameraRotationMatrix);

        //Set matrix depending on type of surface
        ArPlaneType planeType = AR_PLANE_VERTICAL;
        ArPlane_getType(ar_session_, pendingAnchor->GetContainingPlane(), &planeType);

        if (planeType == ArPlaneType::AR_PLANE_VERTICAL) {
            //Wall: follow phone orientation
            SetCameraAlignedVertical(caMat, cameraRotationMatrix, info);
        } else {
            //Ceiling or Floor: follow hit location
            glm::vec3 hitLook(hitPos - cameraPos);
            SetCameraAlignedHorizontal(caMat, planeType, hitLook, info);
        }
    }

    util::MatrixComputationInfo* HelloArApplication::GetMatrixComputationInfo(ArPlane* containingPlane) {
        //Plane model matrix
        glm::mat4 planeModel(1);
        util::GetPlaneModelMatrix(planeModel, ar_session_, containingPlane);

        //Hit model matrix
        glm::mat4 hitModel = planeModel;
        glm::vec4 pendingAnchorPos = pendingAnchor->GetAnchorPos(ar_session_);
        hitModel[3] = pendingAnchorPos;

        //Brings Skia world to ARCore world
        glm::mat4 initRotation(1);
        util::SetSkiaInitialRotation(initRotation);

        //Translation matrices: from plane to origin, and from origin to plane
        glm::mat4 backToOrigin(1);
        backToOrigin = glm::translate(backToOrigin, -glm::vec3(pendingAnchorPos));
        glm::mat4 backToPlane(1);
        backToPlane = glm::translate(backToPlane, glm::vec3(pendingAnchorPos));

        //Axes of Skia object: start with XYZ, rotate to get XZY, paste on plane, go back to origin
        glm::vec3 skiaX, skiaY, skiaZ;
        util::SetSkiaObjectAxes(skiaX, skiaY, skiaZ, backToOrigin * hitModel * initRotation);
        std::vector<glm::vec3> skiaAxes;
        skiaAxes.push_back(skiaX);
        skiaAxes.push_back(skiaY);
        skiaAxes.push_back(skiaZ);

        //Set CamerAlignmentInfo
        util::MatrixComputationInfo* matInfo = new util::MatrixComputationInfo(skiaAxes,
                                   initRotation, backToOrigin, backToPlane, planeModel, hitModel);
        return matInfo;
    }

    /***************** Hit Test Helpers *******************************/

    bool HelloArApplication::CheckNeighborAnchors(ArPlane* containingPlane, const glm::vec4& hitPosition) {
        //Check if plane contains approx the same anchor
        auto planeAnchors = plane_anchors_map_.find(containingPlane);
        if (planeAnchors != plane_anchors_map_.end()) {
            //other anchors existed on this plane
            std::vector<AnchorWrapper*> anchorWrappers = planeAnchors->second;
            int i = 0;
            for(AnchorWrapper* a: anchorWrappers) {
                //Get anchor's pose raw
                ArPose* anchorPose = nullptr;
                ArPose_create(ar_session_, nullptr, &anchorPose);
                ArAnchor_getPose(ar_session_, a->GetArAnchor(), anchorPose);
                float anchorPoseRaw[] = {0, 0, 0, 0, 0, 0, 0};
                ArPose_getPoseRaw(ar_session_, anchorPose, anchorPoseRaw);
                ArPose_destroy(anchorPose);

                //Compute distance between current current anchor and old one
                glm::vec4 oldAnchorPos(anchorPoseRaw[4], anchorPoseRaw[5], anchorPoseRaw[6], 1);
                oldAnchorPos = oldAnchorPos - hitPosition;
                float distance = util::Magnitude(glm::vec3(oldAnchorPos));
                if (distance < 0.1f) {
                    //Distance small enough: editing an old anchor
                    LOGI("CheckNeighborAnchors: editing old anchor");
                    pendingAnchor->SetAnchorWrapper(a);
                    return true;
                }
            }
        }
        return false;
    }

    bool HelloArApplication::TraverseHitResultList(ArHitResultList* hitResultList, int index,
                                                   ArPose** outHitPose, ArHitResult** outHitResult,
                                                   ArPlane** outHitPlane) {
        ArHitResult* currHitResult = nullptr;
        ArTrackable* currTrackable = nullptr;
        if (!util::GetTrackableInfo(ar_session_, hitResultList, index, currHitResult, currTrackable)) {
            return true; //continue traversal
        }

        ArTrackableType currTrackableType = AR_TRACKABLE_NOT_VALID;
        ArTrackable_getType(ar_session_, currTrackable, &currTrackableType);

        if (currTrackableType == AR_TRACKABLE_PLANE) {
            //Get the ArPlane
            ArPlane* currPlane = ArAsPlane(currTrackable);

            //Get the pose at the hit location
            ArPose* currHitPose = nullptr;
            ArPose_create(ar_session_, nullptr, &currHitPose);
            ArHitResult_getHitPose(ar_session_, currHitResult, currHitPose);

            //Check if hit location is inside of plane polygon (accuracy)
            int32_t inPolygon = 0;
            ArPlane_isPoseInPolygon(ar_session_, currPlane, currHitPose, &inPolygon);
            float currHitPoseRaw[] = {0, 0, 0, 0, 0, 0, 0};

            if (!util::CheckHitLocation(ar_session_, ar_frame_, currHitPose, inPolygon, currHitPoseRaw)) {
                //Perform next traversal if hit location not in polygon or below plane
                return true;
            }

            //Position of anchor
            glm::vec4 pendingAnchorPos(currHitPoseRaw[4], currHitPoseRaw[5], currHitPoseRaw[6], 1);
            pendingAnchor->SetContainingPlane(currPlane);

            if (CheckNeighborAnchors(currPlane, pendingAnchorPos)) {
                //Editing an anchor, stop traversal
                pendingAnchor->SetEditMode(true);
                //Clear hit result memory
                ArHitResult_destroy(currHitResult);
                ArHitResultList_destroy(hitResultList);
                return false; //break traversal
            }

            //All other anchors failed: adding a new anchor
            pendingAnchor->SetEditMode(false);
            *outHitResult = currHitResult;
            *outHitPlane = currPlane;

            //New anchor pose
            float hitPoseRaw[] = {0, 0, 0, 0, currHitPoseRaw[4], currHitPoseRaw[5], currHitPoseRaw[6]};
            ArPose_create(ar_session_, hitPoseRaw, outHitPose);
            return false; //break traversal
        }

        //Nothing happened, continue traversal
        return true;
    }

    bool HelloArApplication::TraverseHitResultListOnTranslate(ArHitResultList* hitResultList, int index,
                                                              ArPose** outHitPose, ArHitResult** outHitResult,
                                                              ArPlane** outHitPlane, glm::mat4& outTranslate) {
        ArHitResult* currHitResult = nullptr;
        ArTrackable* currTrackable = nullptr;
        if (!util::GetTrackableInfo(ar_session_, hitResultList, index, currHitResult, currTrackable)) {
            return true; //continue traversal
        }

        ArTrackableType currTrackableType = AR_TRACKABLE_NOT_VALID;
        ArTrackable_getType(ar_session_, currTrackable, &currTrackableType);

        if (currTrackableType == AR_TRACKABLE_PLANE) {
            //Get the ArPlane
            ArPlane* currPlane = ArAsPlane(currTrackable);

            //Get the pose at the hit location
            ArPose* currHitPose = nullptr;
            ArPose_create(ar_session_, nullptr, &currHitPose);
            ArHitResult_getHitPose(ar_session_, currHitResult, currHitPose);

            //Check if hit location is inside of plane polygon (accuracy)
            int32_t inPolygon = 0;
            ArPlane_isPoseInPolygon(ar_session_, currPlane, currHitPose, &inPolygon);
            float currHitPoseRaw[] = {0, 0, 0, 0, 0, 0, 0};

            if (!util::CheckHitLocation(ar_session_, ar_frame_, currHitPose, inPolygon, currHitPoseRaw)) {
                //Perform next traversal if hit location not in polygon or below plane
                return true;
            }

            //Position of anchor
            pendingAnchor->SetContainingPlane(currPlane);

            //Translate by new amount
            glm::vec4 newPos(currHitPoseRaw[4], currHitPoseRaw[5], currHitPoseRaw[6], 1);
            glm::vec4 oldPos = pendingAnchor->GetAnchorPos(ar_session_);
            glm::vec3 movement = glm::vec3(newPos - oldPos);
            outTranslate = glm::translate(outTranslate, movement);

            //All other anchors failed: adding a new anchor
            *outHitResult = currHitResult;
            *outHitPlane = currPlane;

            //New anchor pose
            float hitPoseRaw[] = {0, 0, 0, 0, currHitPoseRaw[4], currHitPoseRaw[5], currHitPoseRaw[6]};
            ArPose_create(ar_session_, hitPoseRaw, outHitPose);
            return false;
        }

        //Nothing happened, continue traversal
        return true;
    }

}  // namespace hello_ar
