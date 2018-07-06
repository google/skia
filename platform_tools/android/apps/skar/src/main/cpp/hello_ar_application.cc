/*
 * Copyright 2018 Google Inc. All Rights Reserved.
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
#include "SkVertices.h"
#include "SkColorFilter.h"
#include "SkColorMatrix.h"

#include "SkArCamera.h"
#include "SkArSession.h"
#include "SkArFrame.h"
#include <memory>

using namespace std;
namespace hello_ar {
    namespace {
        constexpr size_t kMaxNumberOfObjectsToRender = 1;
        const float rotationSpeed = 2.0f;
    }  // namespace

    HelloArApplication::HelloArApplication(AAssetManager *asset_manager)
            : asset_manager_(asset_manager) {
        LOGI("OnCreate()");
    }

    HelloArApplication::~HelloArApplication() {}

    void HelloArApplication::OnPause() {
        LOGI("OnPause()");
        if (ar_session_ != nullptr) {
            ar_session_->pause();
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

            ar_session_ = SkArSession::Make(env, context);
            CHECK(ar_session_);

            ar_frame_= SkArFrame::Make(ar_session_);
            CHECK(ar_frame_);

            ar_session_->setDisplayGeometry(display_rotation_, width_, height_);
        }

        bool status = ar_session_->resume();
        CHECK(status);
    }

    void HelloArApplication::OnSurfaceCreated() {
        LOGI("OnSurfaceCreated()");
        background_renderer_.InitializeGlContent();
        plane_renderer_.InitializeGlContent(asset_manager_);
    }

    void HelloArApplication::OnDisplayGeometryChanged(int display_rotation,
                                                      int width, int height) {
        LOGI("OnSurfaceChanged(%d, %d)", width, height);
        display_rotation_ = display_rotation;
        width_ = width;
        height_ = height;

        if (ar_session_ != nullptr) {
            ar_session_->setDisplayGeometry(display_rotation, width, height);
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

    void DrawText(SkCanvas* canvas, SkPaint paint, const char text[], SkPoint offset) {
        paint.setAntiAlias(true);
        size_t byteLength = strlen(text);
        SkShaper shaper(nullptr);
        SkTextBlobBuilder builder;
        SkPoint p = SkPoint::Make(0, 0);
        shaper.shape(&builder, paint, text, byteLength, true, p, 10);
        canvas->drawTextBlob(builder.make(), offset.fX, offset.fY, paint);
    }

    void DrawAxes(SkCanvas *canvas, SkMatrix44 mvpv) {
        SkPaint p;
        p.setStrokeWidth(10);
        SkPoint3 src[4] = {
                {0,   0,   0},
                {0.2, 0,   0},
                {0,   0.2, 0},
                {0,   0,   0.2},
        };
        SkPoint dst[4];
        Sk3MapPts(dst, mvpv, src, 4);

        //"XYZ";
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

    void HelloArApplication::DrawBackground(SkCanvas* canvas, sk_sp<GrContext> context) {
        GrGLTextureInfo glInfo;
        glInfo.fID = background_renderer_.GetTextureId();
        glInfo.fFormat = GL_RGBA8_OES;
        glInfo.fTarget = GL_TEXTURE_EXTERNAL_OES;
        GrBackendTexture backendTexture = GrBackendTexture(width_, height_, GrMipMapped::kNo,
                                                           glInfo);
        sk_sp<SkImage> image = SkImage::MakeFromTexture(context.get(), backendTexture,
                                                        GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
                SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kOpaque_SkAlphaType, nullptr,
                nullptr, nullptr);

        SkPaint paint;
        SkRect src;
        src.fLeft = 0;
        src.fRight = width_;
        src.fBottom = height_;
        src.fTop = 0;

        SkRect dst;
        dst.fLeft = 0;
        dst.fRight = height_;
        dst.fBottom = width_;
        dst.fTop = 0;
        canvas->save();
        canvas->rotate(90);
        canvas->translate(0, -width_);
        canvas->drawImageRect(image, src, dst, &paint);
        canvas->restore();
    }

    bool GetCameraPerspectiveMatrices(sk_sp<SkArSession> ar_session_, sk_sp<SkArFrame> ar_frame_,
                                      glm::mat4& outViewMat, glm::mat4& outProjMat) {
        std::unique_ptr<SkArCamera> arCamera = SkArCamera::Make(ar_session_, ar_frame_);

        arCamera->getViewMatrix(ar_session_, glm::value_ptr(outViewMat));
        arCamera->getProjectionMatrix(ar_session_, 0.1f, 100.f, glm::value_ptr(outProjMat));

        SkArTrackingState cameraTrackingState;
        arCamera->getTrackingState(ar_session_, cameraTrackingState);

        //If the camera isn't tracking: return false.
        return cameraTrackingState == SkArTrackingState::kTracking;
    }

    void HelloArApplication::GetSkModelMatrices(std::vector<SkMatrix44>& outSkModels) {
        for (const auto &obj_iter : tracked_obj_set_) {
            SkArTrackingState trackingState = SkArTrackingState::kStopped;
            obj_iter->getArAnchor()->getTrackingState(ar_session_, trackingState);
            if (trackingState == SkArTrackingState::kTracking) {
                SkMatrix44 skModel = SkMatrix44::Uninitialized_Constructor();
                switch (currentObjectRotation) {
                    case 0: {
                        auto iter = anchor_skmat4_axis_aligned_map_.find(obj_iter);
                        if (iter != anchor_skmat4_axis_aligned_map_.end()) {
                            skModel = iter->second;
                            outSkModels.push_back(skModel);
                        }
                    }
                        break;
                    case 1: {
                        auto iter = anchor_skmat4_camera_aligned_map_.find(obj_iter);
                        if (iter != anchor_skmat4_camera_aligned_map_.end()) {
                            skModel = iter->second;
                            outSkModels.push_back(skModel);
                        }
                    }
                        break;
                    case 2: {
                        auto iter = anchor_skmat4_snap_aligned_map_.find(obj_iter);
                        if (iter != anchor_skmat4_snap_aligned_map_.end()) {
                            skModel = iter->second;
                            outSkModels.push_back(skModel);
                        }
                    }
                        break;
                    default: {
                        auto iter = anchor_skmat4_axis_aligned_map_.find(obj_iter);
                        if (iter != anchor_skmat4_axis_aligned_map_.end()) {
                            skModel = iter->second;
                            outSkModels.push_back(skModel);
                        }
                    }
                        break;
                }
            }
        }
    }

    void DrawModels(SkSurface* surface, std::vector<SkMatrix44>& skModels, SkMatrix44& vpv,
                    float* colorCorrection) {
        if (surface != nullptr) {
            SkCanvas* modelCanvas = surface->getCanvas();
            SkAutoCanvasRestore acr(modelCanvas, true);
            for(SkMatrix44 skModel: skModels) {
                SkMatrix44 i = SkMatrix44::kIdentity_Constructor;
                modelCanvas->setMatrix(i);
                SkMatrix44 mvpv = vpv * skModel;

                //Draw XYZ axes of Skia object
                DrawAxes(modelCanvas, mvpv);

                //Setup canvas & paint
                modelCanvas->concat(mvpv);
                SkPaint paint;
                SkColor circleColor = SkColorSetARGB(170, 250, 0, 0);
                SkColor textColor = SkColorSetARGB(170, 0, 0, 250);
                paint.setColor(circleColor);

                //No filter circle
                modelCanvas->drawCircle(0, 0, 0.1, paint);

                //Draw Text + halo + no filter
                paint.setColor(textColor);
                paint.setTextSize(0.1);
                DrawText(modelCanvas, paint, "SkAR", SkPoint::Make(0, 0));

                //With filter circle
                paint.setColor(circleColor);
                glm::vec3 colorCorr(colorCorrection[0], colorCorrection[1], colorCorrection[2]);
                colorCorr *= colorCorrection[3] / 0.466f;
                SkColorMatrix mat;
                mat.setScale(colorCorr.r, colorCorr.g, colorCorr.b, 1);
                sk_sp<SkColorFilter> filter = SkColorFilter::MakeMatrixFilterRowMajor255(mat.fMat);
                paint.setColorFilter(filter);
                modelCanvas->drawCircle(0.3f, 0, 0.1, paint);

                //Draw Text + filter
                paint.setColor(textColor);
                paint.setTextSize(0.1);
                DrawText(modelCanvas, paint, "SkAR", SkPoint::Make(0.3f, 0));
            }
            modelCanvas->flush();
        }
    }

    void GetLightEstimateInfo(ArSession* arSession, ArFrame* arFrame, ArLightEstimate* outEstimate,
                              ArLightEstimateState& outState) {
        // Get light estimation value.
        ArLightEstimate_create(arSession, &outEstimate);
        ArFrame_getLightEstimate(arSession, arFrame, outEstimate);
        ArLightEstimate_getState(arSession, outEstimate, &outState);
    }

    void HelloArApplication::OnDrawFrame() {
        if (!ar_session_) {
            return;
        }

        ar_session_->setBackendTextureFromCamera(background_renderer_.GetTextureId());

        if (!ar_session_->update(ar_frame_.get())) {
            LOGE("HelloArApplication::OnDrawFrame ArSession_update error");
        }

        //Skia drawing state
        GrBackendRenderTarget skRenderTarget;
        sk_sp<SkSurface> skSurface = nullptr;
        GrGLFramebufferInfo grFBI;
        grFBI.fFBOID = 0;
        grFBI.fFormat = 0x8058;
        skRenderTarget = GrBackendRenderTarget(width_, height_, 0, 0, grFBI);
        skSurface = SkSurface::MakeFromBackendRenderTarget(grContext.get(),
                                                           skRenderTarget,
                                                           kBottomLeft_GrSurfaceOrigin,
                                                           kRGBA_8888_SkColorType,
                                                           nullptr, nullptr);

        //GrContext global: if nullptr, make one. Else, reset it
        if (!grContext.get()) {
            grContext = GrContext::MakeGL();
        } else {
            grContext.get()->resetContext();
        }

        //Draw camera texture (background)
        if (skSurface != nullptr) {
            SkCanvas* backgroundCanvas = skSurface->getCanvas();
            backgroundCanvas->save();
            SkAutoCanvasRestore acr(backgroundCanvas, true);
            DrawBackground(backgroundCanvas, grContext);
            backgroundCanvas->restore();
        }

        //Camera matrices
        glm::mat4 viewMat;
        glm::mat4 projMat;
        if (!GetCameraPerspectiveMatrices(ar_session_, ar_frame_, viewMat, projMat)) {
            //This means AR Camera isn't tracking: don't bother rendering anything else
            return;
        }

        // Get light estimation value.
        ArLightEstimate* ar_light_estimate;
        ArLightEstimateState ar_light_estimate_state;
        ArLightEstimate_create(ar_session_->getArSession(), &ar_light_estimate);

        ArFrame_getLightEstimate(ar_session_->getArSession(), ar_frame_->getArFrame(),
                                 ar_light_estimate);
        ArLightEstimate_getState(ar_session_->getArSession(), ar_light_estimate,
                                 &ar_light_estimate_state);

        // Set light intensity to default. Intensity value ranges from 0.0f to 1.0f.
        // The first three components are color scaling factors.
        // The last one is the average pixel intensity in gamma space.
        float color_correction[4] = {1.f, 1.f, 1.f, 1.f};
        if (ar_light_estimate_state == AR_LIGHT_ESTIMATE_STATE_VALID) {
            ArLightEstimate_getColorCorrection(ar_session_->getArSession(), ar_light_estimate,
                                               color_correction);
        }

        ArLightEstimate_destroy(ar_light_estimate);
        ar_light_estimate = nullptr;

        //Set projection, view, and viewport matrices
        SkMatrix44 skProj = util::GlmMatToSkMat(projMat);
        SkMatrix44 skView = util::GlmMatToSkMat(viewMat);
        SkMatrix skViewport;
        skViewport.setScale(width_ / 2, -height_ / 2);
        skViewport.postTranslate(width_ / 2, height_ / 2);
        SkMatrix44 vpv = skViewport * skProj * skView;

        //Draw planes then point cloud
        plane_renderer_.Draw(ar_session_->getArSession(), ar_frame_->getArFrame(), grContext.get(),
                             skSurface.get(), vpv, plane_count_);
        point_cloud_renderer_.Draw(ar_session_->getArSession(), ar_frame_->getArFrame(),
                                   skSurface.get(), vpv);

        //Prepare model matrices to be used
        std::vector<SkMatrix44> skModels;
        GetSkModelMatrices(skModels);
        DrawModels(skSurface.get(), skModels, vpv, color_correction);
    }

    /************* OnTouch functions *************************************************/

    bool HelloArApplication::OnTouchedFirst(float x, float y, int drawMode) {
        LOGI("Entered OnTouchedFirst");
        ReleasePendingAnchor();
        pendingAnchor = new PendingAnchor();
        pendingAnchor->setAnchorWrapper(nullptr);
        bool editAnchor = false;

        if (ar_frame_ != nullptr && ar_session_ != nullptr) {
            //Perform hit test & get # of hit objects
            ArHitResultList* hitResultList = nullptr;
            int32_t hitResultListSize = 0;
            util::GetHitTestInfo(ar_session_->getArSession(), ar_frame_->getArFrame(), x, y,
                                 hitResultListSize, &hitResultList);

            //Outputs of hit test: a hit result, the pose at that location, and the concerned plane
            ArHitResult* hitResult = nullptr;
            ArPlane* hitPlane = nullptr;
            unique_ptr<SkArPose> hitPose;

            //Traverse the list of hit results
            for (auto i = 0; i < hitResultListSize; ++i) {
                bool traversalResult = TraverseHitResultList(hitResultList, i, &hitPose, &hitResult,
                                                             &hitPlane);
                if (traversalResult) {
                    //if traversal returned true --> continue traversal
                    continue;
                } else if (pendingAnchor->getEditMode()) {
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

                unique_ptr<SkArAnchor> newAnchor = SkArAnchor::Make(ar_session_,
                                                                    std::move(hitPose));
                if (newAnchor) {
                    //Success
                    AnchorWrapper* newAnchorW = new AnchorWrapper(std::move(newAnchor));
                    pendingAnchor->setAnchorWrapper(newAnchorW);
                    util::ReleaseHitTraversal(hitResult, hitResultList);
                    LOGI("TouchFirst: Edit %d", editAnchor);
                    return editAnchor;
                } else {
                    LOGE("HelloArApplication::OnTouchedFirst ArHitResult_acquireNewAnchor error");
                }
            }

            //Release: result, resultList, hitPose, trackable, pendingAnchor
            util::ReleaseHitTraversal(hitResult, hitResultList);
            ArTrackable* arTrackable = ArAsTrackable(hitPlane);
            ArTrackable_release(arTrackable);
            ReleasePendingAnchor();
            LOGI("TouchFirst: Edit %d", editAnchor);
            return false;
        }
        return false; //nothing happened
    }

    void HelloArApplication::OnTouchTranslate(float x, float y) {
        /*TODO: So far, this method is inefficiently implemented. I create an anchor for EACH
         * translate, which has to be avoided for the translation to appear smooth on the screen.
         * a better way to do this is to translate all the way to the end point, and when the user
         * releases their finger, we create an anchor right there
         * */

        LOGI("Entered OnTouchedTranslate");
        if (!CheckPendingAnchorOnEdit()) {
            return;
        }

        AnchorWrapper* anchorW = pendingAnchor->getAnchorWrapper();
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
            util::GetHitTestInfo(ar_session_->getArSession(), ar_frame_->getArFrame(), x, y,
                                 hitResultListSize, &hitResultList);

            //Outputs of hit test: a hit result, the pose at that location, and the concerned plane
            ArHitResult* hitResult = nullptr;
            ArPlane* hitPlane = nullptr;
            unique_ptr<SkArPose> hitPose;

            //Prepare translation matrix
            glm::mat4 translate(1);
            for (auto i = 0; i < hitResultListSize; ++i) {
                bool traversalResult = TraverseHitResultListOnTranslate(hitResultList, i, &hitPose,
                                                                        &hitResult, &hitPlane,
                                                                        translate);
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
                unique_ptr<SkArAnchor> newAnchor = SkArAnchor::Make(ar_session_,
                                                                    std::move(hitPose));
                if (newAnchor) {
                    //Update anchor wrapper after translate
                    AnchorWrapper* newAnchorW = new AnchorWrapper(std::move(newAnchor));
                    ComputeAnchorWrapperPostTranslate(anchorW, newAnchorW);

                    //Update anchor -> model matrix datastructures
                    AddModelMatrices(newAnchorW, aaMat, caMat, snapMat);

                    //Add anchor to aux datastructures
                    AddAnchorWrapper(newAnchorW, pendingAnchor->getContainingPlane());

                    util::ReleaseHitTraversal(hitResult, hitResultList);
                    return;
                } else {
                    LOGE("HelloArApplication::OnTouchedTranslate "
                                 "ArHitResult_acquireNewAnchor error");
                }
            }

            //Release: result, resultList, hitPose, trackable, pendingAnchor
            util::ReleaseHitTraversal(hitResult, hitResultList);
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

        AnchorWrapper* anchorW = pendingAnchor->getAnchorWrapper();
        glm::vec4 anchorPos = pendingAnchor->getAnchorPos(ar_session_->getArSession());
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

        AnchorWrapper* anchorW = pendingAnchor->getAnchorWrapper();
        glm::vec4 anchorPos = pendingAnchor->getAnchorPos(ar_session_->getArSession());
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
                             -pendingAnchor->getAnchorWrapper()->getMatrixInfo()->skiaAxes[2]);
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

        if (pendingAnchor->getEditMode()) {
            LOGI("WARNING: Editing old anchor in OnTouchedFinal!");
        }

        //Get necessary pending anchor info
        ArPlane* containingPlane = pendingAnchor->getContainingPlane();
        AnchorWrapper* anchorW = pendingAnchor->getAnchorWrapper();

        //Compute necessary information to be packaged for all matrix computations
        anchorW->setMatrixInfo(CreateMatrixComputationInfo(containingPlane));

        //Setup skia object model matrices
        glm::mat4 matrixAxisAligned(1);
        glm::mat4 matrixCameraAligned(1);
        glm::mat4 matrixSnapAligned(1);
        SetModelMatrices(matrixAxisAligned, matrixCameraAligned, matrixSnapAligned,
                         anchorW->getMatrixInfo());


        //Update anchor -> model matrix datastructures
        AddModelMatrices(anchorW, matrixAxisAligned, matrixCameraAligned, matrixSnapAligned);

        //Add anchor to aux datastructures
        AddAnchorWrapper(anchorW, containingPlane);
    }

    /******************* ANCHOR MANAGEMENT ***********************************************/

    void HelloArApplication::AddAnchorWrapper(AnchorWrapper* anchorW, ArPlane* containingPlane) {
        //delete anchor from matrices maps
        //releasing the anchor if it is not tracking anymore
        SkArTrackingState trackingState = SkArTrackingState::kStopped;
        anchorW->getArAnchor()->getTrackingState(ar_session_, trackingState);
        if(trackingState != SkArTrackingState::kTracking) {
            RemoveAnchorWrapper(anchorW);
            return;
        }

        //releasing the first anchor if we exceeded maximum number of objects to be rendered
        if (tracked_obj_set_.size() >= kMaxNumberOfObjectsToRender) {
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
    }

    void HelloArApplication::ComputeAnchorWrapperPostTranslate(AnchorWrapper* oldWrapper,
                                                               AnchorWrapper* newWrapper) {
        //Remove old wrapper from state of app
        RemoveAnchorWrapper(oldWrapper);

        //Update pending anchor
        pendingAnchor->setAnchorWrapper(newWrapper);
        pendingAnchor->setEditMode(true);

        //Prepare new matrix info
        util::MatrixComputationInfo* oldMatInfo = oldWrapper->getMatrixInfo();
        glm::vec4 newAnchorPos = oldWrapper->getAnchorPos(ar_session_->getArSession());
        glm::mat4 newBackToOrigin(1);
        newBackToOrigin = glm::translate(newBackToOrigin, -glm::vec3(newAnchorPos));
        glm::mat4 newBackToPlane(1);
        newBackToPlane = glm::translate(newBackToPlane, glm::vec3(newAnchorPos));
        glm::mat4 newHitModel(oldMatInfo->planeModel);
        newHitModel[3] = newAnchorPos;

        //Set new anchor fields
        std::unique_ptr<util::MatrixComputationInfo> newMatInfo(
                new util::MatrixComputationInfo(oldMatInfo->skiaAxes, oldMatInfo->initRotation,
                                                newBackToOrigin, newBackToPlane,
                                                oldMatInfo->planeModel, newHitModel));
        newWrapper->setMatrixInfo(std::move(newMatInfo));
        delete oldWrapper;
    }

    bool HelloArApplication::CheckPendingAnchorOnEdit() {
        LOGI("Editing anchor");

        if (pendingAnchor == nullptr) {
            LOGI("WARNING: Entered edit with null pending anchor!");
            return false;
        }

        AnchorWrapper* anchorW = pendingAnchor->getAnchorWrapper();
        auto iter = anchor_plane_map_.find(anchorW);
        if (iter == anchor_plane_map_.end()) {
            LOGI("WARNING: Entered edit with no finalized anchor!");
            return false;
        }

        if (!pendingAnchor->getEditMode()) {
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

    void HelloArApplication::SetModelMatrices(glm::mat4& aaMat, glm::mat4& caMat,
                                              glm::mat4& snapMat,
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
        util::GetCameraInfo(ar_session_->getArSession(), ar_frame_->getArFrame(), cameraPos,
                            cameraRotationMatrix);

        //Set matrix depending on type of surface
        ArPlaneType planeType = AR_PLANE_VERTICAL;
        ArPlane_getType(ar_session_->getArSession(), pendingAnchor->getContainingPlane(),
                        &planeType);

        if (planeType == ArPlaneType::AR_PLANE_VERTICAL) {
            //Wall: follow phone orientation
            SetCameraAlignedVertical(caMat, cameraRotationMatrix, info);
        } else {
            //Ceiling or Floor: follow hit location
            glm::vec3 hitLook(hitPos - cameraPos);
            SetCameraAlignedHorizontal(caMat, planeType, hitLook, info);
        }
    }

    std::unique_ptr<util::MatrixComputationInfo>
    HelloArApplication::CreateMatrixComputationInfo(ArPlane* containingPlane) {
        //Plane model matrix
        glm::mat4 planeModel(1);
        util::GetPlaneModelMatrix(planeModel, ar_session_->getArSession(), containingPlane);

        //Hit model matrix
        glm::mat4 hitModel = planeModel;
        glm::vec4 pendingAnchorPos = pendingAnchor->getAnchorPos(ar_session_->getArSession());
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

        return std::unique_ptr<util::MatrixComputationInfo>
                (new util::MatrixComputationInfo(skiaAxes,
                initRotation, backToOrigin, backToPlane, planeModel, hitModel));
    }

    /***************** Hit Test Helpers *******************************/

    bool HelloArApplication::CheckNeighborAnchors(ArPlane* containingPlane,
                                                  const glm::vec4& hitPosition) {
        //Check if plane contains approx the same anchor
        auto planeAnchors = plane_anchors_map_.find(containingPlane);
        if (planeAnchors != plane_anchors_map_.end()) {
            //other anchors existed on this plane
            std::vector<AnchorWrapper*> anchorWrappers = planeAnchors->second;
            for(AnchorWrapper* a: anchorWrappers) {
                //Get anchor's pose raw
                float anchorPoseRaw[] = {0, 0, 0, 0, 0, 0, 0};
                a->getArAnchor()->getSkArPose()->getPoseRaw(ar_session_, anchorPoseRaw);

                //Compute distance between current current anchor and old one
                glm::vec4 oldAnchorPos(anchorPoseRaw[4], anchorPoseRaw[5], anchorPoseRaw[6], 1);
                oldAnchorPos = oldAnchorPos - hitPosition;
                float distance = util::Magnitude(glm::vec3(oldAnchorPos));
                if (distance < 0.1f) {
                    //Distance small enough: editing an old anchor
                    LOGI("CheckNeighborAnchors: editing old anchor");
                    pendingAnchor->setAnchorWrapper(a);
                    return true;
                }
            }
        }
        return false;
    }

    bool HelloArApplication::TraverseHitResultList(ArHitResultList* hitResultList, int index,
                                                   unique_ptr<SkArPose>* outHitPose,
                                                   ArHitResult** outHitResult,
                                                   ArPlane** outHitPlane) {
        ArHitResult* currHitResult = nullptr;
        ArTrackable* currTrackable = nullptr;
        if (!util::GetTrackableInfo(ar_session_->getArSession(), hitResultList, index,
                                    currHitResult, currTrackable)) {
            return true; //continue traversal
        }

        ArTrackableType currTrackableType = AR_TRACKABLE_NOT_VALID;
        ArTrackable_getType(ar_session_->getArSession(), currTrackable, &currTrackableType);

        if (currTrackableType == AR_TRACKABLE_PLANE) {
            //Get the ArPlane
            ArPlane* currPlane = ArAsPlane(currTrackable);

            //Get the pose at the hit location
            ArPose* currHitPose = nullptr;
            ArPose_create(ar_session_->getArSession(), nullptr, &currHitPose);
            ArHitResult_getHitPose(ar_session_->getArSession(), currHitResult, currHitPose);

            //Check if hit location is inside of plane polygon (accuracy)
            int32_t inPolygon = 0;
            ArPlane_isPoseInPolygon(ar_session_->getArSession(), currPlane, currHitPose,
                                    &inPolygon);
            float currHitPoseRaw[] = {0, 0, 0, 0, 0, 0, 0};

            if (!util::CheckHitLocation(ar_session_->getArSession(), ar_frame_->getArFrame(),
                                        currHitPose, inPolygon, currHitPoseRaw)) {
                //Perform next traversal if hit location not in polygon or below plane
                return true;
            }

            //Position of anchor
            glm::vec4 pendingAnchorPos(currHitPoseRaw[4], currHitPoseRaw[5], currHitPoseRaw[6], 1);
            pendingAnchor->setContainingPlane(currPlane);

            if (CheckNeighborAnchors(currPlane, pendingAnchorPos)) {
                //Editing an anchor, stop traversal
                pendingAnchor->setEditMode(true);
                //Clear hit result memory
                ArHitResult_destroy(currHitResult);
                ArHitResultList_destroy(hitResultList);
                return false; //break traversal
            }

            //All other anchors failed: adding a new anchor
            pendingAnchor->setEditMode(false);
            *outHitResult = currHitResult;
            *outHitPlane = currPlane;

            //New anchor pose
            float hitPoseRaw[] = {0, 0, 0, 0, currHitPoseRaw[4], currHitPoseRaw[5],
                                  currHitPoseRaw[6]};
            *outHitPose = SkArPose::Make(ar_session_, hitPoseRaw);
            return false; //break traversal
        }

        //Nothing happened, continue traversal
        return true;
    }

    bool HelloArApplication::TraverseHitResultListOnTranslate(ArHitResultList* hitResultList,
                                                              int index,
                                                              unique_ptr<SkArPose>* outHitPose,
                                                              ArHitResult** outHitResult,
                                                              ArPlane** outHitPlane,
                                                              glm::mat4& outTranslate) {
        ArHitResult* currHitResult = nullptr;
        ArTrackable* currTrackable = nullptr;
        if (!util::GetTrackableInfo(ar_session_->getArSession(), hitResultList, index,
                                    currHitResult, currTrackable)) {
            return true; //continue traversal
        }

        ArTrackableType currTrackableType = AR_TRACKABLE_NOT_VALID;
        ArTrackable_getType(ar_session_->getArSession(), currTrackable, &currTrackableType);

        if (currTrackableType == AR_TRACKABLE_PLANE) {
            //Get the ArPlane
            ArPlane* currPlane = ArAsPlane(currTrackable);

            //Get the pose at the hit location
            ArPose* currHitPose = nullptr;
            ArPose_create(ar_session_->getArSession(), nullptr, &currHitPose);
            ArHitResult_getHitPose(ar_session_->getArSession(), currHitResult, currHitPose);

            //Check if hit location is inside of plane polygon (accuracy)
            int32_t inPolygon = 0;
            ArPlane_isPoseInPolygon(ar_session_->getArSession(), currPlane, currHitPose,
                                    &inPolygon);
            float currHitPoseRaw[] = {0, 0, 0, 0, 0, 0, 0};

            if (!util::CheckHitLocation(ar_session_->getArSession(), ar_frame_->getArFrame(),
                                        currHitPose, inPolygon, currHitPoseRaw)) {
                //Perform next traversal if hit location not in polygon or below plane
                return true;
            }

            //Position of anchor
            pendingAnchor->setContainingPlane(currPlane);

            //Translate by new amount
            glm::vec4 newPos(currHitPoseRaw[4], currHitPoseRaw[5], currHitPoseRaw[6], 1);
            glm::vec4 oldPos = pendingAnchor->getAnchorPos(ar_session_->getArSession());
            glm::vec3 movement = glm::vec3(newPos - oldPos);
            outTranslate = glm::translate(outTranslate, movement);

            //All other anchors failed: adding a new anchor
            *outHitResult = currHitResult;
            *outHitPlane = currPlane;

            //New anchor pose
            float hitPoseRaw[] = {0, 0, 0, 0, currHitPoseRaw[4], currHitPoseRaw[5],
                                  currHitPoseRaw[6]};
            *outHitPose = SkArPose::Make(ar_session_, hitPoseRaw);
            return false;
        }

        //Nothing happened, continue traversal
        return true;
    }

}  // namespace hello_ar
