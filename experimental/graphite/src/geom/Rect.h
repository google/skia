/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_geom_Rect_DEFINED
#define skgpu_geom_Rect_DEFINED

#include "experimental/graphite/src/geom/VectorTypes.h"
#include "include/core/SkRect.h"

namespace skgpu {

#define AI SK_ALWAYS_INLINE

/**
 * SIMD rect implementation. Vales are stored internally in the form: [left, top, -right, -bot].
 *
 * Some operations (e.g., intersect, inset) may return a negative or empty rect
 * (negative meaning, left >= right or top >= bot).
 *
 * Operations on a rect that is either negative or empty, while well-defined, might not give the
 * intended result. It is the caller's responsibility to check isEmptyOrNegative() if needed.
 */
class Rect {
public:
    AI Rect() = default;
    AI Rect(float l, float t, float r, float b) : fVals(NegateBotRight({l,t,r,b})) {}
    AI Rect(float2 topLeft, float2 botRight) : fVals(topLeft, -botRight) {}
    AI Rect(const SkRect& r) : fVals(NegateBotRight(float4::Load(r.asScalars()))) {}

    AI static Rect XYWH(float x, float y, float w, float h) {
        return Rect(x, y, x + w, y + h);
    }
    AI static Rect XYWH(float2 topLeft, float2 size) {
        return Rect(topLeft, topLeft + size);
    }
    AI static Rect WH(float w, float h) {
        return Rect(0, 0, w, h);
    }
    AI static Rect WH(float2 size) {
        return Rect(float2(0), size);
    }
    AI static Rect Point(float2 p) {
        return Rect(p, p);
    }
    AI static Rect FromVals(float4 vals) {  // vals.zw must already be negated.
        return Rect(vals);
    }

    // Constructs a Rect with ltrb = [-inf, -inf, inf, inf], useful for accumulating intersections
    AI static Rect Infinite() {
        return FromVals(float4{SK_FloatNegativeInfinity});
    }
    // Constructs a negative Rect with ltrb = [inf, inf, -inf, -inf], useful for accumulating unions
    AI static Rect InfiniteInverted() {
        return FromVals(float4{SK_FloatInfinity});
    }

    AI bool operator==(Rect rect) const { return all(fVals == rect.fVals); }
    AI bool operator!=(Rect rect) const { return any(fVals != rect.fVals); }

    AI const float4& vals() const { return fVals; }  // [left, top, -right, -bot].
    AI float4& vals() { return fVals; }  // [left, top, -right, -bot].

    AI float x() const { return fVals.x(); }
    AI float y() const { return fVals.y(); }
    AI float left() const { return fVals.x(); }
    AI float top() const { return fVals.y(); }
    AI float right() const { return -fVals.z(); }
    AI float bot() const { return -fVals.w(); }
    AI float2 topLeft() const { return fVals.xy(); }
    AI float2 botRight() const { return -fVals.zw(); }
    AI float4 ltrb() const { return NegateBotRight(fVals); }

    AI void setLeft(float left) { fVals.x() = left; }
    AI void setTop(float top) { fVals.y() = top; }
    AI void setRight(float right) { fVals.z() = -right; }
    AI void setBot(float bot) { fVals.w() = -bot; }
    AI void setTopLeft(float2 topLeft) { fVals.xy() = topLeft; }
    AI void setBotRight(float2 botRight) { fVals.zw() = -botRight; }

    AI SkRect asSkRect() const {
        SkRect r;
        this->ltrb().store(&r);
        return r;
    }

    AI bool isEmptyNegativeOrNaN() const {
        return !all(fVals.xy() + fVals.zw() < 0);  // !([l-r, r-b] < 0) == ([w, h] <= 0)
                                                   // Use "!(-size < 0)" in order to detect NaN.
    }

    AI float2 size() const { return -(fVals.xy() + fVals.zw()); }  // == [-(l-r), -(t-b)] == [w, h]

    AI float2 center() const {
        float4 p = fVals * float4(.5f, .5f, -.5f, -.5f);  // == [l, t, r, b] * .5
        return p.xy() + p.zw();  // == [(l + r)/2, (t + b)/2]
    }

    AI float area() const {
        float2 negativeSize = fVals.xy() + fVals.zw();  // == [l-r, t-b] == [-w, -h]
        return negativeSize.x() * negativeSize.y();
    }

    // A rect stored in a complementary form of: [right, bottom, -left, -top]. Store a local
    // ComplementRect object if intersects() will be called many times.
    struct ComplementRect {
        AI ComplementRect(Rect rect) : fVals(-rect.fVals.zwxy()) {}
        float4 fVals;  // [right, bottom, -left, -top]
    };

    AI bool intersects(ComplementRect comp) const { return all(fVals < comp.fVals); }
    AI bool contains(Rect rect) const { return all(fVals <= rect.fVals); }

    // Some operations may return a negative or empty rect. Operations on a rect that either is
    // negative or empty, while well-defined, might not give the intended result. It is the caller's
    // responsibility to check isEmptyOrNegative() if needed.
    AI Rect makeRoundIn() const { return ceil(fVals); }
    AI Rect makeRoundOut() const { return floor(fVals); }
    AI Rect makeInset(float inset) const { return fVals + inset; }
    AI Rect makeInset(float2 inset) const { return fVals + inset.xyxy(); }
    AI Rect makeOutset(float outset) const { return fVals - outset; }
    AI Rect makeOutset(float2 outset) const { return fVals - outset.xyxy(); }
    AI Rect makeOffset(float2 offset) const { return fVals + float4(offset, -offset); }
    AI Rect makeJoin(Rect rect) const { return min(fVals, rect.fVals); }
    AI Rect makeIntersect(Rect rect) const { return max(fVals, rect.fVals); }
    AI Rect makeSorted() const { return min(fVals, -fVals.zwxy()); }

    AI Rect& roundIn() { return *this = this->makeRoundIn(); }
    AI Rect& roundOut() { return *this = this->makeRoundOut(); }
    AI Rect& inset(float inset) { return *this = this->makeInset(inset); }
    AI Rect& inset(float2 inset) { return *this = this->makeInset(inset); }
    AI Rect& outset(float outset) { return *this = this->makeOutset(outset); }
    AI Rect& outset(float2 outset) { return *this = this->makeOutset(outset); }
    AI Rect& offset(float2 offset) { return *this = this->makeOffset(offset); }
    AI Rect& join(Rect rect) { return *this = this->makeJoin(rect); }
    AI Rect& intersect(Rect rect) { return *this = this->makeIntersect(rect); }
    AI Rect& sort() { return *this = this->makeSorted(); }

private:
    AI static float4 NegateBotRight(float4 vals) {  // Returns [vals.xy, -vals.zw].
        return skvx::bit_pun<float4>(skvx::bit_pun<uint4>(vals) ^ uint4(0, 0, 1u << 31, 1u << 31));
    }

    AI Rect(float4 vals) : fVals(vals) {}  // vals.zw must already be negated.

    float4 fVals;  // [left, top, -right, -bottom]
};

#undef AI

} // namespace skgpu

#endif // skgpu_geom_Rect_DEFINED
