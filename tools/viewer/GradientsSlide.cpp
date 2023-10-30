/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "imgui.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/effects/SkGradientShader.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/viewer/Slide.h"

#include <vector>

///////////////////////////////////////////////////////////////////////////////

class GradientsSlide : public Slide {
public:
    GradientsSlide() {
        fName = "Gradients";
        fColors.push_back(SkColors::kBlue);
        fColors.push_back(SkColors::kYellow);
    }

    void drawUI() {
        ImGui::Begin("Gradient");

        ImGui::Checkbox("Dither", &fDither);

        bool premul = static_cast<bool>(fInterpolation.fInPremul);
        ImGui::Checkbox("Premul", &premul);
        fInterpolation.fInPremul = static_cast<SkGradientShader::Interpolation::InPremul>(premul);

        int hm = static_cast<int>(fInterpolation.fHueMethod);
        ImGui::Combo("Hue Method", &hm, "Shorter\0Longer\0Increasing\0Decreasing\0\0");
        fInterpolation.fHueMethod = static_cast<SkGradientShader::Interpolation::HueMethod>(hm);

        int removeIdx = -1;
        for (int i = 0; i < (int)fColors.size(); ++i) {
            ImGui::PushID(i);
            if (ImGui::Button("X")) {
                removeIdx = i;
            }
            ImGui::SameLine();
            ImGui::ColorEdit4("##Color", fColors[i].vec());
            ImGui::PopID();
        }
        if (removeIdx >= 0 && fColors.size() > 2) {
            fColors.erase(fColors.begin() + removeIdx);
        }

        if (ImGui::Button("+")) {
            fColors.push_back(SkColors::kBlack);
        }

        ImGui::End();
    }

    void draw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorGRAY);

        this->drawUI();

        SkPoint pts[2] = {{0, 0}, {256, 0}};
        SkRect r = {0, 0, 256, 32};
        SkPaint labelPaint;
        SkPaint paint;
        paint.setDither(fDither);

        canvas->save();
        canvas->translate(10, 10);

        using CS = SkGradientShader::Interpolation::ColorSpace;
        struct Config {
            CS fColorSpace;
            const char* fLabel;
        };
        static const Config kConfigs[] = {
            { CS::kDestination, "Destination" },
            { CS::kSRGB,        "sRGB" },
            { CS::kSRGBLinear,  "Linear sRGB" },
            { CS::kLab,         "CIELAB" },
            { CS::kOKLab,       "Oklab" },
            { CS::kLCH,         "LCH" },
            { CS::kOKLCH,       "Oklch" },
            { CS::kHSL,         "HSL" },
            { CS::kHWB,         "HWB" },
        };
        SkFont font = ToolUtils::DefaultFont();

        for (const Config& config : kConfigs) {
            fInterpolation.fColorSpace = config.fColorSpace;

            paint.setShader(SkGradientShader::MakeLinear(pts, fColors.data(),
                                                         SkColorSpace::MakeSRGB(), nullptr,
                                                         (int)fColors.size(), SkTileMode::kClamp,
                                                         fInterpolation, nullptr));
            canvas->drawRect(r, paint);
            canvas->drawSimpleText(config.fLabel, strlen(config.fLabel), SkTextEncoding::kUTF8,
                                   266, 20, font, labelPaint);
            canvas->translate(0, 42);
        }
        canvas->restore();
    }

private:
    std::vector<SkColor4f> fColors;
    SkGradientShader::Interpolation fInterpolation;
    bool fDither = false;
};

///////////////////////////////////////////////////////////////////////////////

DEF_SLIDE( return new GradientsSlide(); )
