/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/geom/Rect.h"
#include "tests/Test.h"

namespace skgpu::graphite {

#define CHECK(A) REPORTER_ASSERT(reporter, A)

DEF_GRAPHITE_TEST(skgpu_Rect, reporter, CtsEnforcement::kNextRelease) {
    using float2 = skvx::float2;
    using float4 = skvx::float4;

    const SkRect skRect = SkRect::MakeLTRB(1,-3,4,0);
    const Rect rect = skRect;
    CHECK(rect == rect);
    CHECK(rect == skRect); // promotes 'skRect' to a Rect for ==
    CHECK(rect.asSkRect() == skRect); // converts 'rect' to SkRect for ==

    for (const float l : {0,1,2}) {
    for (const float t : {-4,-3,-2}) {
    for (const float r : {3,4,5}) {
    for (const float b : {-1,0,1}) {
        const Rect rect2(l,t,r,b);
        const SkRect skRect2{l,t,r,b};

        CHECK(rect2 == rect2);
        CHECK(rect2 == Rect(float2(l,t), float2(r,b)));
        CHECK(rect2 == Rect(skRect2));
        CHECK(rect2.asSkRect() == skRect2);

        CHECK((rect2 == rect) == (rect == rect2));
        CHECK((rect2 != rect) == (rect != rect2));
        CHECK((rect != rect2) == !(rect == rect2));

        CHECK(rect2 == Rect::XYWH(l, t, r - l, b - t));
        CHECK(rect2 == Rect::XYWH(float2(l, t), float2(r - l, b - t)));
        if (l == 0 && t == 0) {
            CHECK(rect2 == Rect::WH(r - l, b - t));
            CHECK(rect2 == Rect::WH(float2(r - l, b - t)));
        }
        CHECK(rect2 == Rect::FromVals(rect2.vals()));

        CHECK(rect2.x() == l);
        CHECK(rect2.y() == t);
        CHECK(rect2.left() == l);
        CHECK(rect2.top() == t);
        CHECK(rect2.right() == r);
        CHECK(rect2.bot() == b);
        CHECK(all(rect2.topLeft() == float2(l,t)));
        CHECK(all(rect2.botRight() == float2(r,b)));
        CHECK(all(rect2.ltrb() == float4(l,t,r,b)));
        CHECK(all(rect2.vals() == float4(l,t,-r,-b)));

        Rect setTest(-99,-99,99,99);
        CHECK(setTest != rect2);
        setTest.setLeft(l);
        setTest.setTop(t);
        setTest.setRight(r);
        setTest.setBot(b);
        CHECK(setTest == rect2);

        setTest = Rect(-99,-99,99,99);
        CHECK(setTest != rect2);
        setTest.setTopLeft({l,t});
        setTest.setBotRight({r,b});
        CHECK(setTest == rect2);

        for (int i = 0; i < 4; ++i) {
            Rect rnan = rect2;
            CHECK(!rnan.isEmptyNegativeOrNaN());
            rnan.vals()[i] = std::numeric_limits<float>::quiet_NaN();
            CHECK(rnan.isEmptyNegativeOrNaN());
        }

        CHECK(all(rect2.size() == float2(skRect2.width(), skRect2.height())));
        CHECK(all(rect2.center() == float2(skRect2.centerX(), skRect2.centerY())));
        CHECK(rect2.area() == skRect2.height() * skRect2.width());

        CHECK(rect.intersects(rect2) == rect2.intersects(rect));
        CHECK(rect.intersects(rect2) == skRect.intersects(skRect2));
        CHECK(rect.contains(rect2) == skRect.contains(skRect2));
        CHECK(rect2.contains(rect) == skRect2.contains(skRect));

        CHECK(rect2.makeRoundIn() == SkRect::Make(skRect2.roundIn()));
        CHECK(rect2.makeRoundOut() == SkRect::Make(skRect2.roundOut()));
        CHECK(rect2.makeInset(.5f) == skRect2.makeInset(.5f, .5f));
        CHECK(rect2.makeInset({.5f, -.25f}) == skRect2.makeInset(.5f, -.25f));
        CHECK(rect2.makeOutset(.5f) == skRect2.makeOutset(.5f, .5f));
        CHECK(rect2.makeOutset({.5f, -.25f}) == skRect2.makeOutset(.5f, -.25f));
        CHECK(rect2.makeOffset({.5f, -.25f}) == skRect2.makeOffset(.5f, -.25f));

        SkRect skJoin = skRect;
        skJoin.join(skRect2);
        CHECK(rect.makeJoin(rect2) == skJoin);
        CHECK(rect.makeJoin(rect2) == rect2.makeJoin(rect));

        CHECK(rect.intersects(rect2) == !rect.makeIntersect(rect2).isEmptyNegativeOrNaN());
        CHECK(rect.makeIntersect(rect2) == rect2.makeIntersect(rect));
        if (rect.intersects(rect2)) {
            CHECK(skRect.intersects(skRect2));
            SkRect skIsect;
            CHECK(skIsect.intersect(skRect, skRect2));
            CHECK(rect.makeIntersect(rect2) == Rect(skIsect));
        }

        const Rect rect3{r,b,l,t}; // intentionally out of order
        const SkRect skRect3{r,b,l,t};
        CHECK(rect3.isEmptyNegativeOrNaN());
        CHECK(skRect3.isEmpty());
        CHECK(rect3.makeSorted() == skRect3.makeSorted());
        CHECK(rect3.makeSorted() == rect2);
    }}}}
}

}  // namespace skgpu
