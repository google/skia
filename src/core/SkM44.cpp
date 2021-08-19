/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/private/SkVx.h"

#include "src/core/SkMatrixInvert.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkPathPriv.h"

using sk4f = skvx::Vec<4, float>;
using sk2f = skvx::Vec<2, float>;

bool SkM44::operator==(const SkM44& other) const {
    if (this == &other) {
        return true;
    }

    sk4f a0 = sk4f::Load(fMat +  0);
    sk4f a1 = sk4f::Load(fMat +  4);
    sk4f a2 = sk4f::Load(fMat +  8);
    sk4f a3 = sk4f::Load(fMat + 12);

    sk4f b0 = sk4f::Load(other.fMat +  0);
    sk4f b1 = sk4f::Load(other.fMat +  4);
    sk4f b2 = sk4f::Load(other.fMat +  8);
    sk4f b3 = sk4f::Load(other.fMat + 12);

    auto eq = (a0 == b0) & (a1 == b1) & (a2 == b2) & (a3 == b3);
    return (eq[0] & eq[1] & eq[2] & eq[3]) == ~0;
}

static void transpose_arrays(SkScalar dst[], const SkScalar src[]) {
    dst[0]  = src[0]; dst[1]  = src[4]; dst[2]  = src[8];  dst[3]  = src[12];
    dst[4]  = src[1]; dst[5]  = src[5]; dst[6]  = src[9];  dst[7]  = src[13];
    dst[8]  = src[2]; dst[9]  = src[6]; dst[10] = src[10]; dst[11] = src[14];
    dst[12] = src[3]; dst[13] = src[7]; dst[14] = src[11]; dst[15] = src[15];
}

void SkM44::getRowMajor(SkScalar v[]) const {
    transpose_arrays(v, fMat);
}

SkM44& SkM44::setConcat(const SkM44& a, const SkM44& b) {
    sk4f c0 = sk4f::Load(a.fMat +  0);
    sk4f c1 = sk4f::Load(a.fMat +  4);
    sk4f c2 = sk4f::Load(a.fMat +  8);
    sk4f c3 = sk4f::Load(a.fMat + 12);

    auto compute = [&](sk4f r) {
        return c0*r[0] + (c1*r[1] + (c2*r[2] + c3*r[3]));
    };

    sk4f m0 = compute(sk4f::Load(b.fMat +  0));
    sk4f m1 = compute(sk4f::Load(b.fMat +  4));
    sk4f m2 = compute(sk4f::Load(b.fMat +  8));
    sk4f m3 = compute(sk4f::Load(b.fMat + 12));

    m0.store(fMat +  0);
    m1.store(fMat +  4);
    m2.store(fMat +  8);
    m3.store(fMat + 12);
    return *this;
}

SkM44& SkM44::preConcat(const SkMatrix& b) {
    sk4f c0 = sk4f::Load(fMat +  0);
    sk4f c1 = sk4f::Load(fMat +  4);
    sk4f c3 = sk4f::Load(fMat + 12);

    auto compute = [&](float r0, float r1, float r3) {
        return (c0*r0 + (c1*r1 + c3*r3));
    };

    sk4f m0 = compute(b[0], b[3], b[6]);
    sk4f m1 = compute(b[1], b[4], b[7]);
    sk4f m3 = compute(b[2], b[5], b[8]);

    m0.store(fMat +  0);
    m1.store(fMat +  4);
    m3.store(fMat + 12);
    return *this;
}

SkM44& SkM44::preTranslate(SkScalar x, SkScalar y, SkScalar z) {
    sk4f c0 = sk4f::Load(fMat +  0);
    sk4f c1 = sk4f::Load(fMat +  4);
    sk4f c2 = sk4f::Load(fMat +  8);
    sk4f c3 = sk4f::Load(fMat + 12);

    // only need to update the last column
    (c0*x + (c1*y + (c2*z + c3))).store(fMat + 12);
    return *this;
}

SkM44& SkM44::postTranslate(SkScalar x, SkScalar y, SkScalar z) {
    sk4f t = { x, y, z, 0 };
    (t * fMat[ 3] + sk4f::Load(fMat +  0)).store(fMat +  0);
    (t * fMat[ 7] + sk4f::Load(fMat +  4)).store(fMat +  4);
    (t * fMat[11] + sk4f::Load(fMat +  8)).store(fMat +  8);
    (t * fMat[15] + sk4f::Load(fMat + 12)).store(fMat + 12);
    return *this;
}

SkM44& SkM44::preScale(SkScalar x, SkScalar y) {
    sk4f c0 = sk4f::Load(fMat +  0);
    sk4f c1 = sk4f::Load(fMat +  4);

    (c0 * x).store(fMat + 0);
    (c1 * y).store(fMat + 4);
    return *this;
}

SkM44& SkM44::preScale(SkScalar x, SkScalar y, SkScalar z) {
    sk4f c0 = sk4f::Load(fMat +  0);
    sk4f c1 = sk4f::Load(fMat +  4);
    sk4f c2 = sk4f::Load(fMat +  8);

    (c0 * x).store(fMat + 0);
    (c1 * y).store(fMat + 4);
    (c2 * z).store(fMat + 8);
    return *this;
}

SkV4 SkM44::map(float x, float y, float z, float w) const {
    sk4f c0 = sk4f::Load(fMat +  0);
    sk4f c1 = sk4f::Load(fMat +  4);
    sk4f c2 = sk4f::Load(fMat +  8);
    sk4f c3 = sk4f::Load(fMat + 12);

    SkV4 v;
    (c0*x + (c1*y + (c2*z + c3*w))).store(&v.x);
    return v;
}

static SkRect map_rect_affine(const SkRect& src, const float mat[16]) {
    // When multiplied against vectors of the form <x,y,x,y>, 'flip' allows a single min(sk4f, sk4f)
    // to compute both the min and "negated" max between the xy coordinates. Once finished, another
    // multiplication produces the original max.
    const sk4f flip{1.f, 1.f, -1.f, -1.f};

    // Since z = 0 and it's assumed ther's no perspective, only load the upper 2x2 and (tx,ty) in c3
    sk4f c0 = skvx::shuffle<0,1,0,1>(sk2f::Load(mat + 0)) * flip;
    sk4f c1 = skvx::shuffle<0,1,0,1>(sk2f::Load(mat + 4)) * flip;
    sk4f c3 = skvx::shuffle<0,1,0,1>(sk2f::Load(mat + 12));

    // Compute the min and max of the four transformed corners pre-translation; then translate once
    // at the end.
    sk4f minMax = c3 + flip * min(min(c0 * src.fLeft  + c1 * src.fTop,
                                      c0 * src.fRight + c1 * src.fTop),
                                  min(c0 * src.fLeft  + c1 * src.fBottom,
                                      c0 * src.fRight + c1 * src.fBottom));

    // minMax holds (min x, min y, max x, max y) so can be copied into an SkRect expecting l,t,r,b
    SkRect r;
    minMax.store(&r);
    return r;
}

static SkRect map_rect_perspective(const SkRect& src, const float mat[16]) {
    // Like map_rect_affine, z = 0 so we can skip the 3rd column, but we do need to compute w's
    // for each corner of the src rect.
    sk4f c0 = sk4f::Load(mat + 0);
    sk4f c1 = sk4f::Load(mat + 4);
    sk4f c3 = sk4f::Load(mat + 12);

    // Unlike map_rect_affine, we do not defer the 4th column since we may need to homogeneous
    // coordinates to clip against the w=0 plane
    sk4f tl = c0 * src.fLeft  + c1 * src.fTop    + c3;
    sk4f tr = c0 * src.fRight + c1 * src.fTop    + c3;
    sk4f bl = c0 * src.fLeft  + c1 * src.fBottom + c3;
    sk4f br = c0 * src.fRight + c1 * src.fBottom + c3;

    // After clipping to w>0 and projecting to 2d, 'project' employs the same negation trick to
    // compute min and max at the same time.
    const sk4f flip{1.f, 1.f, -1.f, -1.f};
    auto project = [&flip](const sk4f& p0, const sk4f& p1, const sk4f& p2) {
        float w0 = p0[3];
        if (w0 >= SkPathPriv::kW0PlaneDistance) {
            // Unclipped, just divide by w
            return flip * skvx::shuffle<0,1,0,1>(p0) / w0;
        } else {
            auto clip = [&](const sk4f& p) {
                float w = p[3];
                if (w >= SkPathPriv::kW0PlaneDistance) {
                    float t = (SkPathPriv::kW0PlaneDistance - w0) / (w - w0);
                    sk2f c = (t * skvx::shuffle<0,1>(p) + (1.f - t) * skvx::shuffle<0,1>(p0)) /
                                  SkPathPriv::kW0PlaneDistance;

                    return flip * skvx::shuffle<0,1,0,1>(c);
                } else {
                    return sk4f(SK_ScalarInfinity);
                }
            };
            // Clip both edges leaving p0, and return the min/max of the two clipped points
            // (since clip returns infinity when both p0 and 2nd vertex have w<0, it'll
            // automatically be ignored).
            return min(clip(p1), clip(p2));
        }
    };

    // Project all 4 corners, and pass in their adjacent vertices for clipping if it has w < 0,
    // then accumulate the min and max xy's.
    sk4f minMax = flip * min(min(project(tl, tr, bl), project(tr, br, tl)),
                             min(project(br, bl, tr), project(bl, tl, br)));

    SkRect r;
    minMax.store(&r);
    return r;
}

SkRect SkMatrixPriv::MapRect(const SkM44& m, const SkRect& src) {
    const bool hasPerspective =
            m.fMat[3] != 0 || m.fMat[7] != 0 || m.fMat[11] != 0 || m.fMat[15] != 1;
    if (hasPerspective) {
        return map_rect_perspective(src, m.fMat);
    } else {
        return map_rect_affine(src, m.fMat);
    }
}

void SkM44::normalizePerspective() {
    // If the bottom row of the matrix is [0, 0, 0, not_one], we will treat the matrix as if it
    // is in perspective, even though it stills behaves like its affine. If we divide everything
    // by the not_one value, then it will behave the same, but will be treated as affine,
    // and therefore faster (e.g. clients can forward-difference calculations).
    if (fMat[15] != 1 && fMat[15] != 0 && fMat[3] == 0 && fMat[7] == 0 && fMat[11] == 0) {
        double inv = 1.0 / fMat[15];
        (sk4f::Load(fMat +  0) * inv).store(fMat +  0);
        (sk4f::Load(fMat +  4) * inv).store(fMat +  4);
        (sk4f::Load(fMat +  8) * inv).store(fMat +  8);
        (sk4f::Load(fMat + 12) * inv).store(fMat + 12);
        fMat[15] = 1.0f;
    }
}

///////////////////////////////////////////////////////////////////////////////

/** We always perform the calculation in doubles, to avoid prematurely losing
    precision along the way. This relies on the compiler automatically
    promoting our SkScalar values to double (if needed).
 */
bool SkM44::invert(SkM44* inverse) const {
    SkScalar tmp[16];
    if (SkInvert4x4Matrix(fMat, tmp) == 0.0f) {
        return false;
    }
    memcpy(inverse->fMat, tmp, sizeof(tmp));
    return true;
}

SkM44 SkM44::transpose() const {
    SkM44 trans(SkM44::kUninitialized_Constructor);
    transpose_arrays(trans.fMat, fMat);
    return trans;
}

SkM44& SkM44::setRotateUnitSinCos(SkV3 axis, SkScalar sinAngle, SkScalar cosAngle) {
    // Taken from "Essential Mathematics for Games and Interactive Applications"
    //             James M. Van Verth and Lars M. Bishop -- third edition
    SkScalar x = axis.x;
    SkScalar y = axis.y;
    SkScalar z = axis.z;
    SkScalar c = cosAngle;
    SkScalar s = sinAngle;
    SkScalar t = 1 - c;

    *this = { t*x*x + c,   t*x*y - s*z, t*x*z + s*y, 0,
              t*x*y + s*z, t*y*y + c,   t*y*z - s*x, 0,
              t*x*z - s*y, t*y*z + s*x, t*z*z + c,   0,
              0,           0,           0,           1 };
    return *this;
}

SkM44& SkM44::setRotate(SkV3 axis, SkScalar radians) {
    SkScalar len = axis.length();
    if (len > 0 && SkScalarIsFinite(len)) {
        this->setRotateUnit(axis * (SK_Scalar1 / len), radians);
    } else {
        this->setIdentity();
    }
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

void SkM44::dump() const {
    static const char* format = "|%g %g %g %g|\n"
                                "|%g %g %g %g|\n"
                                "|%g %g %g %g|\n"
                                "|%g %g %g %g|\n";
    SkDebugf(format,
             fMat[0], fMat[4], fMat[8],  fMat[12],
             fMat[1], fMat[5], fMat[9],  fMat[13],
             fMat[2], fMat[6], fMat[10], fMat[14],
             fMat[3], fMat[7], fMat[11], fMat[15]);
}

///////////////////////////////////////////////////////////////////////////////

SkM44 SkM44::RectToRect(const SkRect& src, const SkRect& dst) {
        if (src.isEmpty()) {
        return SkM44();
    } else if (dst.isEmpty()) {
        return SkM44::Scale(0.f, 0.f, 0.f);
    }

    float sx = dst.width()  / src.width();
    float sy = dst.height() / src.height();

    float tx = dst.fLeft - sx * src.fLeft;
    float ty = dst.fTop  - sy * src.fTop;

    return SkM44{sx,  0.f, 0.f, tx,
                 0.f, sy,  0.f, ty,
                 0.f, 0.f, 1.f, 0.f,
                 0.f, 0.f, 0.f, 1.f};
}

static SkV3 normalize(SkV3 v) { return v * (1.0f / v.length()); }

static SkV4 v4(SkV3 v, SkScalar w) { return {v.x, v.y, v.z, w}; }

SkM44 SkM44::LookAt(const SkV3& eye, const SkV3& center, const SkV3& up) {
    SkV3 f = normalize(center - eye);
    SkV3 u = normalize(up);
    SkV3 s = normalize(f.cross(u));

    SkM44 m(SkM44::kUninitialized_Constructor);
    if (!SkM44::Cols(v4(s, 0), v4(s.cross(f), 0), v4(-f, 0), v4(eye, 1)).invert(&m)) {
        m.setIdentity();
    }
    return m;
}

SkM44 SkM44::Perspective(float near, float far, float angle) {
    SkASSERT(far > near);

    float denomInv = sk_ieee_float_divide(1, far - near);
    float halfAngle = angle * 0.5f;
    float cot = sk_float_cos(halfAngle) / sk_float_sin(halfAngle);

    SkM44 m;
    m.setRC(0, 0, cot);
    m.setRC(1, 1, cot);
    m.setRC(2, 2, (far + near) * denomInv);
    m.setRC(2, 3, 2 * far * near * denomInv);
    m.setRC(3, 2, -1);
    return m;
}
