/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"

#include "SkCanvas.h"
#include "SkColor.h"
#include "SkMatrix.h"
#include "SkMatrix44.h"
#include "SkPath.h"
#include "SkPoint3.h"
#include "SkShader.h"
#include "SkShaderBase.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

#include "SkView.h"
#include "sk_app/Window.h"

#include "effects/GrGLTFLightingFragmentProcessor.h"



#include <iostream>


// For now just define the shader in the sample file to keep footprint small.
class SK_API SkGLTFLightingShader {
public:
    static sk_sp<SkShader> Make(const SkPoint3& lightPos,
                                const SkScalar& viewerDistance,
                                const SkScalar& roughness,
                                const SkScalar& metallicness);
};


// The actual sample code
class SampleGLTF : public SampleView {

public:

    SampleGLTF()
        : fLightPos(SkPoint3::Make(0, 0, 1000))
        , fViewPos(SkPoint3::Make(0, 0, 1000))
        , fRoughness(1.0)
        , fMetallicness(0.0) { }

    SkView::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modifiers) override;

protected:

    bool onQuery(SkEvent* evt) override;

    bool onClick(SkView::Click* click) override;

    void onDrawContent(SkCanvas* canvas) override {
        // Get the current CTM, since we will treat that transform as the view
        // transform; for lights relative to the geometry, we must then apply
        // the matrix explicitly before creating the shader.
        //const SkMatrix& ctm = canvas->getTotalMatrix();

        //SkPoint lightPos;
        //ctm.mapXY(600, 600, &lightPos);

        //SkScalar viewerDistance = 100;

        std::cout << "Vz = " << fViewPos.fZ << ", L = " << fLightPos.fX << " " << fLightPos.fY << " " << fLightPos.fZ << std::endl;
        std::cout << "r = " << fRoughness << ", m = " << fMetallicness << std::endl;

        SkPaint paint;
        paint.setColor(SkColorSetRGB(128, 32, 32));
        paint.setAntiAlias(true);

        sk_sp<SkShader> gltf = SkGLTFLightingShader::Make(fLightPos, fViewPos.fZ, fRoughness, fMetallicness);
        paint.setShader(gltf);

        canvas->drawPaint(paint);
    }

private:
    class Click;

    SkPoint3 fLightPos;
    SkPoint3 fViewPos;

    SkScalar fRoughness;
    SkScalar fMetallicness;

    typedef SampleView INHERITED;
};

bool SampleGLTF::onQuery(SkEvent* evt) {
  if (SampleCode::TitleQ(*evt)) {
        SampleCode::TitleR(evt, "GLTF");
        return true;
    }

    // Track roughness and metallicness with keyboard events
    SkUnichar code;
    if (SampleCode::CharQ(*evt, &code)) {
        SkScalar step = 0.02;
        if (code == '-') {
            // Decrease roughness
            fRoughness -= step;
            if (fRoughness < 0.0) {
                fRoughness = 0.0;
            }
            return true;
        } else if (code == '=' || code == '+') {
            // Increase roughness
            fRoughness += step;
            if (fRoughness > 1.0) {
                fRoughness = 1.0;
            }
            return true;
        } else if (code == '[' || code == '{') {
            // Decrease metallicness
            fMetallicness -= step;
            if (fMetallicness < 0.0) {
                fMetallicness = 0.0;
            }
            return true;
        } else if (code == ']' || code == '}') {
            // Increase metallicness
            fMetallicness += step;
            if (fMetallicness > 1.0) {
                fMetallicness = 1.0;
            }
            return true;
        }
    }

    return this->INHERITED::onQuery(evt);
}

class SampleGLTF::Click : public SampleView::Click {
public:
    Click(SkView* target, SkPoint3* toUpdate, bool modifyZ)
        : SampleView::Click(target)
        , fPos(toUpdate)
        , fModifyZ(modifyZ) {}

    void doClick() {
        SkIPoint delta = fICurr - fIPrev;

        if (fModifyZ) {
            // Update Z based on length of delta vector
            SkScalar deltaLen = sqrt(delta.fX * delta.fX + delta.fY * delta.fY);
            if (abs(delta.fX) > abs(delta.fY)) {
                // Grab sign from x direction
                if (delta.fX < 0) {
                    deltaLen *= -1;
                }
            } else {
                // Grab sign from y direction
                if (delta.fY < 0) {
                    deltaLen *= -1;
                }
            }

            (*fPos).fZ += deltaLen;
        } else {
            // Adjust x and y by orthographic projection
            (*fPos).fX += delta.fX;
            (*fPos).fY += delta.fY;
        }
    }

private:
    SkPoint3* fPos;
    bool fModifyZ;
};

SkView::Click* SampleGLTF::onFindClickHandler(SkScalar x, SkScalar y, unsigned modifiers) {
    bool modifyLight = (modifiers & sk_app::Window::kShift_ModifierKey) == 0;
    bool modifyZ = (modifiers & (sk_app::Window::kControl_ModifierKey | sk_app::Window::kCommand_ModifierKey)) != 0;

    if (!modifyLight) {
        // Always modify the view distance since there's no 2D control over
        // it yet
        modifyZ = true;
    }

    SkPoint3* pos = modifyLight ? &fLightPos : &fViewPos;
    return new Click(this, pos, modifyZ);
}

bool SampleGLTF::onClick(SampleView::Click* click) {
    Click *myClick = (Click*) click;
    myClick->doClick();
    return true;
}

// GLTF Shader implementation


class SkGLTFLightingShaderImpl : public SkShaderBase {
public:

    SkGLTFLightingShaderImpl(const SkPoint3& lightPosition,
                             const SkScalar& viewerDistance,
                             const SkScalar& roughness,
                             const SkScalar& metallicness)
        : fLightPosition(lightPosition)
        , fViewerDistance(viewerDistance)
        , fRoughness(roughness)
        , fMetallicness(metallicness) {}

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&) const override;
#endif

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkGLTFLightingShader)

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writePoint3(fLightPosition);
        buffer.writeScalar(fViewerDistance);
        buffer.writeScalar(fRoughness);
        buffer.writeScalar(fMetallicness);
    }

private:

    SkPoint3 fLightPosition;
    SkScalar fViewerDistance;
    SkScalar fRoughness;
    SkScalar fMetallicness;
};

std::unique_ptr<GrFragmentProcessor> SkGLTFLightingShaderImpl::asFragmentProcessor(const GrFPArgs& args) const {
    std::unique_ptr<GrFragmentProcessor> base = GrGLTFLightingFragmentProcessor::Make(
        *(args.fDeviceSize), fLightPosition, fViewerDistance, fRoughness, fMetallicness);
    return GrFragmentProcessor::MulChildByInputAlpha(std::move(base));
}

sk_sp<SkFlattenable> SkGLTFLightingShaderImpl::CreateProc(SkReadBuffer& buffer) {
    SkPoint3 lightPos;
    buffer.readPoint3(&lightPos);

    SkScalar viewD = buffer.readScalar();
    SkScalar rough = buffer.readScalar();
    SkScalar metal = buffer.readScalar();
    return SkGLTFLightingShader::Make(lightPos, viewD, rough, metal);
}

sk_sp<SkShader> SkGLTFLightingShader::Make(const SkPoint3& lightPosition,
                                           const SkScalar& viewerDistance,
                                           const SkScalar& roughness,
                                           const SkScalar& metallicness) {
    return sk_make_sp<SkGLTFLightingShaderImpl>(lightPosition, viewerDistance,
        roughness, metallicness);
}

DEF_SAMPLE(return new SampleGLTF;)
