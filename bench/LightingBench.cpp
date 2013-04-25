/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkBitmapSource.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkLightingImageFilter.h"

#define FILTER_WIDTH_SMALL  SkIntToScalar(32)
#define FILTER_HEIGHT_SMALL SkIntToScalar(32)
#define FILTER_WIDTH_LARGE  SkIntToScalar(256)
#define FILTER_HEIGHT_LARGE SkIntToScalar(256)

class LightingBaseBench : public SkBenchmark {
public:
    LightingBaseBench(void* param, bool small) : INHERITED(param), fIsSmall(small) {
    }

protected:
    void draw(SkCanvas* canvas, SkImageFilter* imageFilter) const {
        SkRect r = fIsSmall ? SkRect::MakeWH(FILTER_WIDTH_SMALL, FILTER_HEIGHT_SMALL) :
                              SkRect::MakeWH(FILTER_WIDTH_LARGE, FILTER_HEIGHT_LARGE);
        SkPaint paint;
        paint.setImageFilter(imageFilter)->unref();
        canvas->drawRect(r, paint);
    }

    static SkPoint3 getPointLocation() {
        static SkPoint3 pointLocation(0, 0, SkIntToScalar(10));
        return pointLocation;
    }

    static SkPoint3 getDistantDirection() {
        static SkScalar azimuthRad = SkDegreesToRadians(SkIntToScalar(225));
        static SkScalar elevationRad = SkDegreesToRadians(SkIntToScalar(5));
        static SkPoint3 distantDirection(SkScalarMul(SkScalarCos(azimuthRad),
                                                     SkScalarCos(elevationRad)),
                                         SkScalarMul(SkScalarSin(azimuthRad),
                                                     SkScalarCos(elevationRad)),
                                         SkScalarSin(elevationRad));
        return distantDirection;
    }

    static SkPoint3 getSpotLocation() {
        static SkPoint3 spotLocation(SkIntToScalar(-10), SkIntToScalar(-10), SkIntToScalar(20));
        return spotLocation;
    }

    static SkPoint3 getSpotTarget() {
        static SkPoint3 spotTarget(SkIntToScalar(40), SkIntToScalar(40), 0);
        return spotTarget;
    }

    static SkScalar getSpotExponent() {
        static SkScalar spotExponent = SK_Scalar1;
        return spotExponent;
    }

    static SkScalar getCutoffAngle() {
        static SkScalar cutoffAngle = SkIntToScalar(15);
        return cutoffAngle;
    }

    static SkScalar getKd() {
        static SkScalar kd = SkIntToScalar(2);
        return kd;
    }

    static SkScalar getKs() {
        static SkScalar ks = SkIntToScalar(1);
        return ks;
    }

    static SkScalar getShininess() {
        static SkScalar shininess = SkIntToScalar(8);
        return shininess;
    }

    static SkScalar getSurfaceScale() {
        static SkScalar surfaceScale = SkIntToScalar(1);
        return surfaceScale;
    }

    static SkColor getWhite() {
        static SkColor white(0xFFFFFFFF);
        return white;
    }

    bool fIsSmall;
    typedef SkBenchmark INHERITED;
};

class LightingPointLitDiffuseBench : public LightingBaseBench {
public:
    LightingPointLitDiffuseBench(void* param, bool small) : INHERITED(param, small) {
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fIsSmall ? "lightingpointlitdiffuse_small" : "lightingpointlitdiffuse_large";
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        draw(canvas, SkLightingImageFilter::CreatePointLitDiffuse(getPointLocation(), getWhite(),
            getSurfaceScale(), getKd()));
    }

private:
    typedef LightingBaseBench INHERITED;
};

class LightingDistantLitDiffuseBench : public LightingBaseBench {
public:
    LightingDistantLitDiffuseBench(void* param, bool small) : INHERITED(param, small) {
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fIsSmall ? "lightingdistantlitdiffuse_small" : "lightingdistantlitdiffuse_large";
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        draw(canvas, SkLightingImageFilter::CreateDistantLitDiffuse(getDistantDirection(),
            getWhite(), getSurfaceScale(), getKd()));
    }

private:
    typedef LightingBaseBench INHERITED;
};

class LightingSpotLitDiffuseBench : public LightingBaseBench {
public:
    LightingSpotLitDiffuseBench(void* param, bool small) : INHERITED(param, small) {
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fIsSmall ? "lightingspotlitdiffuse_small" : "lightingspotlitdiffuse_large";
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        draw(canvas, SkLightingImageFilter::CreateSpotLitDiffuse(getSpotLocation(),
            getSpotTarget(), getSpotExponent(), getCutoffAngle(), getWhite(), getSurfaceScale(),
            getKd()));
    }

private:
    typedef LightingBaseBench INHERITED;
};

class LightingPointLitSpecularBench : public LightingBaseBench {
public:
    LightingPointLitSpecularBench(void* param, bool small) : INHERITED(param, small) {
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fIsSmall ? "lightingpointlitspecular_small" : "lightingpointlitspecular_large";
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        draw(canvas, SkLightingImageFilter::CreatePointLitSpecular(getPointLocation(), getWhite(),
            getSurfaceScale(), getKs(), getShininess()));
    }

private:
    typedef LightingBaseBench INHERITED;
};

class LightingDistantLitSpecularBench : public LightingBaseBench {
public:
    LightingDistantLitSpecularBench(void* param, bool small) : INHERITED(param, small) {
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fIsSmall ? "lightingdistantlitspecular_small" : "lightingdistantlitspecular_large";
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        draw(canvas, SkLightingImageFilter::CreateDistantLitSpecular(getDistantDirection(),
            getWhite(), getSurfaceScale(), getKs(), getShininess()));
    }

private:
    typedef LightingBaseBench INHERITED;
};

class LightingSpotLitSpecularBench : public LightingBaseBench {
public:
    LightingSpotLitSpecularBench(void* param, bool small) : INHERITED(param, small) {
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fIsSmall ? "lightingspotlitspecular_small" : "lightingspotlitspecular_large";
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        draw(canvas, SkLightingImageFilter::CreateSpotLitSpecular(getSpotLocation(),
            getSpotTarget(), getSpotExponent(), getCutoffAngle(), getWhite(), getSurfaceScale(),
            getKs(), getShininess()));
    }

private:
    typedef LightingBaseBench INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new LightingPointLitDiffuseBench(p, true); )
DEF_BENCH( return new LightingPointLitDiffuseBench(p, false); )
DEF_BENCH( return new LightingDistantLitDiffuseBench(p, true); )
DEF_BENCH( return new LightingDistantLitDiffuseBench(p, false); )
DEF_BENCH( return new LightingSpotLitDiffuseBench(p, true); )
DEF_BENCH( return new LightingSpotLitDiffuseBench(p, false); )
DEF_BENCH( return new LightingPointLitSpecularBench(p, true); )
DEF_BENCH( return new LightingPointLitSpecularBench(p, false); )
DEF_BENCH( return new LightingDistantLitSpecularBench(p, true); )
DEF_BENCH( return new LightingDistantLitSpecularBench(p, false); )
DEF_BENCH( return new LightingSpotLitSpecularBench(p, true); )
DEF_BENCH( return new LightingSpotLitSpecularBench(p, false); )
