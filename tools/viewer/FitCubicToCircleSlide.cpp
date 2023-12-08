/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/private/base/SkTArray.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/viewer/ClickHandlerSlide.h"

#include <tuple>

using namespace skia_private;

// Math constants are not always defined.
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#ifndef M_SQRT2
#define M_SQRT2 1.41421356237309504880168872420969808
#endif

constexpr static int kCenterX = 300;
constexpr static int kCenterY = 325;
constexpr static int kRadius = 250;

// This sample fits a cubic to the arc between two interactive points on a circle. It also finds the
// T-coordinate of max error, and outputs it and its value in pixels. (It turns out that max error
// always occurs at T=0.21132486540519.)
//
// Press 'E' to iteratively cut the arc in half and report the improvement in max error after each
// halving. (It turns out that max error improves by exactly 64x on every halving.)
class SampleFitCubicToCircle : public ClickHandlerSlide {
public:
    SampleFitCubicToCircle() { fName = "FitCubicToCircle"; }
    void load(SkScalar w, SkScalar h) override { this->fitCubic(); }
    void draw(SkCanvas*) override;
    bool onChar(SkUnichar) override;

protected:
    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey) override;
    bool onClick(Click*) override;

private:
    void fitCubic();
    // Coordinates of two points on the unit circle. These are the two endpoints of the arc we fit.
    double fEndptsX[2] = {0, 1};
    double fEndptsY[2] = {-1, 0};

    // Fitted cubic and info, set by fitCubic().
    double fControlLength;  // Length of (p1 - p0) and/or (p3 - p2) in unit circle space.
    double fMaxErrorT;  // T value where the cubic diverges most from the true arc.
    std::array<double, 4> fCubicX;  // Screen space cubic control points.
    std::array<double, 4> fCubicY;
    double fMaxError;  // Max error (in pixels) between the cubic and the screen-space arc.
    double fTheta;  // Angle of the arc. This is only used for informational purposes.
    TArray<SkString> fInfoStrings;

    class Click;
};

// Fits a cubic to an arc on the unit circle with endpoints (x0, y0) and (x1, y1). Using the
// following 3 constraints, we arrive at the formula used in the method:
//
//   1) The endpoints and tangent directions at the endpoints must match the arc.
//   2) The cubic must be symmetric (i.e., length(p1 - p0) == length(p3 - p2)).
//   3) The height of the cubic must match the height of the arc.
//
// Returns the "control length", or length of (p1 - p0) and/or (p3 - p2).
static float fit_cubic_to_unit_circle(double x0, double y0, double x1, double y1,
                                      std::array<double, 4>* X, std::array<double, 4>* Y) {
    constexpr static double kM = -4.0/3;
    constexpr static double kA = 4*M_SQRT2/3;
    double d = x0*x1 + y0*y1;
    double c = (std::sqrt(1 + d) * kM + kA) / std::sqrt(1 - d);
    *X = {x0, x0 - y0*c, x1 + y1*c, x1};
    *Y = {y0, y0 + x0*c, y1 - x1*c, y1};
    return c;
}

static double lerp(double x, double y, double T) {
    return x + T*(y - x);
}

// Evaluates the cubic and 1st and 2nd derivatives at T.
static std::tuple<double, double, double> eval_cubic(double x[], double T) {
    // Use De Casteljau's algorithm for better accuracy and stability.
    double ab = lerp(x[0], x[1], T);
    double bc = lerp(x[1], x[2], T);
    double cd = lerp(x[2], x[3], T);
    double abc = lerp(ab, bc, T);
    double bcd = lerp(bc, cd, T);
    double abcd = lerp(abc, bcd, T);
    return {abcd, 3 * (bcd - abc) /*1st derivative.*/, 6 * (cd - 2*bc + ab) /*2nd derivative.*/};
}

// Uses newton-raphson convergence to find the point where the provided cubic diverges most from the
// unit circle. i.e., the point where the derivative of error == 0. For error we use:
//
//     error = x^2 + y^2 - 1
//     error' = 2xx' + 2yy'
//     error'' = 2xx'' + 2yy'' + 2x'^2 + 2y'^2
//
double find_max_error_T(double cubicX[4], double cubicY[4]) {
    constexpr static double kInitialT = .25;
    double T = kInitialT;
    for (int i = 0; i < 64; ++i) {
        auto [x, dx, ddx] = eval_cubic(cubicX, T);
        auto [y, dy, ddy] = eval_cubic(cubicY, T);
        double dError = 2*(x*dx + y*dy);
        double ddError = 2*(x*ddx + y*ddy + dx*dx + dy*dy);
        T -= dError / ddError;
    }
    return T;
}

void SampleFitCubicToCircle::fitCubic() {
    fInfoStrings.clear();

    std::array<double, 4> X, Y;
    // "Control length" is the length of (p1 - p0) and/or (p3 - p2) in unit circle space.
    fControlLength = fit_cubic_to_unit_circle(fEndptsX[0], fEndptsY[0], fEndptsX[1], fEndptsY[1],
                                              &X, &Y);
    fInfoStrings.push_back().printf("control length=%0.14f", fControlLength);

    fMaxErrorT = find_max_error_T(X.data(), Y.data());
    fInfoStrings.push_back().printf("max error T=%0.14f", fMaxErrorT);

    for (int i = 0; i < 4; ++i) {
        fCubicX[i] = X[i] * kRadius + kCenterX;
        fCubicY[i] = Y[i] * kRadius + kCenterY;
    }
    double errX = std::get<0>(eval_cubic(fCubicX.data(), fMaxErrorT)) - kCenterX;
    double errY = std::get<0>(eval_cubic(fCubicY.data(), fMaxErrorT)) - kCenterY;
    fMaxError = std::sqrt(errX*errX + errY*errY) - kRadius;
    fInfoStrings.push_back().printf("max error=%.5gpx", fMaxError);

    fTheta = std::atan2(fEndptsY[1], fEndptsX[1]) - std::atan2(fEndptsY[0], fEndptsX[0]);
    fTheta = std::abs(fTheta * 180/M_PI);
    if (fTheta > 180) {
        fTheta = 360 - fTheta;
    }
    fInfoStrings.push_back().printf("(theta=%.2f)", fTheta);

    SkDebugf("\n");
    for (const SkString& infoString : fInfoStrings) {
        SkDebugf("%s\n", infoString.c_str());
    }
}

void SampleFitCubicToCircle::draw(SkCanvas* canvas) {
    canvas->clear(SK_ColorBLACK);

    SkPaint circlePaint;
    circlePaint.setColor(0x80ffffff);
    circlePaint.setStyle(SkPaint::kStroke_Style);
    circlePaint.setStrokeWidth(0);
    circlePaint.setAntiAlias(true);
    canvas->drawArc(SkRect::MakeXYWH(kCenterX - kRadius, kCenterY - kRadius, kRadius * 2,
                                     kRadius * 2), 0, 360, false, circlePaint);

    SkPaint cubicPaint;
    cubicPaint.setColor(SK_ColorGREEN);
    cubicPaint.setStyle(SkPaint::kStroke_Style);
    cubicPaint.setStrokeWidth(10);
    cubicPaint.setAntiAlias(true);
    SkPath cubicPath;
    cubicPath.moveTo(fCubicX[0], fCubicY[0]);
    cubicPath.cubicTo(fCubicX[1], fCubicY[1], fCubicX[2], fCubicY[2], fCubicX[3], fCubicY[3]);
    canvas->drawPath(cubicPath, cubicPaint);

    SkPaint endpointsPaint;
    endpointsPaint.setColor(SK_ColorBLUE);
    endpointsPaint.setStrokeWidth(8);
    endpointsPaint.setAntiAlias(true);
    SkPoint points[2] = {{(float)fCubicX[0], (float)fCubicY[0]},
                         {(float)fCubicX[3], (float)fCubicY[3]}};
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, points, endpointsPaint);

    SkPaint textPaint;
    textPaint.setColor(SK_ColorWHITE);
    constexpr static float kInfoTextSize = 16;
    SkFont font(ToolUtils::DefaultTypeface(), kInfoTextSize);
    int infoY = 10 + kInfoTextSize;
    for (const SkString& infoString : fInfoStrings) {
        canvas->drawString(infoString.c_str(), 10, infoY, font, textPaint);
        infoY += kInfoTextSize * 3/2;
    }
}

class SampleFitCubicToCircle::Click : public ClickHandlerSlide::Click {
public:
    Click(int ptIdx) : fPtIdx(ptIdx) {}

    void doClick(SampleFitCubicToCircle* that) {
        double dx = fCurr.fX - kCenterX;
        double dy = fCurr.fY - kCenterY;
        double l = std::sqrt(dx*dx + dy*dy);
        that->fEndptsX[fPtIdx] = dx/l;
        that->fEndptsY[fPtIdx] = dy/l;
        if (that->fEndptsX[0] * that->fEndptsY[1] - that->fEndptsY[0] * that->fEndptsX[1] < 0) {
            std::swap(that->fEndptsX[0], that->fEndptsX[1]);
            std::swap(that->fEndptsY[0], that->fEndptsY[1]);
            fPtIdx = 1 - fPtIdx;
        }
        that->fitCubic();
    }

private:
    int fPtIdx;
};

ClickHandlerSlide::Click* SampleFitCubicToCircle::onFindClickHandler(SkScalar x, SkScalar y,
                                                                     skui::ModifierKey) {
    double dx0 = x - fCubicX[0];
    double dy0 = y - fCubicY[0];
    double dx3 = x - fCubicX[3];
    double dy3 = y - fCubicY[3];
    if (dx0*dx0 + dy0*dy0 < dx3*dx3 + dy3*dy3) {
        return new Click(0);
    } else {
        return new Click(1);
    }
}

bool SampleFitCubicToCircle::onClick(ClickHandlerSlide::Click* click) {
    Click* myClick = (Click*)click;
    myClick->doClick(this);
    return true;
}

bool SampleFitCubicToCircle::onChar(SkUnichar unichar) {
    if (unichar == 'E') {
        constexpr static double kMaxErrorT = 0.21132486540519;  // Always the same.
        // Split the arc in half until error =~0, and report the improvement after each halving.
        double lastError = -1;
        for (double theta = fTheta; lastError != 0; theta /= 2) {
            double rads = theta * M_PI/180;
            std::array<double, 4> X, Y;
            fit_cubic_to_unit_circle(1, 0, std::cos(rads), std::sin(rads), &X, &Y);
            auto [x, dx, ddx] = eval_cubic(X.data(), kMaxErrorT);
            auto [y, dy, ddy] = eval_cubic(Y.data(), kMaxErrorT);
            double error = std::sqrt(x*x + y*y) * kRadius - kRadius;
            if ((float)error <= 0) {
                error = 0;
            }
            SkDebugf("%6.2f degrees:   error= %10.5gpx", theta, error);
            if (lastError > 0) {
                SkDebugf(" (%17.14fx improvement)", lastError / error);
            }
            SkDebugf("\n");
            lastError = error;
        }
        return true;
    }
    return false;
}

DEF_SLIDE(return new SampleFitCubicToCircle;)
