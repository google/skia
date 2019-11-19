/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "src/core/SkColorFilterPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkRTShader.h"
#include "tools/Resources.h"

#include <stddef.h>

static const char* gProg = R"(
// device uniforms
uniform float2 uSize;
uniform float2 uGlowPosition;
uniform float uAspectRatio;
uniform float uCornerRadius;

// glow uniforms
uniform float2 uGlowRadius;
uniform float3 uGradientStops;
uniform float4 uGradientColor1;
uniform float4 uGradientColor2;
uniform float4 uGradientColor3;
uniform float uBlurRadius;
uniform float uOpacity;

// uniforms for constant pulsating glow when in asleep mode
uniform float uPulsateAmp;
uniform float uTime;

// line uniforms
uniform float uLineThickness;
uniform float uLineWidth;
uniform float uLineAlpha;
uniform float2 uFadeMask;
uniform float4 uLineColor;
uniform float uLinePosX;

float random(float2 st) {
    return fract(sin(dot(st.xy, float2(12.9898, 78.233))) * 43758.5453123);
}

/**
 * Ellipse functions
 */
float ellipseDist(in float2 _st, in float2 _ellipsePos, in float2 _radius) {
    float dist = length((_st - float2(0.5, 0.) - _ellipsePos) / _radius);
    return dist;
}

float4 ellipse(in float2 _st, in float2 _ellipsePos, in float2 _radius, in float2 delta, float noiseInt,
    in float3 stops, in float4 color1, in float4 color2, in float4 color3) {

    float dist = ellipseDist(_st, _ellipsePos + delta, _radius);

    dist *= ((2. * random(_st + delta) - 1.) * noiseInt + (1. - noiseInt));
    float4 color = mix(color1, color2, smoothstep(stops.x, stops.y, dist));
    color = mix(color, color3, smoothstep(stops.y, stops.z, dist));
    return color;
}

float4 glowEllipse(in float2 _st, in float2 _ellipsePos, in float2 _radius, in float3 stops, in float4 color1,
    in float4 color2, in float4 color3, in float blurRadius) {

    // Simulates kind of a blur.
    float4 color = ellipse(_st, _ellipsePos, _radius * 1.3, float2(0.), 0.005, stops, color1, color2,
        color3) * 0.25;
    color += ellipse(_st, _ellipsePos, _radius, float2(blurRadius, 0.), 0.03, stops, color1, color2,
        color3) * 0.1875;
    color += ellipse(_st, _ellipsePos, _radius, float2(-blurRadius, 0.), 0.001, stops, color1, color2,
        color3) * 0.1875;
    color += ellipse(_st, _ellipsePos, _radius, float2(0., -blurRadius), 0.03, stops, color1, color2,
        color3) * 0.1875;
    color += ellipse(_st, _ellipsePos, _radius, float2(0., blurRadius), 0.002, stops, color1, color2,
        color3) * 0.1875;

    return color;
}

/**
 * Line functions
 */
float lineMask(float2 st, float width, float radius, float2 maskHeight) {
    // normalized coord: x = [-1,1], y = [-1,1]
    float2 centered = abs(st - 0.5) * 2.;
    float2 centeredAbs = abs(centered);

    // Line Rectangle
    float2 lines = step(1. - width, centered);
    float rect = lines.x * lines.x + lines.y * lines.y;

    // Radius corners
    float corners = step(1. - radius, min(centeredAbs.x, centeredAbs.y));
    float2 radiusCenter = float2(1. - radius);
    float circle = length(centeredAbs - radiusCenter);
    float r = (radius - width);
    float circleLine = smoothstep(1. - 0.001 / r, 1., circle / r);

    return (rect * (1. - corners) + circleLine * corners) * smoothstep(maskHeight.y, maskHeight.x,
        st.y);
}

float glowLine(float2 st, float x, float mask, float lineMargin) {
    float mixer = abs(st.x - 0.5 - x) * 2.;
    float alpha = 1. - smoothstep(0.0, lineMargin, mixer);
    return alpha * mask;
}

/**
 * Main
 */
void main(float x, float y, inout half4 color) {
    float2 st = float2(x, y);
    // ellipse that draws glow
    float4 glow = glowEllipse(st, uGlowPosition, uGlowRadius, uGradientStops, uGradientColor1,
        uGradientColor2, uGradientColor3, uBlurRadius);

    // line
    float mask = lineMask(st, uLineThickness * 2., uCornerRadius * 2., uFadeMask);
    float line = glowLine(st, uLinePosX, mask, uLineWidth);

    // time
    float pulsateVal = 1.0 - uPulsateAmp * smoothstep(-0.7, 1.0, sin(uTime * 4.0));

    // combine
    float4 result = glow * uOpacity * pulsateVal;
    color = half4(mix(result, uLineColor, line * uLineAlpha));
}


)";

static sk_sp<SkShader> gShader;

struct inputs {
    float uSize[2];
    float uGlowPosition[2];
    float uAspectRatio;
    float uCornerRadius;

    float uGlowRadius[2];
    float uGradientStops[3];
    float uGradientColor1[4];
    float uGradientColor2[4];
    float uGradientColor3[4];
    float uBlurRadius;
    float uOpacity;

    float uPulsateAmp;
    float uTime;

    float uLineThickness;
    float uLineWidth;
    float uLineAlpha;
    float uFadeMask[2];
    float uLineColor[4];
    float uLinePosX;
};

class RuntimeShaderExample : public skiagm::GM {
    sk_sp<SkData> fData;

    bool runAsBench() const override { return true; }

    SkString onShortName() override { return SkString("runtime_shader_android_example"); }

    SkISize onISize() override { return {512, 256}; }

    void onOnceBeforeDraw() override {
        // use global to pass gl persistent cache test in dm
        if (!gShader) {
            SkMatrix localM;
            localM.setRotate(90, 128, 128);

            fData = SkData::MakeUninitialized(sizeof(inputs));
            inputs* in = (inputs*)fData->writable_data();
            in->uSize[0] = 100;
            in->uSize[1] = 100;
            in->uGlowPosition[0] = 50;
            in->uGlowPosition[1] = 50;
            in->uAspectRatio = 1;
            in->uCornerRadius = 10;
            in->uGlowRadius[0] = 5;
            in->uGlowRadius[1] = 5;
            in->uGradientStops[0] = 10;
            in->uGradientStops[1] = 20;
            in->uGradientStops[2] = 30;
            in->uGradientColor1[0] = 1;
            in->uGradientColor1[1] = 0;
            in->uGradientColor1[2] = 0;
            in->uGradientColor1[3] = 1;
            in->uGradientColor2[0] = 0;
            in->uGradientColor2[1] = 1;
            in->uGradientColor2[2] = 0;
            in->uGradientColor2[3] = 1;
            in->uGradientColor3[0] = 0;
            in->uGradientColor3[1] = 0;
            in->uGradientColor3[2] = 1;
            in->uGradientColor3[3] = 1;
            in->uBlurRadius = 3;
            in->uOpacity = 1;
            in->uPulsateAmp = 0.1;
            in->uTime = 10;
            in->uLineThickness = 2;
            in->uLineWidth = 2;
            in->uLineAlpha = 1;
            in->uFadeMask[0] = 1;
            in->uFadeMask[1] = 1;
            in->uLineColor[0] = 1;
            in->uLineColor[1] = 0;
            in->uLineColor[2] = 0;
            in->uLineColor[3] = 1;
            in->uLinePosX = 20;
            gShader = SkRuntimeShaderFactory(SkString(gProg), true).make(fData, &localM);
        }
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint p;
        p.setShader(gShader);
        canvas->drawRect({0, 0, 256, 256}, p);
    }
};
DEF_GM(return new RuntimeShaderExample;)
