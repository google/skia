/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPoint3.h"
#include "include/effects/SkLightingImageFilter.h"

#define FILTER_WIDTH_SMALL  SkIntToScalar(32)
#define FILTER_HEIGHT_SMALL SkIntToScalar(32)
#define FILTER_WIDTH_LARGE  SkIntToScalar(256)
#define FILTER_HEIGHT_LARGE SkIntToScalar(256)

class LightingBaseBench : public Benchmark {
public:
    LightingBaseBench(bool small) : fIsSmall(small) { }

protected:
    void draw(int loops, SkCanvas* canvas, sk_sp<SkImageFilter> imageFilter) const {
        SkRect r = fIsSmall ? SkRect::MakeWH(FILTER_WIDTH_SMALL, FILTER_HEIGHT_SMALL) :
                              SkRect::MakeWH(FILTER_WIDTH_LARGE, FILTER_HEIGHT_LARGE);
        SkPaint paint;
        paint.setImageFilter(std::move(imageFilter));
        for (int i = 0; i < loops; i++) {
            canvas->drawRect(r, paint);
        }
    }

    static SkPoint3 GetPointLocation() {
        static SkPoint3 pointLocation = SkPoint3::Make(0, 0, SkIntToScalar(10));
        return pointLocation;
    }

    static SkPoint3 GetDistantDirection() {
        static SkScalar azimuthRad = SkDegreesToRadians(SkIntToScalar(225));
        static SkScalar elevationRad = SkDegreesToRadians(SkIntToScalar(5));
        static SkPoint3 distantDirection = SkPoint3::Make(
                                              SkScalarCos(azimuthRad) * SkScalarCos(elevationRad),
                                              SkScalarSin(azimuthRad) * SkScalarCos(elevationRad),
                                              SkScalarSin(elevationRad));
        return distantDirection;
    }

    static SkPoint3 GetSpotLocation() {
        static SkPoint3 spotLocation = SkPoint3::Make(SkIntToScalar(-10),
                                                      SkIntToScalar(-10),
                                                      SkIntToScalar(20));
        return spotLocation;
    }

    static SkPoint3 GetSpotTarget() {
        static SkPoint3 spotTarget = SkPoint3::Make(SkIntToScalar(40), SkIntToScalar(40), 0);
        return spotTarget;
    }

    static SkScalar GetSpotExponent() {
        static SkScalar spotExponent = SK_Scalar1;
        return spotExponent;
    }

    static SkScalar GetCutoffAngle() {
        static SkScalar cutoffAngle = SkIntToScalar(15);
        return cutoffAngle;
    }

    static SkScalar GetKd() {
        static SkScalar kd = SkIntToScalar(2);
        return kd;
    }

    static SkScalar GetKs() {
        static SkScalar ks = SkIntToScalar(1);
        return ks;
    }

    static SkScalar GetShininess() {
        static SkScalar shininess = SkIntToScalar(8);
        return shininess;
    }

    static SkScalar GetSurfaceScale() {
        static SkScalar surfaceScale = SkIntToScalar(1);
        return surfaceScale;
    }

    static SkColor GetWhite() {
        static SkColor white(0xFFFFFFFF);
        return white;
    }

    bool fIsSmall;
    typedef Benchmark INHERITED;
};

class LightingPointLitDiffuseBench : public LightingBaseBench {
public:
    LightingPointLitDiffuseBench(bool small) : INHERITED(small) { }

protected:
    const char* onGetName() override {
        return fIsSmall ? "lightingpointlitdiffuse_small" : "lightingpointlitdiffuse_large";
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        draw(loops, canvas, SkLightingImageFilter::MakePointLitDiffuse(GetPointLocation(),
                                                                       GetWhite(),
                                                                       GetSurfaceScale(),
                                                                       GetKd(),
                                                                       nullptr));
    }

private:
    typedef LightingBaseBench INHERITED;
};

class LightingDistantLitDiffuseBench : public LightingBaseBench {
public:
    LightingDistantLitDiffuseBench(bool small) : INHERITED(small) { }

protected:
    const char* onGetName() override {
        return fIsSmall ? "lightingdistantlitdiffuse_small" : "lightingdistantlitdiffuse_large";
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        draw(loops, canvas, SkLightingImageFilter::MakeDistantLitDiffuse(GetDistantDirection(),
                                                                         GetWhite(),
                                                                         GetSurfaceScale(),
                                                                         GetKd(),
                                                                         nullptr));
    }

private:
    typedef LightingBaseBench INHERITED;
};

class LightingSpotLitDiffuseBench : public LightingBaseBench {
public:
    LightingSpotLitDiffuseBench(bool small) : INHERITED(small) { }

protected:
    const char* onGetName() override {
        return fIsSmall ? "lightingspotlitdiffuse_small" : "lightingspotlitdiffuse_large";
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        draw(loops, canvas, SkLightingImageFilter::MakeSpotLitDiffuse(GetSpotLocation(),
                                                                       GetSpotTarget(),
                                                                       GetSpotExponent(),
                                                                       GetCutoffAngle(),
                                                                       GetWhite(),
                                                                       GetSurfaceScale(),
                                                                       GetKd(),
                                                                       nullptr));
    }

private:
    typedef LightingBaseBench INHERITED;
};

class LightingPointLitSpecularBench : public LightingBaseBench {
public:
    LightingPointLitSpecularBench(bool small) : INHERITED(small) { }

protected:
    const char* onGetName() override {
        return fIsSmall ? "lightingpointlitspecular_small" : "lightingpointlitspecular_large";
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        draw(loops, canvas, SkLightingImageFilter::MakePointLitSpecular(GetPointLocation(),
                                                                        GetWhite(),
                                                                        GetSurfaceScale(),
                                                                        GetKs(),
                                                                        GetShininess(),
                                                                        nullptr));
    }

private:
    typedef LightingBaseBench INHERITED;
};

class LightingDistantLitSpecularBench : public LightingBaseBench {
public:
    LightingDistantLitSpecularBench(bool small) : INHERITED(small) { }

protected:
    const char* onGetName() override {
        return fIsSmall ? "lightingdistantlitspecular_small" : "lightingdistantlitspecular_large";
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        draw(loops, canvas, SkLightingImageFilter::MakeDistantLitSpecular(GetDistantDirection(),
                                                                          GetWhite(),
                                                                          GetSurfaceScale(),
                                                                          GetKs(),
                                                                          GetShininess(),
                                                                          nullptr));
    }

private:
    typedef LightingBaseBench INHERITED;
};

class LightingSpotLitSpecularBench : public LightingBaseBench {
public:
    LightingSpotLitSpecularBench(bool small) : INHERITED(small) { }

protected:
    const char* onGetName() override {
        return fIsSmall ? "lightingspotlitspecular_small" : "lightingspotlitspecular_large";
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        draw(loops, canvas, SkLightingImageFilter::MakeSpotLitSpecular(GetSpotLocation(),
                                                                       GetSpotTarget(),
                                                                       GetSpotExponent(),
                                                                       GetCutoffAngle(),
                                                                       GetWhite(),
                                                                       GetSurfaceScale(),
                                                                       GetKs(),
                                                                       GetShininess(),
                                                                       nullptr));
    }

private:
    typedef LightingBaseBench INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new LightingPointLitDiffuseBench(true); )
DEF_BENCH( return new LightingPointLitDiffuseBench(false); )
DEF_BENCH( return new LightingDistantLitDiffuseBench(true); )
DEF_BENCH( return new LightingDistantLitDiffuseBench(false); )
DEF_BENCH( return new LightingSpotLitDiffuseBench(true); )
DEF_BENCH( return new LightingSpotLitDiffuseBench(false); )
DEF_BENCH( return new LightingPointLitSpecularBench(true); )
DEF_BENCH( return new LightingPointLitSpecularBench(false); )
DEF_BENCH( return new LightingDistantLitSpecularBench(true); )
DEF_BENCH( return new LightingDistantLitSpecularBench(false); )
DEF_BENCH( return new LightingSpotLitSpecularBench(true); )
DEF_BENCH( return new LightingSpotLitSpecularBench(false); )
