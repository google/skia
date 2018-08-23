/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SampleEditorController_DEFINED
#define SampleEditorController_DEFINED

#include "SampleEditorCommon.h"
#include "SampleEditorData.h"

#include "SkCanvas.h"
#include "SkPaint.h"

#include "../../third_party/externals/imgui/imgui.h"

/*
 * Controllers handle the GUI aspect of the animation editor. They hook into the data objects and
 * display and change them.
 */

// ImGui expects an array of const char* when displaying a ListBox. This function is for an
// overload of ImGui::ListBox that takes a getter so that ListBox works with
// std::vector<std::string>.
static bool vector_getter(void* v, int index, const char** out) {
    auto vector = reinterpret_cast<std::vector<std::string>*>(v);
    *out = vector->at(index).c_str();
    return true;
}

/*
 * Path controller handles take a point that they should move, as well as an optional child
 * position to move along with the main point. The child position is necessary because the endpoint
 * handles should move their child control points along with them.
 */
class EditorPathControllerHandle : public EditorClickable {
public:
    EditorPathControllerHandle()
            : fPosition(nullptr)
            , fChild(nullptr)
    {}

    void setPosition(SkPoint* position) {
        fPosition = position;
    }

    void setChild(SkPoint* child) {
        fChild = child;
    }

    void onDraw(SkCanvas* canvas) {
        SkPaint paint;
        paint.setAntiAlias(true);

        paint.setColor(0x44DD0000);
        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawCircle(*fPosition, kRadius, paint);

        paint.setColor(0x88DD0000);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(2.0f);
        canvas->drawCircle(*fPosition, kRadius, paint);
    }

    void onMouseDown(EditorClick* click) override {
        fMouseLast = click->fCurr;
        if ((fMouseLast - *fPosition).length() <= kRadius) {
            click->fTarget = this;
        }
    }

    void onMouseMove(EditorClick* click) override {
        if (click->fTarget == this) {
            SkVector diff = click->fCurr - fMouseLast;
            *fPosition += diff;
            if (fChild) {
                *fChild += diff;
            }
        }
        fMouseLast = click->fCurr;
    }

private:
    static constexpr float kRadius = 6.0f;

    SkPoint* fPosition;
    SkPoint* fChild;

    SkPoint fMouseLast;

    typedef EditorClickable INHERITED;
};

/*
 * Path controllers are essentially containers for path controller handles, which are used to move
 * control points, and thus modify the paths.
 */
class EditorPathController : public EditorClickable {
public:
    EditorPathController(EditorPathData* data)
            : fData(data)
            , fSelectedIndex(-1)
            , fHandles(4)
    {}

    void setData(EditorPathData* data) {
        fData = data;
    }

    void reset() {
        fSelectedIndex = -1;
    }

    void onDraw(SkCanvas* canvas) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(2.0f);

        canvas->save();

        // Draw the path.
        paint.setColor(0x44FF0000);
        canvas->drawPath(fData->fPath, paint);

        canvas->restore();

        // Draw the handles.
        if (fSelectedIndex > -1) {
            for (EditorPathControllerHandle& handle : fHandles) {
                handle.onDraw(canvas);
            }

            // Draw lines between the endpoints and the control points.
            paint.setColor(0x88DD0000);
            canvas->drawLine(*fData->fCubics[fSelectedIndex].fStart,
                             fData->fCubics[fSelectedIndex].fControl1, paint);
            canvas->drawLine(fData->fCubics[fSelectedIndex].fControl2,
                             fData->fCubics[fSelectedIndex].fEnd, paint);
        }
    }

    void onConfigure() {
        // List of cubics.
        ImGui::Spacing();
        ImGui::Text("Path Cubics");

        std::vector<std::string> cubicsNames;
        for (size_t i = 0; i < fData->fCubics.size(); i ++) {
            cubicsNames.push_back("Cubic " + std::to_string(i));
        }
        ImGui::PushItemWidth(-1);
        if (ImGui::ListBox("Cubics",
                           &fSelectedIndex,
                           vector_getter,
                           reinterpret_cast<void*>(&cubicsNames),
                           cubicsNames.size(),
                           5)) {
            refreshHandles();
        }
        if (ImGui::Button("Add Cubic")) {
            // Add another cubic to the path.
            EditorPathCubicData cubicData;
            cubicData.fControl1 = *fData->fEnd + SkVector::Make(0.0f, 50.0f);
            cubicData.fControl2 = *fData->fEnd + SkVector::Make(0.0f, 100.0f);
            cubicData.fEnd = *fData->fEnd + SkVector::Make(0.0f, 150.0f);
            fData->fCubics.push_back(cubicData);
            fData->update();
            refreshHandles();
        }
        if (fSelectedIndex > -1) {
            ImGui::SameLine();
            if (ImGui::Button("Remove Cubic")) {
                // Remove the selected cubic.
                fData->fCubics.erase(fData->fCubics.begin() + fSelectedIndex);
                fSelectedIndex --;
                fData->update();
                refreshHandles();
            }
        }
        if (fSelectedIndex > 0) {
            ImGui::SameLine();
            if (ImGui::Button("Match Tangent")) {
                EditorPathCubicData& current = fData->fCubics[fSelectedIndex];
                EditorPathCubicData& previous = fData->fCubics[fSelectedIndex - 1];

                // Get the previous tanget.
                SkVector prevTangent = previous.fEnd - previous.fControl2;
                prevTangent.normalize();

                // Apply it to the current cubic.
                SkVector currTangent = current.fControl1 - *(current.fStart);
                float currLength = currTangent.length();
                prevTangent.scale(currLength);
                current.fControl1 = *(current.fStart) + prevTangent;
            }
        }
    }

    void onMouseDown(EditorClick* click) override {
        if (fSelectedIndex > -1) {
            for (EditorPathControllerHandle& handle : fHandles) {
                handle.onMouseDown(click);
            }
        }
    }

    void onMouseMove(EditorClick* click) override {
        if (fSelectedIndex > -1) {
            for (EditorPathControllerHandle& handle : fHandles) {
                handle.onMouseMove(click);
            }

            // If something moved, update the path.
            if (click->fTarget) {
                fData->update();
            }
        }
    }

private:
    void refreshHandles() {
        // Start handle.
        fHandles[0].setPosition(fData->fCubics[fSelectedIndex].fStart);
        fHandles[0].setChild(&fData->fCubics[fSelectedIndex].fControl1);

        // Control 1 handle.
        fHandles[1].setPosition(&fData->fCubics[fSelectedIndex].fControl1);
        fHandles[1].setChild(nullptr);

        // Control 2 handle.
        fHandles[2].setPosition(&fData->fCubics[fSelectedIndex].fControl2);
        fHandles[2].setChild(nullptr);

        // End handle.
        fHandles[3].setPosition(&fData->fCubics[fSelectedIndex].fEnd);
        fHandles[3].setChild(&fData->fCubics[fSelectedIndex].fControl2);
    }

private:
    EditorPathData* fData;

    int fSelectedIndex;
    std::vector<EditorPathControllerHandle> fHandles;

    typedef EditorClickable INHERITED;
};

/*
 * The bone controllers are responsible for changing the orientation of bones. They also contain
 * the path controllers that modify the path each bone travels along during animation.
 */
class EditorBoneController : public EditorClickable {
public:
    EditorBoneController(EditorBoneData* data)
            : fData(data)
            , fMouseDiff()
            , fSelectState(kNone_SelectState)
            , fPathController(&data->fPath)
    {}

    bool selected() const { return fSelectState != kNone_SelectState; }

    void setData(EditorBoneData* data) {
        fData = data;
        fPathController.setData(&data->fPath);
    }

    void setPosition(float x, float y) {
        fData->fPosition.set(x, y);
        fData->update();
    }

    void setRotation(float rotation) {
        fData->fRotation = rotation;
        fData->update();
    }

    void reset() {
        // Set the position and rotation to their bind orientations.
        fData->fPosition.set(fData->fBindPosition.x(), fData->fBindPosition.y());
        fData->fRotation = fData->fBindRotation;
        fData->update();
    }

    void onDraw(SkCanvas* canvas, EditorMode mode) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStrokeWidth(2.0f);

        // Set the transforms so that we can draw in bone space.
        canvas->save();
        canvas->concat(fData->fMatrix);
        canvas->translate(fData->fBindPosition.x(), fData->fBindPosition.y());
        canvas->rotate(fData->fBindRotation);

        // Draw the point.
        this->fillStyle(paint);
        canvas->drawCircle(0.0f, 0.0f, kRadius, paint);
        this->strokeStyle(paint);
        canvas->drawCircle(0.0f, 0.0f, kRadius, paint);

        // Draw the direction.
        SkRect directionRect = SkRect::MakeXYWH(kRadius + 3.0f, -kRadius * 0.5f, kLength, kRadius);
        this->fillStyle(paint);
        canvas->drawRoundRect(directionRect, 5.0f, 5.0f, paint);
        this->strokeStyle(paint);
        canvas->drawRoundRect(directionRect, 5.0f, 5.0f, paint);

        canvas->restore();

        // Draw the path.
        if (mode == kAnimate_EditorMode) {
            fPathController.onDraw(canvas);
        }
    }

    void onConfigure(EditorMode mode) {
        if (fSelectState == kNone_SelectState) {
            return;
        }
        ImGui::Spacing();
        if (!ImGui::CollapsingHeader("Bone Config", ImGuiTreeNodeFlags_DefaultOpen)) {
            return;
        }

        switch (mode) {
            case kSetup_EditorMode: {
                // Bone information.
                ImGui::Text("Position: (%.1f, %.1f)", fData->fPosition.x(), fData->fPosition.y());
                ImGui::Text("Rotation: %.1f", fData->fRotation);
                ImGui::Spacing();

                // Weight function.
                ImGui::Text("Weight Function");
                int type = fData->fFunction->type();
                ImGui::RadioButton("Uniform", &type, kUniform_EditorWeightFunctionType);
                ImGui::RadioButton("Gaussian", &type, kGaussian_EditorWeightFunctionType);

                if (fData->fFunction->type() != type) {
                    switch (type) {
                        case kUniform_EditorWeightFunctionType: {
                            fData->fFunction = std::make_unique<UniformEditorWeightFunction>();
                            break;
                        }
                        case kGaussian_EditorWeightFunctionType: {
                            fData->fFunction = std::make_unique<GaussianEditorWeightFunction>();
                            break;
                        }
                    }
                }
                fData->fFunction->onConfigure();

                break;
            }
            case kAnimate_EditorMode: {
                // Bone information.
                ImGui::Text("Position: (%.1f, %.1f)", fData->fPosition.x(), fData->fPosition.y());
                ImGui::Text("Rotation: %.1f", fData->fRotation);

                // Path configuration.
                fPathController.onConfigure();
                break;
            }
        }
    }

    void onMouseDown(EditorClick* click) override {
        if (click->fTarget) {
            fSelectState = kNone_SelectState;
            return;
        }

        // Pass to the path controller.
        if (this->selected()) {
            fPathController.onMouseDown(click);
            if (click->fTarget) {
                return;
            }
        }

        float x = click->fCurr.x();
        float y = click->fCurr.y();

        // Check if the position has been clicked.
        fMouseDiff = SkVector::Make(x, y) - fData->fPosition;
        if (fMouseDiff.length() <= kRadius) {
            click->fTarget = this;
            fSelectState = kPosition_SelectState;
            return;
        }

        // Check if the rotation has been clicked.
        SkMatrix rotationMatrix = SkMatrix::I();
        rotationMatrix.setRotate(-fData->fRotation);
        SkVector rotated = rotationMatrix.mapXY(fMouseDiff.x(), fMouseDiff.y());
        if (rotated.x() >= kRadius + 3.0f && rotated.x() <= kRadius + 3.0f + kLength &&
            rotated.y() >= -kRadius * 0.5f && rotated.y() <= kRadius * 0.5f) {
            click->fTarget = this;
            fSelectState = kRotation_SelectState;
            return;
        }

        // Not selected.
        if (!click->fTarget) {
            fSelectState = kNone_SelectState;
            fPathController.reset();
        }
    }

    void onMouseMove(EditorClick* click) override {
        if (click->fTarget != this || click->fMode != kSetup_EditorMode) {
            // Pass to the path controller.
            fPathController.onMouseMove(click);
            return;
        }
        float x = click->fCurr.x();
        float y = click->fCurr.y();

        switch (fSelectState) {
            case kNone_SelectState: {
                return;
            }
            case kPosition_SelectState: {
                // Move the position.
                this->setPosition(x - fMouseDiff.x(), y - fMouseDiff.y());
                fData->setBind();
                return;
            }
            case kRotation_SelectState: {
                // Follow the mouse to adjust the rotation.
                SkVector diff = SkPoint::Make(x, y) - fData->fPosition;
                this->setRotation(SkRadiansToDegrees(SkScalarATan2(diff.y(), diff.x())));
                fData->setBind();
                return;
            }
        }
    }

private:
    void fillStyle(SkPaint& paint) const {
        paint.setStyle(SkPaint::kFill_Style);
        switch (fSelectState) {
            case kNone_SelectState: {
                paint.setColor(0x22000000);
                break;
            }
            case kPosition_SelectState:
            case kRotation_SelectState: {
                paint.setColor(0x44AAAAFF);
                break;
            }
        }
    }

    void strokeStyle(SkPaint& paint) const {
        paint.setStyle(SkPaint::kStroke_Style);
        switch (fSelectState) {
            case kNone_SelectState: {
                paint.setColor(0x44000000);
                break;
            }
            case kPosition_SelectState:
            case kRotation_SelectState: {
                paint.setColor(0x88AAAAFF);
                break;
            }
        }
    }

private:
    static constexpr float kRadius = 10.0f;
    static constexpr float kLength = 20.0f;

    enum SelectState {
        kNone_SelectState     = 0,
        kPosition_SelectState = 1,
        kRotation_SelectState = 2,
    };

    EditorBoneData* fData;

    SkVector fMouseDiff;
    SelectState fSelectState;

    EditorPathController fPathController;
};

/*
 * The EditorController controls the animation playback and editor mode switching. For the most
 * part, it is responsible for passing input events to the bone controllers.
 */
class EditorController {
public:
    EditorController(EditorAnimation* animation);

    void onDraw(SkCanvas* canvas);
    void onConfigure();
    void onAnimate(float delta);

    void onMouseDown(EditorClick* click);
    void onMouseMove(EditorClick* click);
    void onMouseUp(EditorClick* click);

private:
    void addBone();
    void removeBone(int index);

    void update();

private:
    EditorAnimation* fAnimation;
    float fAnimationTime;
    bool fPlaying;

    EditorMode fMode;

    std::vector<EditorBoneController> fBoneControllers;
};

#endif
