/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkShader.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkPointPriv.h"
#include "src/gpu/geometry/GrWangsFormula.h"
#include "src/utils/SkArcLengthParameterization.h"

namespace {

constexpr uint64_t choose(uint64_t n, uint64_t k) {
    SkASSERT(n >= k);
    uint64_t result = 1;
    for (uint64_t i = 1; i <= k; i++) {
        result *= (n + 1 - i);
        result /= i;
    }
    return result;
}

constexpr float pow(float x, int y) {
    // Negative exponents not handled: SkASSERT(y > 0)
    return y == 0 ? 1 : x * pow(x, y - 1);
}

template <size_t n, size_t k> constexpr float BernsteinCoeff(float t) {
    return choose(n, k) * pow(t, k) * pow(1 - t, n - k);
}

class MeasureBezier {
public:
    static SkScalar LineLength(const SkPoint* pts) { return (pts[1] - pts[0]).length(); }

    static SkScalar QuadLength(const SkPoint* pts) {
        if (quadIsNearlyLine(pts)) {
            return (pts[1] - pts[0]).length() + (pts[2] - pts[1]).length();
        }

        /* from http://www.malczak.linuxpl.com/blog/quadratic-bezier-curve-length/ */
        SkPoint a, b;
        a.fX = pts[0].fX - 2 * pts[1].fX + pts[2].fX;
        a.fY = pts[0].fY - 2 * pts[1].fY + pts[2].fY;
        b.fX = 2 * (pts[1].fX - pts[0].fX);
        b.fY = 2 * (pts[1].fY - pts[0].fY);
        double A = 4 * (a.fX * a.fX + a.fY * a.fY);
        double B = 4 * (a.fX * b.fX + a.fY * b.fY);
        double C = b.fX * b.fX + b.fY * b.fY;

        double Sabc = 2 * sqrt(A + B + C);
        double A_2 = sqrt(A);
        double A_32 = 2 * A * A_2;
        double C_2 = 2 * sqrt(C);
        double BA = B / A_2;

        const auto res = (A_32 * Sabc + A_2 * B * (Sabc - C_2) +
                          (4 * C * A - B * B) * log((2 * A_2 + BA + Sabc) / (BA + C_2))) /
                         (4 * A_32);
        SkASSERT(!std::isnan(res));
        return res;
    }

    static SkScalar ConicLength(const SkPoint* pts, SkScalar w) {
        constexpr SkScalar kTol = 0.25f;
        SkAutoConicToQuads conv;
        const SkPoint* quadPts = conv.computeQuads(pts, w, kTol);
        const int nquads = conv.countQuads();
        SkScalar len = 0;
        for (int i = 0; i < nquads; i++) {
            len += QuadLength(quadPts + 2 * i);
        }
        return len;
    }

    static SkScalar CubicLength(const SkPoint* pts) {
        const int nsegs = GrWangsFormula::cubic(10.f, pts);
        const float dt = 1.f / nsegs;
        SkPoint p0 = pts[0];
        float len = 0;
        for (int i = 1; i <= nsegs; i++) {
            SkPoint p1;
            SkEvalCubicAt(pts, i * dt, &p1, nullptr, nullptr);
            len += (p1 - p0).length();
            p0 = p1;
        }
        SkASSERT(!std::isnan(len));
        // constexpr SkScalar tolScale = 1;
        // const SkScalar tolSqd = SkScalarSquare(tolScale);

        // SkPoint chopped[10];
        // const int count = SkChopCubicAtInflections(pts, chopped);
        // SkScalar len = 0;
        // for (int i = 0; i < count; ++i) {
        //     SkPoint* cubic = chopped + 3 * i;
        //     len += noninflectCubicLength(cubic, tolSqd);
        // }
        return len;
    }

private:
    static bool quadIsNearlyLine(const SkPoint* pts) {
        const SkPoint v0 = pts[1] - pts[0];
        const SkPoint v1 = pts[2] - pts[1];
        const SkScalar dot = v0.dot(v1);
        // Either a line or a nearly perfect 180 (folding back on itself)
        return SkScalarNearlyZero(1 - dot) || SkScalarNearlyZero(1 + dot);
    }

    // Computes length of a non-inflecting cubic segment by approximating with quads
    static SkScalar noninflectCubicLength(const SkPoint p[4],
                                          SkScalar toleranceSqd,
                                          int sublevel = 0,
                                          bool preserveFirstTangent = true,
                                          bool preserveLastTangent = true) {
        // Notation: Point a is always p[0]. Point b is p[1] unless p[1] == p[0], in which case it
        // is p[2]. Point d is always p[3]. Point c is p[2] unless p[2] == p[3], in which case it is
        // p[1].
        SkVector ab = p[1] - p[0];
        SkVector dc = p[2] - p[3];

        if (SkPointPriv::LengthSqd(ab) < SK_ScalarNearlyZero) {
            if (SkPointPriv::LengthSqd(dc) < SK_ScalarNearlyZero) {
                // Degenerate quad with control points [p0, p0, p3]
                return (p[3] - p[0]).length();
            }
            ab = p[2] - p[0];
        }
        if (SkPointPriv::LengthSqd(dc) < SK_ScalarNearlyZero) {
            dc = p[1] - p[3];
        }

        static const SkScalar kLengthScale = 3 * SK_Scalar1 / 2;
        static const int kMaxSubdivs = 10;

        ab.scale(kLengthScale);
        dc.scale(kLengthScale);

        // c0 and c1 are extrapolations along vectors ab and dc.
        SkPoint c0 = p[0] + ab;
        SkPoint c1 = p[3] + dc;

        SkScalar dSqd = sublevel > kMaxSubdivs ? 0 : SkPointPriv::DistanceToSqd(c0, c1);
        if (dSqd < toleranceSqd) {
            SkPoint newC;
            if (preserveFirstTangent == preserveLastTangent) {
                // We used to force a split when both tangents need to be preserved and c0 != c1.
                // This introduced a large performance regression for tiny paths for no noticeable
                // quality improvement. However, we aren't quite fulfilling our contract of
                // guaranteeing the two tangent vectors and this could introduce a missed pixel in
                // GrAAHairlinePathRenderer.
                newC = (c0 + c1) * 0.5f;
            } else if (preserveFirstTangent) {
                newC = c0;
            } else {
                newC = c1;
            }

            // Quad with control points [p0, newC, p3]
            SkPoint quadPts[3] = {p[0], newC, p[3]};
            return QuadLength(quadPts);
        }

        SkPoint choppedPts[7];
        SkChopCubicAtHalf(p, choppedPts);
        return noninflectCubicLength(choppedPts + 0, toleranceSqd, sublevel + 1,
                                     preserveFirstTangent, false) +
               noninflectCubicLength(choppedPts + 3, toleranceSqd, sublevel + 1, false,
                                     preserveLastTangent);
    }
};

}  // namespace

//////////////////////////////////////////////////////////////////////////////

SkArcLengthParameterization::SkArcLengthParameterization(const SkPath& path) { build(path); }

float SkArcLengthParameterization::mapTToU(const Curve& curve, float t) const {
    SkASSERT(false);  // TODO
    return 0;
}

float SkArcLengthParameterization::mapUToT(const Curve& curve, float u) const {
    SkASSERT(curve.fIndex < fCurveInfo.size());
    const CurveInfo& info = fCurveInfo[curve.fIndex];
    return computeRelativeT(info, u);
}

SkArcLengthParameterization::Curve SkArcLengthParameterization::getCurve(float u, float* t) const {
    SkASSERT(0 <= u && u <= 1);
    for (size_t i = 0; i < fCurveInfo.size(); i++) {
        const CurveInfo& info = fCurveInfo[i];
        if (info.fUMin <= u && u <= info.fUMax) {
            if (t) {
                *t = computeRelativeT(info, u);
            }
            return toCurve(info, i);
        }
    }
    // Bug or empty path
    SkASSERT(false);
    return {};
}

SkArcLengthParameterization::Curve SkArcLengthParameterization::advance(const Curve& curve,
                                                                        float u,
                                                                        float du,
                                                                        float* t) const {
    SkASSERT(curve.fUMin <= u && u <= curve.fUMax);
    SkASSERT(du > 0);  // TODO: can easily handle du < 0
    const float nextU = std::min(1.f, u + du);
    for (size_t i = curve.fIndex; i < fCurveInfo.size(); i++) {
        const CurveInfo& info = fCurveInfo[i];
        if (info.fUMin <= nextU && nextU <= info.fUMax) {
            if (t) {
                *t = computeRelativeT(info, nextU);
            }
            return toCurve(info, i);
        }
    }
    return {};
}

bool SkArcLengthParameterization::advance(const Curve& curve, Curve* nextCurve) const {
    if (curve.fIndex < fCurveInfo.size() - 1) {
        *nextCurve = toCurve(fCurveInfo[curve.fIndex + 1], curve.fIndex + 1);
        return true;
    }
    return false;
}

SkArcLengthParameterization::Curve SkArcLengthParameterization::toCurve(const CurveInfo& info,
                                                                        size_t index) const {
    Curve curve;
    curve.fVerb = info.fVerb;
    curve.fPts = info.fPoints;
    curve.fLength = info.fLength;
    curve.fUMin = info.fUMin;
    curve.fUMax = info.fUMax;
    curve.fIndex = index;
    return curve;
}

void SkArcLengthParameterization::build(const SkPath& path) {
    BuildContext ctx;
    fPathPoints = nullptr;
    for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
        if (fPathPoints == nullptr) {
            fPathPoints = pts;
        }
        switch (verb) {
            case SkPathVerb::kLine:
                buildLine(pts, &ctx);
                break;
            case SkPathVerb::kQuad:
                buildQuad(pts, &ctx);
                break;
            case SkPathVerb::kConic:
                buildConic(pts, *w, &ctx);
                break;
            case SkPathVerb::kCubic:
                buildCubic(pts, &ctx);
                break;
            case SkPathVerb::kMove:
                SkASSERT(ctx.fPointIndex == 0 && "multi-contours not yet handled");
                continue;
            default:
                continue;
        }
    }

    fTotalLength = ctx.fTotalLength;
    fCurveInfo.swap(ctx.fCurveInfo);
    fConicQuadPts.swap(ctx.fConicQuadPts);

    // Total length now available; compute u intervals per curve.
    float umin = 0;
    for (CurveInfo& curve : fCurveInfo) {
        const float amountOfU = curve.fLength / fTotalLength;
        curve.fUMin = umin;
        curve.fUMax = umin + amountOfU;
        umin = curve.fUMax;

        if (curve.fVerb == SkPathVerb::kConic) {
            // This has to wait until all points have been pushed in case the vector
            // reallocs and moves things around.
            curve.fPoints = &fConicQuadPts[curve.fPointIndex];
            curve.fVerb = SkPathVerb::kQuad;
        } else {
            curve.fPoints = fPathPoints + curve.fPointIndex;
        }
    }
    if (!fCurveInfo.empty()) {
        // Round up to a perfect 1.0 for the end
        fCurveInfo.back().fUMax = 1.f;
    }
}

void SkArcLengthParameterization::buildLine(const SkPoint* pts, BuildContext* ctx) {
    CurveInfo info;
    // Lines are already arc length parameterized, so we don't use an approximation.
    info.fVerb = SkPathVerb::kLine;
    info.fPointIndex = ctx->fPointIndex;
    info.fLength = MeasureBezier::LineLength(pts);
    ctx->fPointIndex += 1;
    ctx->fTotalLength += info.fLength;
    ctx->fCurveInfo.push_back(std::move(info));
}

void SkArcLengthParameterization::buildQuad(const SkPoint* pts, BuildContext* ctx) {
    CurveInfo info;
    computeParameters<2>(pts, &info.fAlpha, &info.fU);
    info.fVerb = SkPathVerb::kQuad;
    info.fPointIndex = ctx->fPointIndex;
    info.fLength = MeasureBezier::QuadLength(pts);
    ctx->fPointIndex += 2;
    ctx->fTotalLength += info.fLength;
    ctx->fCurveInfo.push_back(std::move(info));
}

void SkArcLengthParameterization::buildConic(const SkPoint* pts, float w, BuildContext* ctx) {
    SkAutoConicToQuads conv;
    constexpr SkScalar kTol = 0.25f;
    const SkPoint* allQuadPts = conv.computeQuads(pts, w, kTol);
    const int nquads = conv.countQuads();
    for (int i = 0; i < nquads; i++) {
        const SkPoint* quadPts = allQuadPts + 2 * i;
        CurveInfo info;
        computeParameters<2>(quadPts, &info.fAlpha, &info.fU);
        info.fVerb = SkPathVerb::kConic;
        info.fPointIndex = ctx->fConicQuadPts.empty() ? 0 : ctx->fConicQuadPts.size() - 1;
        info.fLength = MeasureBezier::QuadLength(quadPts);
        if (i == 0) {
            ctx->fConicQuadPts.push_back(quadPts[0]);
        }
        ctx->fConicQuadPts.push_back(quadPts[1]);
        ctx->fConicQuadPts.push_back(quadPts[2]);
        ctx->fTotalLength += info.fLength;
        ctx->fCurveInfo.push_back(std::move(info));
    }

    ctx->fPointIndex += 2;
}

void SkArcLengthParameterization::buildCubic(const SkPoint* pts, BuildContext* ctx) {
    CurveInfo info;
    computeParameters<3>(pts, &info.fAlpha, &info.fU);
    info.fVerb = SkPathVerb::kCubic;
    info.fPointIndex = ctx->fPointIndex;
    info.fLength = MeasureBezier::CubicLength(pts);
    ctx->fPointIndex += 3;
    ctx->fTotalLength += info.fLength;
    ctx->fCurveInfo.push_back(std::move(info));
}

float SkArcLengthParameterization::computeRelativeT(const CurveInfo& info, float u) {
    SkASSERT(0 <= u && u <= 1);
    SkASSERT(info.fUMin <= u && u <= info.fUMax);

    // Global u -> local u
    u = (u - info.fUMin) / (info.fUMax - info.fUMin);

    if (info.fVerb == SkPathVerb::kLine) {
        return u;
    }

    // Advance until we find the piece of the piecewise approximation containing u
    size_t j = 0;
    while (u > info.fU[j + 1]) {
        j++;
        SkASSERT(j < info.fU.size());
        SkASSERT(j < info.fAlpha.size());
    }
    SkASSERT(info.fU[j] <= u && u <= info.fU[j + 1]);

    const float alpha = info.fAlpha[j];
    const float utilde = (u - info.fU[j]) / (info.fU[j + 1] - info.fU[j]);
    return T[j] + (T[j + 1] - T[j]) *
                          (((1 - alpha) * utilde) / (alpha * (1 - utilde) + (1 - alpha) * utilde));
}

namespace {
template <size_t kN> using LUT1d = std::array<float, kN>;
template <size_t kI, size_t kJ> using LUT2d = std::array<std::array<float, kJ>, kI>;
template <size_t kI, size_t kJ, size_t kK>
using LUT3d = std::array<std::array<std::array<float, kK>, kJ>, kI>;

template <int kDeg> constexpr LUT1d<2 * kDeg - 1> GenTab1a() {
    LUT1d<2 * kDeg - 1> res = {};
    for (int k = 0; k < 2 * kDeg - 1; k++) {
        res[k] = float(kDeg * kDeg) / choose(2 * kDeg - 2, k);
    }
    return res;
}

template <int kDeg> constexpr LUT2d<2 * kDeg - 1, kDeg> GenTab1b() {
    LUT2d<2 * kDeg - 1, kDeg> res = {};
    for (int k = 0; k < 2 * kDeg - 1; k++) {
        const int lmin = std::max(0, k - kDeg + 1);
        const int lmax = std::min(k, kDeg - 1);
        for (int l = lmin; l <= lmax; l++) {
            res[k][l] = choose(kDeg - 1, l) * choose(kDeg - 1, k - l);
        }
    }
    return res;
}

template <int kN, int kDeg>
constexpr LUT3d<kN, 3, 2 * kDeg + 1> GenTab2(const std::array<float, kN + 1>& T) {
    LUT3d<kN, 3, 2 * kDeg + 1> res = {};
    for (int j = 0; j < kN; j++) {
        const float t0 = T[j + 1], t1 = T[j];
        for (int r = 0; r < 3; r++) {
            const float nn = 2 * kDeg - 1 + r;
            for (int i = r; i <= 2 * kDeg - 2 + r; i++) {
                float sum = 0;  // j, r, i?
                for (int k = i + 1; k <= 2 * kDeg - 1 + r; k++) {
                    const float first = choose(nn, k) * pow(t0, k) * pow(1 - t0, nn - k);
                    const float second = choose(nn, k) * pow(t1, k) * pow(1 - t1, nn - k);
                    sum += first - second;
                }
                res[j][r][i] =
                        sum * (choose(2 * kDeg - 2, i - r) / float(choose(2 * kDeg - 2 + r, i)));
            }
        }
    }
    return res;
}

}  // namespace

template <int kDeg>
void SkArcLengthParameterization::computeParameters(const SkPoint* pts,
                                                    AlphaVals* alpha,
                                                    UVals* u) {
    // Generate LUTs
    static constexpr auto Tab1a = GenTab1a<kDeg>();
    static constexpr auto Tab1b = GenTab1b<kDeg>();
    static constexpr auto Tab2 = GenTab2<kN, kDeg>(T);

    // First compute coefficients C_k for use in evaluating K_jr.
    float C[2 * kDeg - 1];
    for (int k = 0; k < 2 * kDeg - 1; k++) {
        const int lmin = std::max(0, k - kDeg + 1);
        const int lmax = std::min(k, kDeg - 1);
        float s = 0;
        for (int l = lmin; l <= lmax; l++) {
            const float fwdiff = (pts[l + 1] - pts[l]).dot(pts[k - l + 1] - pts[k - l]);
            s += fwdiff * Tab1b[k][l];
        }
        s *= Tab1a[k];
        C[k] = s;
    }

    // Next compute K_jr.
    float K[kN * 3];
    for (int j = 0; j < kN; j++) {
        float rs[3];
        for (int r = 0; r < 3; r++) {
            float s = 0;
            for (int i = r; i <= 2 * kDeg - 2 + r; i++) {
                s += C[i - r] * Tab2[j][r][i];
            }
            s *= 1.f / (2 * kDeg - 1 + r);
            rs[r] = s;
        }
        K[j * 3 + 0] = rs[0];
        K[j * 3 + 1] = rs[1] - T[j] * rs[0];
        K[j * 3 + 2] = rs[2] - 2 * T[j] * rs[1] + T[j] * T[j] * rs[0];
    }

    // Compute alpha_i (Equation 17), A_i and u_nought (used for computing U).
    float A[kN];
    float A0 = 0, unought = 1;
    for (int i = 0; i < kN; i++) {
        const float k0 = K[i * 3 + 0], k1 = K[i * 3 + 1], k2 = K[i * 3 + 2];
        const float fwdifft = T[i + 1] - T[i], fwdifft2 = fwdifft * fwdifft;
        const float Pi = sqrtf((fwdifft2 * k0 - 2 * fwdifft * k1 + k2) / k2);
        const float alphai = Pi / (1 + Pi);
        (*alpha)[i] = alphai;

        const float rho = (1 - alphai) / alphai;
        float Ai = (rho * rho * fwdifft2 * k0 + 2 * rho * (1 - rho) * fwdifft * k1 +
                    (1 - rho) * (1 - rho) * k2) /
                   (rho * fwdifft);
        Ai = SkScalarNearlyZero(Ai) ? 0 : Ai;

        if (i == 0) {
            A0 = Ai;
            A[i] = 1;
        } else {
            const float val = sqrtf(Ai / A0);
            A[i] = val;
            unought += val;
        }
    }
    unought = 1.f / unought;

    // Compute u_i (Equation 18)
    (*u)[0] = 0;
    (*u)[kN] = 1;
    for (int i = 1; i < kN; i++) {
        float Ui = 0;
        for (int k = 0; k <= i - 1; k++) {
            Ui += A[k];
        }
        (*u)[i] = unought * Ui;
    }
}

//////////////////////////////////////////////////////////////////////////////

SkArcLengthSegmentIter::SkArcLengthSegmentIter(const SkArcLengthParameterization& param)
        : fParam(param), fU(0) {
    fCurrCurve = fParam.getCurve(0);
}

void SkArcLengthSegmentIter::getSegment(float ulen, SkPath* dst) {
    const float umin = fU, umax = std::min(1.f, fU + ulen);

    if (umax <= fCurrCurve.fUMax) {
        // Entire u interval contained in the current curve
        const float tmin = fParam.mapUToT(fCurrCurve, umin);
        const float tmax = fParam.mapUToT(fCurrCurve, umax);
        appendCurveSection(fCurrCurve, tmin, tmax, dst);
    } else {
        // u interval spans several curves.
        const float tmin = fParam.mapUToT(fCurrCurve, umin);
        appendCurveSection(fCurrCurve, tmin, 1.0f, dst);
        while (umax > fCurrCurve.fUMax && fParam.advance(fCurrCurve, &fCurrCurve)) {
            const float tmax = fParam.mapUToT(fCurrCurve, umax);
            SkASSERT(tmax <= 1.f);
            appendCurveSection(fCurrCurve, 0, tmax, dst, false);
        }
    }

    fU = umax;
}

bool SkArcLengthSegmentIter::advance(float du) {
    if (fU >= 1) {
        return false;
    }

    fCurrCurve = fParam.advance(fCurrCurve, fU, du);
    fU = std::min(1.f, fU + du);
    return true;
}

void SkArcLengthSegmentIter::appendCurveSection(const SkArcLengthParameterization::Curve& curve,
                                                float tmin,
                                                float tmax,
                                                SkPath* dst,
                                                bool moveTo) {
    const SkPoint* pts = curve.fPts;
    switch (curve.fVerb) {
        case SkPathVerb::kLine: {
            SkPoint p0 = pts[0] + (pts[1] - pts[0]) * tmin;
            SkPoint p1 = pts[0] + (pts[1] - pts[0]) * tmax;
            if (moveTo) {
                dst->moveTo(p0);
            }
            dst->lineTo(p1);
            break;
        }
        case SkPathVerb::kQuad: {
            if (tmin == 0) {
                SkASSERT(tmax < 1);
                SkPoint chopped[8];
                SkChopQuadAt(pts, chopped, tmax);
                if (moveTo) {
                    dst->moveTo(chopped[0]);
                }
                dst->quadTo(chopped[1], chopped[2]);
            } else if (tmax == 1) {
                SkASSERT(tmin > 0);
                SkPoint chopped[8];
                SkChopQuadAt(pts, chopped, tmin);

                if (moveTo) {
                    dst->moveTo(chopped[2]);
                }
                dst->quadTo(chopped[3], chopped[4]);
            } else {
                const float trel = tmin / tmax;
                SkPoint chopped[8], chopped2[8];
                SkChopQuadAt(pts, chopped, tmax);
                SkChopQuadAt(chopped, chopped2, trel);
                if (moveTo) {
                    dst->moveTo(chopped2[2]);
                }
                dst->quadTo(chopped2[3], chopped2[4]);
            }
            break;
        }
        case SkPathVerb::kCubic: {
            SkPoint dstPts[10];
            SkChopCubicAt(pts, dstPts, tmin, tmax);

            if (moveTo) {
                dst->moveTo(dstPts[3]);
            }
            dst->cubicTo(dstPts[4], dstPts[5], dstPts[6]);
            break;
        }
        default:
            SkASSERT(false);
    }
}
