/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SampleEditorData_DEFINED
#define SampleEditorData_DEFINED

#include "SampleEditorCommon.h"

#include "SkPath.h"
#include "SkPoint.h"
#include "SkMatrix.h"
#include "SkVertices.h"

#include "../../third_party/externals/imgui/imgui.h"

#include <functional>
#include <vector>

// WEIGHT FUNCTIONS //////////////////////////////////////////////////////////////////////////////

typedef std::function<float(const SkPoint&)> WeightFunction;

enum EditorWeightFunctionType {
    kUniform_EditorWeightFunctionType = 0,
};

class EditorWeightFunction {
public:
    EditorWeightFunction(EditorWeightFunctionType type) : fType(type) {}
    virtual ~EditorWeightFunction() {}

    EditorWeightFunctionType type() const { return fType; }

    virtual WeightFunction func() const = 0;
    virtual void onConfigure() = 0;

    virtual EditorWeightFunction* clone() const = 0;

private:
    EditorWeightFunctionType fType;
};

class UniformEditorWeightFunction : public EditorWeightFunction {
public:
    UniformEditorWeightFunction()
            : INHERITED(kUniform_EditorWeightFunctionType)
            , fFunc()
            , fWeight(1.0f)
    {
        update();
    }

    WeightFunction func() const override {
        return fFunc;
    }

    void onConfigure() override {
        ImGui::Spacing();
        ImGui::PushItemWidth(-1);
        ImGui::SliderFloat("", &fWeight, 0.0f, 1.0f, "Weight: %.1f");
    }

    UniformEditorWeightFunction* clone() const override {
        UniformEditorWeightFunction* func = new UniformEditorWeightFunction(*this);
        func->update();
        return func;
    }

private:
    void update() {
        fFunc = [this](const SkPoint&) {
            //std::cout << fWeight << "i" << std::endl;
            return fWeight;
        };
    }

private:
    WeightFunction fFunc;

    float fWeight;

    typedef EditorWeightFunction INHERITED;
};

// PATH DATA /////////////////////////////////////////////////////////////////////////////////////

struct EditorPathCubicData {
    SkPoint *fStart;
    SkPoint fControl1;
    SkPoint fControl2;
    SkPoint fEnd;
};

struct EditorPathData {
    // Start and end position.
    SkPoint fStart;
    SkPoint* fEnd;

    // Cubic data.
    std::vector<EditorPathCubicData> fCubics;

    // Path.
    SkPath fPath;

    // Default constructor.
    EditorPathData() {}

    // Copy constructor.
    EditorPathData(const EditorPathData& that)
        : fStart(that.fStart)
        , fEnd(nullptr)
        , fCubics(that.fCubics)
        , fPath(that.fPath)
    {
        this->update();
    }

    // Copy assignment operator.
    EditorPathData& operator=(const EditorPathData& that) {
        if (&that == this) {
            return *this;
        }
        fStart = that.fStart;
        fEnd = nullptr;
        fCubics = that.fCubics;
        fPath = that.fPath;
        this->update();
        return *this;
    }

    void update() {
        fPath = SkPath();
        fPath.moveTo(fStart);
        fEnd = &fStart;
        for (EditorPathCubicData& data : fCubics) {
            data.fStart = fEnd;
            fPath.cubicTo(data.fControl1, data.fControl2, data.fEnd);
            fEnd = &data.fEnd;
        }
    }
};

// BONE DATA /////////////////////////////////////////////////////////////////////////////////////

struct EditorBoneData {
    // Primary bone data.
    SkPoint fPosition;
    float   fRotation;

    // Bind data.
    float    fBindRotation;
    SkPoint  fBindPosition;

    // Composite transforms.
    SkMatrix         fMatrix;
    SkVertices::Bone fBone;

    // Weight function.
    std::unique_ptr<EditorWeightFunction> fFunction;

    // Path.
    EditorPathData fPath;

    // Default constructor.
    EditorBoneData() {}

    // Copy constructor.
    EditorBoneData(const EditorBoneData& that)
        : fPosition(that.fPosition)
        , fRotation(that.fRotation)
        , fBindRotation(that.fBindRotation)
        , fBindPosition(that.fBindPosition)
        , fMatrix(that.fMatrix)
        , fBone(that.fBone)
        , fFunction(that.fFunction->clone())
        , fPath(that.fPath)
    {}

    // Copy assignment operator.
    EditorBoneData& operator=(const EditorBoneData& that) {
        if (&that == this) {
            return *this;
        }
        fPosition = that.fPosition;
        fRotation = that.fRotation;
        fBindRotation = that.fBindRotation;
        fBindPosition = that.fBindPosition;
        fMatrix = that.fMatrix;
        fBone = that.fBone;
        fFunction.reset(that.fFunction->clone());
        fPath = that.fPath;
        return *this;
    }

    // Set the current transform as the bind transform.
    void setBind() {
        fBindRotation = fRotation;
        fBindPosition = fPosition;
        this->update();
    }

    // Update the bone matrix. Should be called after changing any of the primary data fields.
    void update() {
        fMatrix = SkMatrix::I();

        // Transform to bone space.
        fMatrix.postTranslate(-fBindPosition.x(), -fBindPosition.y());
        fMatrix.postRotate(-fBindRotation);

        // Perform the transformations and transform back to world space.
        fMatrix.postRotate(fRotation);
        fMatrix.postTranslate(fPosition.x(), fPosition.y());

        // Update the bone.
        SkAssertResult(fMatrix.asAffine(reinterpret_cast<float*>(&fBone)));
    }

    static EditorBoneData Make(float x, float y) {
        EditorBoneData bone;

        bone.fPosition.set(x, y);
        bone.fRotation = 90.0f;
        bone.setBind();

        bone.fFunction = std::make_unique<UniformEditorWeightFunction>();

        bone.fPath.fStart = bone.fPosition;
        bone.fPath.update();

        return bone;
    }
};

// ANIMATION DATA ////////////////////////////////////////////////////////////////////////////////

struct EditorAnimation {
    SkISize fSize;
    std::vector<EditorBoneData> fBones;
    float fSpeed;

    static EditorAnimation Default() {
        EditorAnimation animation;

        animation.fSize.set(500, 500);
        animation.fBones.push_back(EditorBoneData::Make(250, 250));
        animation.fSpeed = 1.0f;

        return animation;
    }
};

#endif
