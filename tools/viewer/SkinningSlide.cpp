/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkinningSlide.h"

#include "SkAnimTimer.h"
#include "SkPath.h"
#include "SkPathMeasure.h"
#include "SkSurface.h"
#include "imgui.h"

#include <functional>
#include <iostream>

using namespace sk_app;

typedef std::function<float(const SkPoint&)> WeightFunction;

struct BoneData {
    // Primary bone data.
    SkPoint fPosition;
    float   fRotation;

    // Bind data.
    float    fBindRotation;
    SkPoint  fBindPosition;

    // Composite transforms.
    SkMatrix         fMatrix;
    SkVertices::Bone fBone;

    // Set the current transform as the bind transform.
    void setBind() {
        fBindRotation = fRotation;
        fBindPosition = fPosition;
        this->update();
    }

    // Update the bone matrix. Should be called after changing any of the primary data fields.
    void update() {
        fMatrix = SkMatrix::I();

        // Get the changes.
        SkVector deltaPosition = fPosition - fBindPosition;
        float deltaRotation = fRotation - fBindRotation;

        // Transform to bone space.
        fMatrix.postTranslate(-fBindPosition.x(), -fBindPosition.y());
        fMatrix.postRotate(-fBindRotation);

        // Perform the transformations.
        fMatrix.postRotate(deltaRotation);
        fMatrix.postTranslate(deltaPosition.x(), deltaPosition.y());

        // Transform back to world space.
        fMatrix.postRotate(fBindRotation);
        fMatrix.postTranslate(fBindPosition.x(), fBindPosition.y());

        // Update the bone.
        SkAssertResult(fMatrix.asAffine(reinterpret_cast<float*>(&fBone)));
    }
};

//////////////////////////////////////////////////////////////////////////////////////////////////

class SampleBone {
public:
    SampleBone(float x, float y, WeightFunction func)
            : fData()
            , fSelectState(kNone_SelectState)
            , fFunc(func)
    {
        fData.fPosition.set(x, y);
        fData.setBind();
    }

    const SkPoint& position() const { return fData.fPosition; }
    float rotation() const { return fData.fRotation; }
    const SkVertices::Bone& bone() const { return fData.fBone; }

    bool selected() const { return fSelectState != kNone_SelectState; }

    void setPosition(float x, float y) {
        fData.fPosition.set(x, y);
        fData.update();
    }

    void setRotation(float rotation) {
        fData.fRotation = rotation;
        fData.update();
    }

    void draw(SkCanvas* canvas) const {
        SkPaint paint;
        paint.setAntiAlias(true);

        // Set the transforms so that we can draw in bone space.
        canvas->save();
        canvas->concat(fData.fMatrix);
        canvas->translate(fData.fBindPosition.x(), fData.fBindPosition.y());

        // Draw the point.
        fillStyle(paint);
        canvas->drawCircle(0.0f, 0.0f, kRadius, paint);
        strokeStyle(paint);
        canvas->drawCircle(0.0f, 0.0f, kRadius, paint);

        // Draw the direction.
        SkRect directionRect = SkRect::MakeXYWH(kRadius + 3.0f, -kRadius * 0.5f, kLength, kRadius);
        fillStyle(paint);
        canvas->drawRoundRect(directionRect, 5.0f, 5.0f, paint);
        strokeStyle(paint);
        canvas->drawRoundRect(directionRect, 5.0f, 5.0f, paint);

        canvas->restore();
    }

    bool onMouseDown(float x, float y) {
        // Check if the position has been clicked.
        fMouseDiff = SkPoint::Make(x, y) - fData.fPosition;
        if (fMouseDiff.length() <= kRadius) {
            fSelectState = kPosition_SelectState;
            return true;
        }

        // Check if the rotation has been clicked.
        SkMatrix rotationMatrix = SkMatrix::I();
        rotationMatrix.setRotate(-fData.fRotation);
        SkVector rotated = rotationMatrix.mapXY(fMouseDiff.x(), fMouseDiff.y());
        if (rotated.x() >= kRadius + 3.0f && rotated.x() <= kRadius + 3.0f + kLength &&
            rotated.y() >= -kRadius * 0.5f && rotated.y() <= kRadius * 0.5f) {
            fSelectState = kRotation_SelectState;
            return true;
        }

        // Not selected.
        fSelectState = kNone_SelectState;
        return false;
    }

    bool onMouseMove(float x, float y) {
        switch (fSelectState) {
            case kNone_SelectState:
            case kPassive_SelectState: {
                break;
            }
            case kPosition_SelectState: {
                // Move the position.
                this->setPosition(x - fMouseDiff.x(), y - fMouseDiff.y());
                return true;
            }
            case kRotation_SelectState: {
                // Follow the mouse to adjust the rotation.
                SkVector diff = SkPoint::Make(x, y) - fData.fPosition;
                this->setRotation(SkRadiansToDegrees(SkScalarATan2(diff.y(), diff.x())));
                return true;
            }
        }
        return false;
    }

    bool onMouseUp(float x, float y) {
        // Change to passive select state.
        if (this->selected()) {
            fSelectState = kPassive_SelectState;
            return true;
        }

        return false;
    }

    float getWeight(const SkPoint& point) const {
        return fFunc(point);
    }

private:
    enum SelectState {
        kNone_SelectState     = 0,
        kPosition_SelectState = 1,
        kRotation_SelectState = 2,
        kPassive_SelectState  = 3,
    };

    void fillStyle(SkPaint& paint) const {
        paint.setStyle(SkPaint::kFill_Style);
        SkColor color;
        switch (fSelectState) {
            case kNone_SelectState: {
                color = 0x22000000;
                break;
            }
            case kPosition_SelectState:
            case kRotation_SelectState: {
                color = 0x4488DD88;
                break;
            }
            case kPassive_SelectState: {
                color = 0x44AAAAFF;
                break;
            }
        }
        paint.setColor(color);
    }

    void strokeStyle(SkPaint& paint) const {
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(2.0f);
        SkColor color;
        switch (fSelectState) {
            case kNone_SelectState: {
                color = 0x44000000;
                break;
            }
            case kPosition_SelectState:
            case kRotation_SelectState: {
                color = 0x8888DD88;
                break;
            }
            case kPassive_SelectState: {
                color = 0x88AAAAFF;
                break;
            }
        }
        paint.setColor(color);
    }

private:
    static constexpr float kRadius = 10.0f;
    static constexpr float kLength = 20.0f;

    BoneData fData;

    SelectState fSelectState;
    SkVector    fMouseDiff;

    WeightFunction fFunc;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

class SamplePathHandle {
public:
    SamplePathHandle(SkPoint position)
            : fPosition(position)
            , fChild(nullptr)
            , fMouseLast()
            , fDragging(false)
    {}

    const SkPoint& position() const { return fPosition; }

    void setChild(SamplePathHandle* child) {
        fChild = child;
    }

    void onDraw(SkCanvas* canvas) {
        SkPaint paint;
        paint.setAntiAlias(true);

        paint.setColor(0x44DD0000);
        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawCircle(fPosition, kRadius, paint);

        paint.setColor(0x88DD0000);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(2.0f);
        canvas->drawCircle(fPosition, kRadius, paint);
    }

    bool onMouseDown(float x, float y) {
        fMouseLast = SkPoint::Make(x, y);
        fDragging = (fMouseLast - fPosition).length() <= kRadius;
        return fDragging;
    }

    bool onMouseMove(float x, float y) {
        if (fDragging) {
            SkVector diff = SkPoint::Make(x, y) - fMouseLast;
            fPosition += diff;
            if (fChild) {
                fChild->fPosition += diff;
            }

            fMouseLast = SkPoint::Make(x, y);
        }

        return fDragging;
    }

    bool onMouseUp(float x, float y) {
        fDragging = false;
        return false;
    }

private:
    static constexpr float kRadius = 5.0f;

    SkPoint fPosition;

public:
    SamplePathHandle* fChild;

    SkPoint fMouseLast;
    bool fDragging;
};

class SamplePath {
public:
    SamplePath(float x, float y)
            : fPath()
            , fStart(SkPoint::Make(x, y))
            , fControl1(SkPoint::Make(x, y + 20.0f))
            , fControl2(SkPoint::Make(x, y + 40.0f))
            , fEnd(SkPoint::Make(x, y + 60.0f))
    {
        fStart.setChild(&fControl1);
        fEnd.setChild(&fControl2);

        update();
    }

    SamplePath(const SamplePath& that)
            : fPath(that.fPath)
            , fStart(that.fStart.position())
            , fControl1(that.fControl1.position())
            , fControl2(that.fControl2.position())
            , fEnd(that.fEnd.position())
    {
        fStart.setChild(&fControl1);
        fEnd.setChild(&fControl2);
    }

    const SkPath& path() const { return fPath; }

    void onDraw(SkCanvas* canvas) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(2.0f);

        canvas->save();

        // Draw the path.
        paint.setColor(0x44FF0000);
        canvas->drawPath(fPath, paint);

        // Draw control point connections.
        paint.setColor(0x88DD0000);
        canvas->drawLine(fStart.position(), fControl1.position(), paint);
        canvas->drawLine(fControl2.position(), fEnd.position(), paint);

        // Draw the control points.
        fStart.onDraw(canvas);
        fControl1.onDraw(canvas);
        fControl2.onDraw(canvas);
        fEnd.onDraw(canvas);

        canvas->restore();
    }

    bool onMouseDown(float x, float y) {
        bool handled = false;
        handled |= fStart.onMouseDown(x, y);
        handled |= fControl1.onMouseDown(x, y);
        handled |= fControl2.onMouseDown(x, y);
        handled |= fEnd.onMouseDown(x, y);
        return handled;
    }

    bool onMouseMove(float x, float y) {
        bool handled = false;
        handled |= fStart.onMouseMove(x, y);
        handled |= fControl1.onMouseMove(x, y);
        handled |= fControl2.onMouseMove(x, y);
        handled |= fEnd.onMouseMove(x, y);

        // If a control point has changed, reconstruct the path.
        if (handled) {
            update();
        }

        return handled;
    }

    bool onMouseUp(float x, float y) {
        bool handled = false;
        handled |= fStart.onMouseUp(x, y);
        handled |= fControl1.onMouseUp(x, y);
        handled |= fControl2.onMouseUp(x, y);
        handled |= fEnd.onMouseUp(x, y);
        return handled;
    }

private:
    void update() {
        fPath = SkPath();
        fPath.moveTo(fStart.position());
        fPath.cubicTo(fControl1.position(), fControl2.position(), fEnd.position());
    }

private:
    SkPath fPath;

    SamplePathHandle fStart;
    SamplePathHandle fControl1;
    SamplePathHandle fControl2;
    SamplePathHandle fEnd;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

class SampleAnimation {
public:
    SampleAnimation(SkISize size)
            : fSize(size)
    {}
    virtual ~SampleAnimation() {}

    virtual void onAdvance(float delta) {}
    virtual void onDraw(SkCanvas* canvas) {}
    virtual bool onMouseDown(float x, float y) { return false; }
    virtual bool onMouseMove(float x, float y) { return false; }
    virtual bool onMouseUp(float x, float y) { return false; }

    virtual SampleBone* sampleBones() { return fSampleBones.data(); }
    virtual int sampleBoneCount() { return fSampleBones.size(); }

protected:
    SkISize fSize;
    std::vector<SampleBone> fSampleBones;
};

class DefaultAnimation : public SampleAnimation {
public:
    DefaultAnimation(SkISize size)
            : INHERITED(size)
    {
        // Create bones at the bounds.
        fSampleBones.emplace_back(0.0f, 0.0f, [size](const SkPoint& point) {
            float rx = point.x() / size.width();
            float ry = point.y() / size.height();
            return (1.0f - rx) * (1.0f - ry);
        });
        fSampleBones.emplace_back(size.width(), 0.0f, [size](const SkPoint& point) {
            float rx = point.x() / size.width();
            float ry = point.y() / size.height();
            return (rx) * (1.0f - ry);
        });
        fSampleBones.emplace_back(0.0f, size.height(), [size](const SkPoint& point) {
            float rx = point.x() / size.width();
            float ry = point.y() / size.height();
            return (1.0f - rx) * (ry);
        });
        fSampleBones.emplace_back(size.width(), size.height(), [size](const SkPoint& point) {
            float rx = point.x() / size.width();
            float ry = point.y() / size.height();
            return (rx) * (ry);
        });
    }

private:
    typedef SampleAnimation INHERITED;
};

class RotateAnimation : public SampleAnimation {
public:
    RotateAnimation(SkISize size)
            : INHERITED(size)
    {
        // Create a single bone at the center.
        fSampleBones.emplace_back(size.width() / 2, size.height() / 2, [](const SkPoint&) {
            return 1.0f;
        });
    }

private:
    typedef SampleAnimation INHERITED;
};

class ScaleAnimation : public SampleAnimation {
public:
    ScaleAnimation(SkISize size)
            : INHERITED(size)
    {
        // Create bones at the bound edges.
        fSampleBones.emplace_back(size.width() / 2, 0.0f, [size](const SkPoint& point) {
            float ry = point.y() / size.height();
            return (1.0f - ry);
        });
        fSampleBones.emplace_back(size.width() / 2, size.height(), [size](const SkPoint& point) {
            float ry = point.y() / size.height();
            return (ry);
        });
        fSampleBones.emplace_back(0.0f, size.height() / 2, [size](const SkPoint& point) {
            float rx = point.x() / size.width();
            return (1.0f - rx);
        });
        fSampleBones.emplace_back(size.width(), size.height() / 2, [size](const SkPoint& point) {
            float rx = point.x() / size.width();
            return (rx);
        });
    }

private:
    typedef SampleAnimation INHERITED;
};

class WaveAnimation : public SampleAnimation {
public:
    WaveAnimation(SkISize size)
            : INHERITED(size)
            , total(0.0f)
    {
        // Create bones along the middle.
        for (int i = 0; i <= kSlices; i ++) {
            float x = size.width() * i / static_cast<float>(kSlices);
            fSampleBones.emplace_back(x, size.height() / 2, [x, size](const SkPoint& point) {
                float mean = x / size.width();
                float stddev = 1.0f / (kSlices * 2.0f);

                // Calculate the gaussian.
                float rx = point.x() / size.width();
                float weight = 1.0f / SkScalarSqrt(2.0f * SK_ScalarPI * stddev * stddev) *
                               SkScalarExp(-(rx - mean) * (rx - mean) / (2.0f * stddev * stddev));
                return weight;
            });
        }
    }

    void onAdvance(float delta) override {
        constexpr static float yOffset = 100.0f;

        total += delta * 10.0f;
        while (total >= 2.0f * SK_ScalarPI) {
            total -= 2.0f * SK_ScalarPI;
        }

        // Animate according to a sin wave.
        for (int i = 0; i <= kSlices; i ++) {
            SampleBone& sampleBone = fSampleBones[i];
            float x = fSize.width() * i / static_cast<float>(kSlices) * 2.0f;
            float y = SkScalarSin(x + total) * 50.0f;
            sampleBone.setPosition(sampleBone.position().x(), yOffset + fSize.height() * 0.5f + y);
        }
    }

private:
    constexpr static int kSlices = 8;

    float total;

    typedef SampleAnimation INHERITED;
};

class BendAnimation : public SampleAnimation {
public:
    BendAnimation(SkISize size)
            : INHERITED(size)
            , total(0.0f)
    {
        // Create a bone on each end.
        fSampleBones.emplace_back(0, size.height() / 2, [size](const SkPoint& point) {
            float rx = point.x() / size.width();
            return (1.0f - rx);
        });
        fSampleBones.emplace_back(size.width(), size.height() / 2, [size](const SkPoint& point) {
            float rx = point.x() / size.width();
            return (rx);
        });
    }

    void onAdvance(float delta) override {
        constexpr float xOffset = 50.0f;
        constexpr float yOffset = 50.0f;

        total += delta * 5.0f;
        while (total > 2.0f * SK_ScalarPI) {
            total -= 2.0f * SK_ScalarPI;
        }

        // Move the sides in.
        float s = SkScalarAbs(SkScalarSin(total)) * 10.0f;
        fSampleBones[0].setPosition(xOffset + s, yOffset + fSize.height() * 0.5f);
        fSampleBones[1].setPosition(xOffset + fSize.width() - s, yOffset + fSize.height() * 0.5f);

        // Rotate the sides.
        float r = SkScalarSin(total) * 8.0f;
        fSampleBones[0].setRotation(r);
        fSampleBones[1].setRotation(-r);
    }

private:
    float total;

    typedef SampleAnimation INHERITED;
};

class PathAnimation : public SampleAnimation {
public:
    PathAnimation(SkISize size)
            : INHERITED(size)
            , total(0.0f)
            , fPaths()
    {
        // Create bones at the bounds.
        fSampleBones.emplace_back(0.0f, 0.0f, [size](const SkPoint& point) {
            float rx = point.x() / size.width();
            float ry = point.y() / size.height();
            return (1.0f - rx) * (1.0f - ry);
        });
        fSampleBones.emplace_back(size.width(), 0.0f, [size](const SkPoint& point) {
            float rx = point.x() / size.width();
            float ry = point.y() / size.height();
            return (rx) * (1.0f - ry);
        });
        fSampleBones.emplace_back(0.0f, size.height(), [size](const SkPoint& point) {
            float rx = point.x() / size.width();
            float ry = point.y() / size.height();
            return (1.0f - rx) * (ry);
        });
        fSampleBones.emplace_back(size.width(), size.height(), [size](const SkPoint& point) {
            float rx = point.x() / size.width();
            float ry = point.y() / size.height();
            return (rx) * (ry);
        });

        // Create paths.
        fPaths.emplace_back(50.0f, 50.0f);
        fPaths.emplace_back(size.width() + 50.0f, 50.0f);
        fPaths.emplace_back(50.0f, size.height() + 50.0f);
        fPaths.emplace_back(size.width() + 50.0f, size.height() + 50.0f);
    }

    void onAdvance(float delta) override {
        total += delta;
        while (total >= 2.0f) {
            total -= 2.0f;
        }

        float t = total;
        if (t > 1.0f) {
            t = 2.0f - t;
        }

        // Move all bones.
        for (int i = 0; i < 4; i ++) {
            SkPoint pos;
            SkVector tan;
            SkPathMeasure measure(fPaths[i].path(), false);
            if (measure.getPosTan(t * measure.getLength(), &pos, &tan)) {
                fSampleBones[i].setRotation(SkRadiansToDegrees(SkScalarATan2(tan.y(), tan.x())));
                fSampleBones[i].setPosition(pos.x(), pos.y());
            }
        }
    }

    void onDraw(SkCanvas* canvas) override {
        // Draw the paths.
        for (SamplePath& samplePath : fPaths) {
            samplePath.onDraw(canvas);
        }
    }

    bool onMouseDown(float x, float y) override {
        for (SamplePath& samplePath : fPaths) {
            samplePath.onMouseDown(x, y);
        }
        return true;
    }

    bool onMouseMove(float x, float y) override {
        for (SamplePath& samplePath : fPaths) {
            samplePath.onMouseMove(x, y);
        }
        return true;
    }

    bool onMouseUp(float x, float y) override {
        for (SamplePath& samplePath : fPaths) {
            samplePath.onMouseUp(x, y);
        }
        return true;
    }

private:
    float total;

    std::vector<SamplePath> fPaths;

    typedef SampleAnimation INHERITED;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

SkinningSlide::SkinningSlide()
        : fBones()
        , fSurface(nullptr)
        , fVertices(nullptr)
        , fLOD(0.0f)
        , fMouseDown(false)
        , fDrag(false)
        , fDragStart()
        , fShowBones(true)
        , fAnimation(nullptr)
        , fLastTime(-1.0f)
        , fPlaying(true)
{
    fName = "Skinning";

    // Generate the surface.
    SkISize size = this->onSize();
    fSurface = SkSurface::MakeRasterN32Premul(size.width(), size.height());

    // Create the animation.
    fAnimation = std::make_unique<DefaultAnimation>(size);


    // Generate the quad.
    fVertices = this->generateQuad(size.width(), size.height(),
                                   size.width(), size.height(),
                                   1, 1);
}

SkinningSlide::~SkinningSlide() {}

void SkinningSlide::draw(SkCanvas* canvas) {
    // Wait for the bones to be set up.
    if (!fBones.size()) {
        return;
    }

    // Draw the surface.
    SkCanvas* surfaceCanvas = fSurface->getCanvas();
    surfaceCanvas->clear(0xFFFAFAFA);
    int surfaceState = surfaceCanvas->save();
    this->onDraw(surfaceCanvas);
    surfaceCanvas->restoreToCount(surfaceState);

    // Generate the image shader.
    SkPaint paint;
    sk_sp<SkImage> snapshot = fSurface->makeImageSnapshot();
    paint.setShader(snapshot->makeShader());

    // Draw the vertices.
    paint.setFilterQuality(SkFilterQuality::kLow_SkFilterQuality);
    canvas->drawVertices(fVertices.get(), fBones.data(), fBones.size(), SkBlendMode::kSrc, paint);

    // Draw the bones.
    if (fShowBones) {
        for (int i = 0; i < fAnimation->sampleBoneCount(); i ++) {
            SampleBone& sampleBone = fAnimation->sampleBones()[i];

            sampleBone.draw(canvas);

            // Display information about the bone if selected.
            if (sampleBone.selected()) {
                displayBoneInfo(sampleBone);
            }
        }
    }

    // Draw animation controls.
    int canvasState = canvas->save();
    fAnimation->onDraw(canvas);
    canvas->restoreToCount(canvasState);

    // Draw the controls.
    displayControls();
}

void SkinningSlide::load(SkScalar winWidth, SkScalar winHeight) {
}

void SkinningSlide::unload() {
}

bool SkinningSlide::animate(const SkAnimTimer& timer) {
    // Set up fBones if necessary.
    if (fAnimation->sampleBoneCount() + 1 != static_cast<int>(fBones.size())) {
        fBones.resize(fAnimation->sampleBoneCount() + 1);
        fBones[0] = {{ 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f }};
    }

    // Copy the bone data.
    for (int i = 0; i < fAnimation->sampleBoneCount(); i ++) {
        fBones[i + 1] = fAnimation->sampleBones()[i].bone();
    }

    // Animate the content.
    onAnimate(timer);

    // Advance the animation.
    if (fLastTime > 0.0f && fPlaying) {
        float delta = timer.secs() - fLastTime;
        fAnimation->onAdvance(delta);
    }
    fLastTime = timer.secs();

    return true;
}

bool SkinningSlide::onChar(SkUnichar c) {
    return false;
}

bool SkinningSlide::onMouse(SkScalar x, SkScalar y, Window::InputState state, uint32_t modifiers) {
    bool handled = false;
    switch (state) {
        case Window::kDown_InputState: {
            if (fShowBones) {
                for (int i = 0; i < fAnimation->sampleBoneCount(); i ++) {
                    handled |= fAnimation->sampleBones()[i].onMouseDown(x, y);
                }
            }
            handled |= fAnimation->onMouseDown(x, y);

            // If not handled, start dragging.
            if (!handled) {
                fDrag = true;
                fDragStart.set(x, y);
            }

            fMouseDown = true;
            break;
        }
        case Window::kMove_InputState: {
            if (!fMouseDown) {
                break;
            }

            if (fShowBones) {
                for (int i = 0; i < fAnimation->sampleBoneCount(); i ++) {
                    handled |= fAnimation->sampleBones()[i].onMouseMove(x, y);
                }
            }
            handled |= fAnimation->onMouseMove(x, y);

            // If dragging, move all the bones.
            if (fDrag) {
                // Move the bones.
                SkVector diff = SkPoint::Make(x, y) - fDragStart;
                for (int i = 0; i < fAnimation->sampleBoneCount(); i ++) {
                    SampleBone& sampleBone = fAnimation->sampleBones()[i];
                    SkPoint newPosition = sampleBone.position() + diff;
                    sampleBone.setPosition(newPosition.x(), newPosition.y());
                }

                // Reset the drag start position.
                fDragStart.set(x, y);
            }

            break;
        }
        case Window::kUp_InputState: {
            if (fShowBones) {
                for (int i = 0; i < fAnimation->sampleBoneCount(); i ++) {
                    handled = fAnimation->sampleBones()[i].onMouseUp(x, y);
                }
            }
            handled |= fAnimation->onMouseUp(x, y);

            // Stop dragging.
            fDrag = false;

            fMouseDown = false;
            break;
        }
    }

    return true;
}

sk_sp<SkVertices> SkinningSlide::generateQuad(float width, float height,
                                              float imageWidth, float imageHeight,
                                              int xTessellations, int yTessellations) {
    // Allocate space for the data.
    std::vector<SkPoint> positions;
    std::vector<SkPoint> texs;
    std::vector<SkVertices::BoneIndices> boneIndices;
    std::vector<SkVertices::BoneWeights> boneWeights;
    std::vector<uint16_t> indices;

    // Generate the tessellations.
    for (int i = 0; i <= yTessellations; i ++) {
        for (int j = 0; j <= xTessellations; j ++) {
            float x = width * j / xTessellations;
            float y = height * i / yTessellations;
            float tx = imageWidth * j / xTessellations;
            float ty = imageHeight * i / yTessellations;
            SkPoint p = SkPoint::Make(x, y);

            // Positions and texs are trivial.
            positions.push_back(SkPoint::Make(x, y));
            texs.push_back(SkPoint::Make(tx, ty));

            // Get the 4 highest weights.
            SkVertices::BoneIndices bIndices = {{ 0, 0, 0, 0 }};
            SkVertices::BoneWeights bWeights = {{ 0.0f, 0.0f, 0.0f, 0.0f }};
            for (int k = 0; k < fAnimation->sampleBoneCount(); k ++) {
                int index = k + 1;
                float weight = fAnimation->sampleBones()[k].getWeight(p);

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
                                texs.data(),
                                nullptr,
                                boneIndices.data(),
                                boneWeights.data(),
                                indices.size(),
                                indices.data());
}

void SkinningSlide::displayBoneInfo(SampleBone& sampleBone) {
    ImGui::SetNextWindowSize(ImVec2(0, 0));
    ImGui::Begin("Bone Data");

    // Display the bone data.
    ImGui::Text("X:        %.1f", sampleBone.position().x());
    ImGui::Text("Y:        %.1f", sampleBone.position().y());
    ImGui::Text("Rotation: %.1f", sampleBone.rotation());

    ImGui::End();
}

void SkinningSlide::displayControls() {
    static constexpr int kMaxTessellations = 32;

    bool regenerateVertices = false;

    ImGui::SetNextWindowSize(ImVec2(300, 0));
    ImGui::Begin("Controls");

    // Playback control.
    if (ImGui::Button("Play")) {
        fPlaying = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Pause")) {
        fPlaying = false;
    }
    ImGui::Spacing();

    // Bone Display.
    ImGui::Checkbox("Show Bones", &fShowBones);
    ImGui::Spacing();

    // LOD slider.
    float currentLOD = fLOD;
    ImGui::PushItemWidth(-1);
    ImGui::SliderFloat("Level of Detail", &fLOD, 0.0f, 1.0f, "LOD: %.1f");
    if (currentLOD != fLOD) {
        regenerateVertices = true;
    }
    ImGui::Spacing();

    // Animations.
    ImGui::Text("Animations");
    if (ImGui::Button("Default")) {
        regenerateVertices = true;
        fAnimation.reset(new DefaultAnimation(onSize()));
    }
    if (ImGui::Button("Rotate")) {
        regenerateVertices = true;
        fAnimation.reset(new RotateAnimation(onSize()));
    }
    if (ImGui::Button("Scale")) {
        regenerateVertices = true;
        fAnimation.reset(new ScaleAnimation(onSize()));
    }
    if (ImGui::Button("Wave")) {
        regenerateVertices = true;
        fAnimation.reset(new WaveAnimation(onSize()));
    }
    if (ImGui::Button("Bend")) {
        regenerateVertices = true;
        fAnimation.reset(new BendAnimation(onSize()));
    }
    if (ImGui::Button("Path")) {
        regenerateVertices = true;
        fAnimation.reset(new PathAnimation(onSize()));
    }

    ImGui::End();

    // Regenerate the quad if needed.
    if (regenerateVertices) {
        SkISize size = this->onSize();

        // Regenerate the vertices.
        int xTessellations = SkTMax(fLOD * kMaxTessellations, 1.0f);
        int yTessellations = SkTMax(fLOD * kMaxTessellations, 1.0f);

        fVertices = generateQuad(size.width(), size.height(),
                                 size.width(), size.height(),
                                 xTessellations, yTessellations);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////

SkISize SkinningSlide::onSize() {
    return SkISize::Make(500, 500);
}

void SkinningSlide::onAnimate(const SkAnimTimer& timer) {
}

void SkinningSlide::onDraw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(0xFF000000);

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
