/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleEditorCommon.h"
#include "SampleEditorController.h"
#include "SampleEditorData.h"
#include "SampleEditorSerializer.h"

#include "SkAnimTimer.h"
#include "SkSurface.h"

class EditorContent {
public:
    EditorContent(SkISize size)
            : fSize(size)
    {
        SkAssertResult(fSize.width());
    }

    void onDraw(SkCanvas* canvas) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(0xFFDDDDDD);

        // Draw a border.
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(3.0f);
        canvas->drawRect(SkRect::MakeXYWH(0, 0, 500, 500), paint);

        // Draw a checkerboard.
        paint.setStyle(SkPaint::kFill_Style);
        for (int i = 0; i < 25; i ++) {
            for (int j = 0; j < 25; j ++) {
                if (i % 2) {
                    if (j % 2) {
                        continue;
                    }
                } else {
                    if (!(j % 2)) {
                        continue;
                    }
                }
                canvas->save();
                canvas->translate(j * 20, i * 20);
                canvas->drawRect(SkRect::MakeXYWH(0, 0, 20, 20), paint);
                canvas->restore();
            }
        }
    }

private:
    SkISize fSize;
};

class EditorView : public Sample {
public:
    EditorView()
            : INHERITED()
            , fAnimation(EditorAnimation::Default())
            , fController(&fAnimation)
            , fContent(fAnimation.fSize)
            , fVertices(nullptr)
            , fBones()
            , fSurface(nullptr)
            , fLastTick(-1.0f)
            , fInputBuf("default.json")
    {
        fSurface = SkSurface::MakeRasterN32Premul(fAnimation.fSize.width(),
                                                  fAnimation.fSize.height());
    }

protected:
    virtual bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "Editor");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onOnceBeforeDraw() override {
    }

    void onDrawContent(SkCanvas* canvas) override {
        // Regenerate the quad.
        fVertices = this->generateQuad(fAnimation.fSize.width(), fAnimation.fSize.height(),
                                       16, 16); // TODO: Adjustable LOD.

        // Draw the surface.
        SkCanvas* surfaceCanvas = fSurface->getCanvas();
        surfaceCanvas->clear(0xFFFAFAFA);
        int surfaceState = surfaceCanvas->save();
        fContent.onDraw(surfaceCanvas);
        surfaceCanvas->restoreToCount(surfaceState);

        // Generate the image shader.
        SkPaint paint;
        sk_sp<SkImage> snapshot = fSurface->makeImageSnapshot();
        paint.setShader(snapshot->makeShader());

        // Draw the vertices.
        if (fBones.size()) {
            paint.setFilterQuality(SkFilterQuality::kLow_SkFilterQuality);
            canvas->drawVertices(fVertices.get(),
                                 fBones.data(), fBones.size(),
                                 SkBlendMode::kSrc, paint);
        }

        // Draw the controller.
        fController.onDraw(canvas);
        fController.onConfigure();

        this->onConfigure();
    }

    void onConfigure() {
        ImGui::SetNextWindowSize(ImVec2(300, 0));
        ImGui::Begin("Save/Load");

        ImGui::PushItemWidth(-1);
        ImGui::InputText("##value", fInputBuf, 256);

        if (ImGui::Button("Save")) {
            EditorSerializer::Serialize(&fAnimation, fInputBuf);
        }
        ImGui::SameLine();
        if (ImGui::Button("Load")) {
            fAnimation = EditorSerializer::Deserialize(fInputBuf);
            fController = EditorController(&fAnimation);
        }

        ImGui::End();
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        // Set up fBones if necessary.
        if (fAnimation.fBones.size() + 1 != fBones.size()) {
            fBones.resize(fAnimation.fBones.size() + 1);
            fBones[0] = {{ 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f }};
        }

        // Copy the bone data.
        for (size_t i = 0; i < fAnimation.fBones.size(); i ++) {
            fBones[i + 1] = fAnimation.fBones[i].fBone;
        }

        // Animate the controller.
        if (fLastTick > -1.0f) {
            float delta = timer.secs() - fLastTick;
            fController.onAnimate(delta);
        }
        fLastTick = timer.secs();

        return true;
    }

    Sample::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) override {
        return new EditorClick(this);
    }

    bool onClick(Click* click) override {
        switch (click->fState) {
            case Sample::Click::kDown_State: {
                fController.onMouseDown((EditorClick*) click);
                break;
            }
            case Sample::Click::kMoved_State: {
                fController.onMouseMove((EditorClick*) click);
                break;
            }
            case Sample::Click::kUp_State: {
                fController.onMouseUp((EditorClick*) click);
                break;
            }
        }
        return true;
    }

    sk_sp<SkVertices> generateQuad(float width, float height,
                                   int xTessellations, int yTessellations) {
        // Allocate space for the data.
        std::vector<SkPoint> positions;
        std::vector<SkVertices::BoneIndices> boneIndices;
        std::vector<SkVertices::BoneWeights> boneWeights;
        std::vector<uint16_t> indices;

        // Generate the tessellations.
        for (int i = 0; i <= yTessellations; i ++) {
            for (int j = 0; j <= xTessellations; j ++) {
                float x = width * j / xTessellations;
                float y = height * i / yTessellations;
                SkPoint p = SkPoint::Make(x, y);

                // Positions and texs are the same.
                positions.push_back(SkPoint::Make(x, y));

                // Get the 4 highest weights.
                SkVertices::BoneIndices bIndices = {{ 0, 0, 0, 0 }};
                SkVertices::BoneWeights bWeights = {{ 0.0f, 0.0f, 0.0f, 0.0f }};
                for (size_t k = 0; k < fAnimation.fBones.size(); k ++) {
                    EditorBoneData& bone = fAnimation.fBones[k];

                    int index = k + 1;
                    float weight = bone.fFunction->func()(bone.fBindPosition, p);

                    // Look for a weight to replace.
                    int minIndex = -1;
                    float minWeight = weight;
                    for (int l = 0; l < 4; l ++) {
                        if (bWeights[l] < minWeight) {
                            minIndex = l;
                            minWeight = bWeights[l];
                        }
                    }

                    // Set the index and weight data.
                    if (minIndex != -1) {
                        bIndices[minIndex] = index;
                        bWeights[minIndex] = weight;
                    }
                }

                // Normalize the weights.
                float weightLen = 0.0f;
                for (int k = 0; k < 4; k ++) {
                    weightLen += bWeights[k];
                }
                if (weightLen > 0.0f) {
                    for (int k = 0; k < 4; k ++) {
                        bWeights[k] /= weightLen;
                    }
                }

                // Add the bone attribute data.
                boneIndices.push_back(bIndices);
                boneWeights.push_back(bWeights);

                // Calculate the indices.
                if (i < yTessellations && j < xTessellations) {
                    indices.push_back(i * (yTessellations + 1) + j);
                    indices.push_back(i * (yTessellations + 1) + j + 1);
                    indices.push_back((i + 1) * (yTessellations + 1) + j);
                    indices.push_back(i * (yTessellations + 1) + j + 1);
                    indices.push_back((i + 1) * (yTessellations + 1) + j);
                    indices.push_back((i + 1) * (yTessellations + 1) + j + 1);
                }
            }
        }

        // Generate the SkVertices.
        return SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode,
                                    positions.size(),
                                    positions.data(),
                                    positions.data(), // Positions and texs are the same.
                                    nullptr,
                                    boneIndices.data(),
                                    boneWeights.data(),
                                    indices.size(),
                                    indices.data());
}

private:
    typedef Sample INHERITED;

    EditorAnimation  fAnimation;
    EditorController fController;
    EditorContent    fContent;

    sk_sp<SkVertices>             fVertices;
    std::vector<SkVertices::Bone> fBones;

    sk_sp<SkSurface> fSurface;

    float fLastTick;

    char fInputBuf[256];
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new EditorView(); )
