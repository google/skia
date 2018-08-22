/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SampleEditorData_DEFINED
#define SampleEditorData_DEFINED

#include "SampleEditorCommon.h"

#include "SkJSON.h"
#include "SkJSONWriter.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkMatrix.h"
#include "SkVertices.h"

#include "../../third_party/externals/imgui/imgui.h"

#include <functional>
#include <vector>

// WEIGHT FUNCTIONS //////////////////////////////////////////////////////////////////////////////

typedef std::function<float(const SkPoint&, const SkPoint&)> WeightFunction;

enum EditorWeightFunctionType {
    kUniform_EditorWeightFunctionType  = 0,
    kGaussian_EditorWeightFunctionType = 1,
};

class EditorWeightFunction {
public:
    EditorWeightFunction(EditorWeightFunctionType type) : fType(type) {}
    virtual ~EditorWeightFunction() {}

    EditorWeightFunctionType type() const { return fType; }

    virtual WeightFunction func() const = 0;
    virtual void onConfigure() = 0;

    virtual EditorWeightFunction* clone() const = 0;

    virtual void serialize(SkJSONWriter& writer) const {
        writer.appendName("type");
        writer.appendS32(fType);
    }

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
        ImGui::SliderFloat("", &fWeight, 0.0f, 1.0f, "Weight: %.2f");
    }

    UniformEditorWeightFunction* clone() const override {
        UniformEditorWeightFunction* func = new UniformEditorWeightFunction(*this);
        func->update();
        return func;
    }

    void serialize(SkJSONWriter& writer) const override {
        INHERITED::serialize(writer);

        writer.appendName("weight");
        writer.appendFloat(fWeight);
    }

    static UniformEditorWeightFunction* Deserialize(const skjson::ObjectValue& funcJson) {
        using namespace skjson;

        const NumberValue* weightJson = funcJson["weight"];

        UniformEditorWeightFunction* func = new UniformEditorWeightFunction();
        func->fWeight = **weightJson;
        func->update();
        return func;
    }

private:
    void update() {
        fFunc = [this](const SkPoint&, const SkPoint&) {
            return fWeight;
        };
    }

private:
    WeightFunction fFunc;

    float fWeight;

    typedef EditorWeightFunction INHERITED;
};

class GaussianEditorWeightFunction : public EditorWeightFunction {
public:
    GaussianEditorWeightFunction()
            : INHERITED(kGaussian_EditorWeightFunctionType)
            , fFunc()
            , fWeight(1.0f)
            , fStddev(100.0f)
    {
        update();
    }

    WeightFunction func() const override {
        return fFunc;
    }

    void onConfigure() override {
        ImGui::Spacing();
        ImGui::PushItemWidth(-1);
        ImGui::SliderFloat("", &fWeight, 0.0f, 1.0f, "Weight: %.2f");
        ImGui::InputFloat("Standard Deviation", &fStddev, 0.0f, 1000.0f, "%.2f");
    }

    GaussianEditorWeightFunction* clone() const override {
        GaussianEditorWeightFunction* func = new GaussianEditorWeightFunction(*this);
        func->update();
        return func;
    }

    void serialize(SkJSONWriter& writer) const override {
        INHERITED::serialize(writer);

        writer.appendName("weight");
        writer.appendFloat(fWeight);
        writer.appendName("stddev");
        writer.appendFloat(fStddev);
    }

    static GaussianEditorWeightFunction* Deserialize(const skjson::ObjectValue& funcJson) {
        using namespace skjson;

        const NumberValue* weightJson = funcJson["weight"];
        const NumberValue* stddevJson = funcJson["stddev"];

        GaussianEditorWeightFunction* func = new GaussianEditorWeightFunction();
        func->fWeight = **weightJson;
        func->fStddev = **stddevJson;
        func->update();
        return func;
    }

private:
    void update() {
        fFunc = [this](const SkPoint& bone, const SkPoint& point) {
            float error = (point - bone).length();
            float a = 1.0f / (fStddev * SkScalarSqrt(2.0f * SK_ScalarPI));
            float ms = -0.5f * (error / fStddev) * (error / fStddev);
            return a * SkScalarExp(ms) * fWeight;
        };
    }

private:
    WeightFunction fFunc;

    float fWeight;
    float fStddev;

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

    void serialize(SkJSONWriter& writer) const {
        writer.appendFloat(fStart.x());
        writer.appendFloat(fStart.y());

        for (const EditorPathCubicData& data : fCubics) {
            writer.appendFloat(data.fControl1.x());
            writer.appendFloat(data.fControl1.y());
            writer.appendFloat(data.fControl2.x());
            writer.appendFloat(data.fControl2.y());
            writer.appendFloat(data.fEnd.x());
            writer.appendFloat(data.fEnd.y());
        }
    }

    static EditorPathData Deserialize(const skjson::ArrayValue& pathJson) {
        using namespace skjson;

        EditorPathData path;

        // Read the start position.
        const NumberValue* startxJson = pathJson[0];
        const NumberValue* startyJson = pathJson[1];
        path.fStart.set(**startxJson, **startyJson);

        // Read the cubic data.
        int size = pathJson.size();
        int i = 2;
        while (i < size) {
            const NumberValue* control1xJson = pathJson[i ++];
            const NumberValue* control1yJson = pathJson[i ++];
            const NumberValue* control2xJson = pathJson[i ++];
            const NumberValue* control2yJson = pathJson[i ++];
            const NumberValue* endxJson = pathJson[i ++];
            const NumberValue* endyJson = pathJson[i ++];
            EditorPathCubicData cubic = {
                nullptr,
                SkPoint::Make(**control1xJson, **control1yJson),
                SkPoint::Make(**control2xJson, **control2yJson),
                SkPoint::Make(**endxJson, **endyJson)
            };
            path.fCubics.push_back(cubic);
        }
        path.update();

        return path;
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

    void serialize(SkJSONWriter& writer) const {
        // Bind orientation.
        writer.appendName("bx");
        writer.appendFloat(fBindPosition.x());
        writer.appendName("by");
        writer.appendFloat(fBindPosition.y());
        writer.appendName("br");
        writer.appendFloat(fBindRotation);

        // Weight function.
        writer.appendName("func");
        writer.beginObject();
        fFunction->serialize(writer);
        writer.endObject();

        // Path.
        writer.appendName("path");
        writer.beginArray();
        fPath.serialize(writer);
        writer.endArray();
    }

    static EditorBoneData Deserialize(const skjson::ObjectValue& boneJson) {
        using namespace skjson;

        // Bind orientation.
        const NumberValue* bxJson = boneJson["bx"];
        const NumberValue* byJson = boneJson["by"];
        const NumberValue* brJson = boneJson["br"];
        EditorBoneData bone = EditorBoneData::Make(**bxJson, **byJson);
        bone.fRotation = **brJson;
        bone.setBind();

        // Weight function.
        const ObjectValue* funcJson = boneJson["func"];
        const NumberValue* typeJson = (*funcJson)["type"];
        switch (static_cast<int>(**typeJson)) {
            case kUniform_EditorWeightFunctionType: {
                bone.fFunction.reset(UniformEditorWeightFunction::Deserialize(*funcJson));
                break;
            }
            case kGaussian_EditorWeightFunctionType: {
                bone.fFunction.reset(GaussianEditorWeightFunction::Deserialize(*funcJson));
                break;
            }
            default: {
                bone.fFunction.reset(nullptr);
            }
        }

        // Path.
        const ArrayValue* pathJson = boneJson["path"];
        bone.fPath = EditorPathData::Deserialize(*pathJson);

        return bone;
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
