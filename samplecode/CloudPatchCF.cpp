/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "samplecode/Sample.h"
#include "src/shaders/SkRTShader.h"
#include "src/utils/SkPatchUtils.h"
#include "tools/skui/ModifierKey.h"

static const char* gProg = R"(
uniform float iTime;

float2x2 rot(in float a){float c = cos(a), s = sin(a);return float2x2(c,s,-s,c);}
float mag2(float2 p){return dot(p,p);}
float linstep(in float mn, in float mx, in float x){ return clamp((x - mn)/(mx - mn), 0., 1.); }

float2 disp(float t){ return float2(sin(t*0.22)*1., cos(t*0.175)*1.)*2.; }

float2 map(float3 p, float prm1, float2 bsMo)
{
    float3 p2 = p;
    p2.xy -= disp(p.z).xy;
    p.xy *= rot(sin(p.z+iTime)*(0.1 + prm1*0.05) + iTime*0.09);
    float cl = mag2(p2.xy);
    float d = 0.;
    p *= .61;
    float z = 1.;
    float trk = 1.;
    float dspAmp = 0.1 + prm1*0.2;
    for(int i = 0; i < 5; i++)
    {
        p += sin(p.zxy*0.75*trk + iTime*trk*.8)*dspAmp;
        d -= abs(dot(cos(p), sin(p.yzx))*z);
        z *= 0.57;
        trk *= 1.4;
        p = p*float3x3(0.33338, 0.56034, -0.71817, -0.87887, 0.32651, -0.15323, 0.15162, 0.69596, 0.61339)*1.93;
    }
    d = abs(d + prm1*3.)+ prm1*.3 - 2.5 + bsMo.y;
    return float2(d + cl*.2 + 0.25, cl);
}

float4 render( in float3 ro, in float3 rd, float time, float prm1, float2 bsMo )
{
    float4 rez = float4(0);
    const float ldst = 8.;
    float3 lpos = float3(disp(time + ldst)*0.5, time + ldst);
    float t = 1.5;
    float fogT = 0.;
    for(int i=0; i<130; i++)
    {
        if(rez.a > 0.99)break;

        float3 pos = ro + t*rd;
        float2 mpv = map(pos, prm1, bsMo);
        float den = clamp(mpv.x-0.3,0.,1.)*1.12;
        float dn = clamp((mpv.x + 2.),0.,3.);

        float4 col = float4(0);
        if (mpv.x > 0.6)
        {

            col = float4(sin(float3(5.,0.4,0.2) + mpv.y*0.1 +sin(pos.z*0.4)*0.5 + 1.8)*0.5 + 0.5,0.08);
            col *= den*den*den;
            col.rgb *= linstep(4.,-2.5, mpv.x)*2.3;
            float dif =  clamp((den - map(pos+.8, prm1, bsMo).x)/9., 0.001, 1. );
            dif += clamp((den - map(pos+.35, prm1, bsMo).x)/2.5, 0.001, 1. );
            col.xyz *= den*(float3(0.005,.045,.075) + 1.5*float3(0.033,0.07,0.03)*dif);
        }

        float fogC = exp(t*0.2 - 2.2);
        col.rgba += float4(0.06,0.11,0.11, 0.1)*clamp(fogC-fogT, 0., 1.);
        fogT = fogC;
        rez = rez + col*(1. - rez.a);
        t += clamp(0.5 - dn*dn*.05, 0.09, 0.3);
    }
    return clamp(rez, 0.0, 1.0);
}

float getsat(float3 c)
{
    float mi = min(min(c.x, c.y), c.z);
    float ma = max(max(c.x, c.y), c.z);
    return (ma - mi)/(ma+ 1e-7);
}

//from my "Will it blend" shader (https://www.shadertoy.com/view/lsdGzN)
float3 iLerp(in float3 a, in float3 b, in float x)
{
    float3 ic = mix(a, b, x) + float3(1e-6,0.,0.);
    float sd = abs(getsat(ic) - mix(getsat(a), getsat(b), x));
    float3 dir = normalize(float3(2.*ic.x - ic.y - ic.z, 2.*ic.y - ic.x - ic.z, 2.*ic.z - ic.y - ic.x));
    float lgt = dot(float3(1.0), ic);
    float ff = dot(dir, normalize(ic));
    ic += 1.5*dir*sd*ff*lgt;
    return clamp(ic,0.,1.);
}

void main( float x, float y, inout half4 fragColor )
{
    float2 iResolution = float2(640, 480);
    float2 fragCoord = float2(x, y);
    float4 iMouse = float4(320, 240, 0, 0);
    float2 q = fragCoord.xy/iResolution.xy;
    float2 p = (fragCoord.xy - 0.5*iResolution.xy)/iResolution.y;
    float2 bsMo = (iMouse.xy - 0.5*iResolution.xy)/iResolution.y;

    float time = iTime*3.;
    float3 ro = float3(0,0,time);

    ro += float3(sin(iTime)*0.5,sin(iTime*1.)*0.,0);

    float dspAmp = .85;
    ro.xy += disp(ro.z)*dspAmp;
    float tgtDst = 3.5;

    float3 target = normalize(ro - float3(disp(time + tgtDst)*dspAmp, time + tgtDst));
    ro.x -= bsMo.x*2.;
    float3 rightdir = normalize(cross(target, float3(0,1,0)));
    float3 updir = normalize(cross(rightdir, target));
    rightdir = normalize(cross(updir, target));
    float3 rd=normalize((p.x*rightdir + p.y*updir)*1. - target);
    rd.xy *= rot(-disp(time + 3.5).x*0.2 + bsMo.x);
    float prm1 = smoothstep(-0.4, 0.4,sin(iTime*0.3));
    float4 scn = render(ro, rd, time, prm1, bsMo);

    float3 col = scn.rgb;
    col = iLerp(col.bgr, col.rgb, clamp(1.-prm1,0.05,1.));

    col = pow(col, float3(.55,0.65,0.6))*float3(1.,.97,.9);

    col *= pow( 16.0*q.x*q.y*(1.0-q.x)*(1.0-q.y), 0.12)*0.7+0.3; //Vign

    fragColor = half4(float4( col, 1.0 ));
}
)";

static float current;

static void draw_control_points(SkCanvas* canvas, const SkPoint cubics[12]) {
    //draw control points
    SkPaint paint;
    SkPoint bottom[SkPatchUtils::kNumPtsCubic];
    SkPatchUtils::GetBottomCubic(cubics, bottom);
    SkPoint top[SkPatchUtils::kNumPtsCubic];
    SkPatchUtils::GetTopCubic(cubics, top);
    SkPoint left[SkPatchUtils::kNumPtsCubic];
    SkPatchUtils::GetLeftCubic(cubics, left);
    SkPoint right[SkPatchUtils::kNumPtsCubic];
    SkPatchUtils::GetRightCubic(cubics, right);

    paint.setColor(SK_ColorBLACK);
    paint.setStrokeWidth(0.5f);
    SkPoint corners[4] = { bottom[0], bottom[3], top[0], top[3] };
    canvas->drawPoints(SkCanvas::kLines_PointMode, 4, bottom, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, bottom + 1, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 4, top, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 4, left, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 4, right, paint);

    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, top + 1, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, left + 1, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, right + 1, paint);

    paint.setStrokeWidth(2);

    paint.setColor(SK_ColorRED);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 4, corners, paint);

    paint.setColor(SK_ColorBLUE);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, bottom + 1, paint);

    paint.setColor(SK_ColorCYAN);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, top + 1, paint);

    paint.setColor(SK_ColorYELLOW);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, left + 1, paint);

    paint.setColor(SK_ColorGREEN);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, right + 1, paint);
}

// These are actually half the total width and hieghts
const SkScalar TexWidth = 100.0f;
const SkScalar TexHeight = 100.0f;

class CloudPatchCFView : public Sample {
    sk_sp<SkShader> fShader0;
    sk_sp<SkShader> fShader1;
    sk_sp<SkShader> fShaderCompose;
    SkScalar fXFreq;
    SkScalar fYFreq;
    SkScalar fSeed;
    SkPoint  fPts[SkPatchUtils::kNumCtrlPts];
    SkScalar fTexX;
    SkScalar fTexY;
    SkScalar fTexScale;
    SkMatrix fInvMatrix;
    bool     fShowGrid = false;

public:
    CloudPatchCFView() : fXFreq(0.025f), fYFreq(0.025f), fSeed(0.0f),
                        fTexX(100.0), fTexY(50.0), fTexScale(1.0f) {
        const SkScalar s = 2;
        // The order of the colors and points is clockwise starting at upper-left corner.
        //top points
        fPts[0].set(100 * s, 100 * s);
        fPts[1].set(150 * s, 50 * s);
        fPts[2].set(250 * s, 150 * s);
        fPts[3].set(300 * s, 100 * s);
        //right points
        fPts[4].set(275 * s, 150 * s);
        fPts[5].set(350 * s, 250 * s);
        //bottom points
        fPts[6].set(300 * s, 300 * s);
        fPts[7].set(250 * s, 250 * s);
        //left points
        fPts[8].set(150 * s, 350 * s);
        fPts[9].set(100 * s, 300 * s);
        fPts[10].set(50 * s, 250 * s);
        fPts[11].set(150 * s, 150 * s);

        const SkColor colors[SkPatchUtils::kNumCorners] = {
            0xFF5555FF, 0xFF8888FF, 0xFFCCCCFF
        };
        const SkPoint points[2] = { SkPoint::Make(0.0f, 0.0f),
                                    SkPoint::Make(100.0f, 100.0f) };
        fShader0 = SkGradientShader::MakeLinear(points,
                                                  colors,
                                                  nullptr,
                                                  3,
                                                  SkTileMode::kMirror,
                                                  0,
                                                  nullptr);
    }

protected:
    SkString name() override { return SkString("CloudPatchCF"); }

    bool onChar(SkUnichar uni) override {
            switch (uni) {
                case 'g': fShowGrid = !fShowGrid; return true;
                default: break;
            }
            return false;
    }

    bool onAnimate(double nanos) override {
        fSeed += 0.005f;
        current = nanos / 1000000000.0;
        return true;
    }

    void onDrawContent(SkCanvas* canvas) override {
        if (!canvas->getTotalMatrix().invert(&fInvMatrix)) {
            return;
        }

        SkPaint paint;

        SkScalar texWidth = fTexScale * TexWidth;
        SkScalar texHeight = fTexScale * TexHeight;
        const SkPoint texCoords[SkPatchUtils::kNumCorners] = {
            { fTexX - texWidth, fTexY - texHeight},
            { fTexX + texWidth, fTexY - texHeight},
            { fTexX + texWidth, fTexY + texHeight},
            { fTexX - texWidth, fTexY + texHeight}}
        ;

        SkScalar scaleFreq = 2.0;
/*        fShader1 = SkPerlinNoiseShader::MakeImprovedNoise(fXFreq/scaleFreq, fYFreq/scaleFreq, 4,
                                                             fSeed);*/
        SkMatrix localM;
        localM.setScale(0.5, 0.5);
        localM.postTranslate(0, -50);

        sk_sp<SkData> fData = SkData::MakeUninitialized(sizeof(SkColor4f));
        float* time = (float*)fData->writable_data();
        *time = current;
        fShader1 = SkRuntimeShaderFactory(SkString(gProg), true).make(fData, &localM);

        fShaderCompose = SkShaders::Blend(SkBlendMode::kSrcOver, fShader0, fShader1);

        paint.setShader(fShaderCompose);
        paint.setColorFilter(SkColorFilters::Blend(SK_ColorGREEN, SkBlendMode::kColorBurn));

        const SkPoint* tex = texCoords;
        if (fShowGrid) {
            tex = nullptr;
        }
        canvas->drawPatch(fPts, nullptr, tex, SkBlendMode::kSrc, paint);

        draw_control_points(canvas, fPts);
    }

    class PtClick : public Click {
    public:
        int fIndex;
        PtClick(int index) : fIndex(index) {}
    };

    static bool hittest(const SkPoint& pt, SkScalar x, SkScalar y) {
        return SkPoint::Length(pt.fX - x, pt.fY - y) < SkIntToScalar(5);
    }

    Sample::Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modi) override {
        modi &= ~skui::ModifierKey::kFirstPress;  // ignore this
        if (skui::ModifierKey::kShift == modi) {
            return new PtClick(-1);
        }
        if (skui::ModifierKey::kControl == modi) {
            return new PtClick(-2);
        }
        SkPoint clickPoint = {x, y};
        fInvMatrix.mapPoints(&clickPoint, 1);
        for (size_t i = 0; i < SK_ARRAY_COUNT(fPts); i++) {
            if (hittest(fPts[i], clickPoint.fX, clickPoint.fY)) {
                return new PtClick((int)i);
            }
        }
        return nullptr;
    }

    bool onClick(Click* click) override {
        PtClick* ptClick = (PtClick*)click;
        if (ptClick->fIndex >= 0) {
            fPts[ptClick->fIndex].set(click->fCurr.fX , click->fCurr.fY );
        } else if (-1 == ptClick->fIndex) {
            SkScalar xDiff = click->fPrev.fX - click->fCurr.fX;
            SkScalar yDiff = click->fPrev.fY - click->fCurr.fY;
            fTexX += xDiff * fTexScale;
            fTexY += yDiff * fTexScale;
        } else if (-2 == ptClick->fIndex) {
            SkScalar yDiff = click->fCurr.fY - click->fPrev.fY;
            fTexScale += yDiff / 10.0f;
            fTexScale = SkTMax(0.1f, SkTMin(20.f, fTexScale));
        }
        return true;
    }

private:
    typedef Sample INHERITED;
};

DEF_SAMPLE( return new CloudPatchCFView(); )
