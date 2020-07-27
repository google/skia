/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "src/core/SkPointPriv.h"
#include "src/core/SkStrokerPriv.h"
#include "tests/Test.h"

#define _CP(x, y) (SkPoint){x, y}

static inline void ppol(const SkPoint& l, const SkVector& uv,
        const SkPoint& p, SkPoint* pp) {
    *pp = l + uv * uv.dot(p - l);
}

static inline SkScalar line_point_dist(const SkPoint& l,
        const SkVector& uv, const SkPoint& p) {
    SkPoint pp;
    ppol(l, uv, p, &pp);
    return SkPoint::Distance(p, pp);
}

DEF_TEST(ArcsJoinPrimitives, reporter) {
    SkPoint t1, t2;

    t1.set(-7,3);
    SkStrokerPriv::closerIntersection(_CP(-10.1,0),_CP(10.1,0),_CP(0,2),_CP(-7,4),t1, &t2);
    REPORTER_ASSERT(reporter, t1==t2);

    t1.set(-7,3);
    SkStrokerPriv::closerIntersection(_CP(-10.1,0),_CP(10.1,0),_CP(0,2),_CP(-10,-1),t1, &t2);
    REPORTER_ASSERT(reporter, t1==t2);
}

DEF_TEST(ArcsJoinGTests, reporter) {
    int r = SkStrokerPriv::lineCircleTest(_CP(-13.58151,-3.85179),_CP(.62005,.78456),_CP(-6.0247,-7.72246),6.25151);
    REPORTER_ASSERT(reporter, r==0);
    r = SkStrokerPriv::lineCircleTest(_CP(-13.58151,-3.85179),_CP(.62005,.78456),_CP(-6.0247,-7.72246),8.91672);
    REPORTER_ASSERT(reporter, r==2);
    r = SkStrokerPriv::lineCircleTest(_CP(-13.58151,-3.85179),_CP(.62005,.78456),_CP(-6.0247,-7.72246),8.3288);
    REPORTER_ASSERT(reporter, r==1);

    r = SkStrokerPriv::circlesTest(_CP(-6.0247,-7.72246),10.41537,_CP(18.71414,-10.73445),10.60439);
    REPORTER_ASSERT(reporter, r==0);
    r = SkStrokerPriv::circlesTest(_CP(-6.0247,-7.72246),10.41537,_CP(18.71414,-10.73445),16.26834);
    REPORTER_ASSERT(reporter, r==2);
    r = SkStrokerPriv::circlesTest(_CP(-6.0247,-7.72246),10.41537,_CP(18.71414,-10.73445),14.506);
    REPORTER_ASSERT(reporter, r==1);
    r = SkStrokerPriv::circlesTest(_CP(-6.0247,-7.72246),38.8,_CP(18.71414,-10.73445),12.6);
    REPORTER_ASSERT(reporter, r==-1);
    r = SkStrokerPriv::circlesTest(_CP(-6.0247,-7.72246),14.6,_CP(18.71414,-10.73445),41.7);
    REPORTER_ASSERT(reporter, r==-2);
}

DEF_TEST(ArcsJoinIntersections, reporter) {
    SkPoint t1, t2;

    SkStrokerPriv::intersectLines(_CP(-13.78069,-4.05394),_CP(.84254,.53863),_CP(-11.36226,-19.46367),_CP(-.86414,.50326), &t1);
    REPORTER_ASSERT(reporter, line_point_dist(_CP(-13.78069,-4.05394),_CP(.84254,.53863), t1) < 1e-2);
    REPORTER_ASSERT(reporter, line_point_dist(_CP(-11.36226,-19.46367),_CP(-.86414,.50326), t1) < 1e-2);

    SkStrokerPriv::intersectLineCircle(_CP(-11.36226,-19.46367),_CP(.77374,.63351),_CP(18.71414,-10.73445),16.95197, &t1, &t2);
    REPORTER_ASSERT(reporter, line_point_dist(_CP(-11.36226,-19.46367),_CP(.77374,.63351), t1) < 1e-2);
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(SkPoint::Distance(_CP(18.71414,-10.73445), t1) - 16.95197, 1e-2));
    REPORTER_ASSERT(reporter, line_point_dist(_CP(-11.36226,-19.46367),_CP(.77374,.63351), t2) < 1e-2);
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(SkPoint::Distance(_CP(18.71414,-10.73445), t2) - 16.95197, 1e-2));
    SkStrokerPriv::intersectCircles(_CP(-6.0247,-7.72246),10.84393,_CP(1.20025,-11.15018),18.03037, &t1, &t2);
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(SkPoint::Distance(_CP(-6.0247,-7.72246), t1) - 10.84393, 1e-2));
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(SkPoint::Distance(_CP(1.20025,-11.15018), t1) - 18.03037, 1e-2));
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(SkPoint::Distance(_CP(-6.0247,-7.72246), t2) - 10.84393, 1e-2));
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(SkPoint::Distance(_CP(1.20025,-11.15018), t2) - 18.03037, 1e-2));
}

DEF_TEST(ArcsJoinAdjustments, reporter) {
    SkPoint c, pp;
    SkScalar r = SkStrokerPriv::adjustLineCircle(_CP(-78.15152,23.5078),_CP(0.11355,.99353),_CP(-56.77556,23.58413),_CP(-.71814,.6959));
    c = _CP(-56.77556,23.58413) + _CP(-.71814,.6959)*r;
    ppol(_CP(-78.15152,23.5078),_CP(0.11355,.99353),c,&pp);
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(SkPoint::Distance(pp, c) - r, 1e-2));

    r = 6.22412;
    SkScalar r2 = 5.17623;
    REPORTER_ASSERT(reporter, SkStrokerPriv::adjustCircles(_CP(-78.15152,23.5078),_CP(0.11355,.99353),&r,_CP(-49.44412,24.58337),_CP(.89443,.44721),&r2,false));
    c = _CP(-78.15152,23.5078) + _CP(0.11355,.99353)*r;
    SkPoint c2 = _CP(-49.44412,24.58337) + _CP(.89443,.44721)*r2;
    SkPoint t = c2 - c;
    t.setLength(r);
    SkPoint t2 = c + t;
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(SkPoint::Distance(c2, t2) - r2, 1e-2));

    r = 10.45227;
    r2 = 5.17623;

    REPORTER_ASSERT(reporter, SkStrokerPriv::adjustCircles(_CP(-59.55902,24.4769),_CP(.86946,.49401),&r,_CP(-49.44412,24.58337),_CP(.38131,.92445),&r2,true));
    c = _CP(-59.55902,24.4769) + _CP(.86946,.49401)*r;
    c2 = _CP(-49.44412,24.58337)+_CP(.38131,.92445)*r2;
    t = c2-c;
    t.setLength(r);
    t2 = c + t;
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(SkPoint::Distance(c2, t2) - r2, 1e-2));

    r = 12.0f;
    r2 = 5.0f;

    // 180 degree case
    REPORTER_ASSERT(reporter, SkStrokerPriv::adjustCircles(_CP(-59,24),_CP(1,0),&r,_CP(-40,24),_CP(-1,0),&r2,true));
    c = _CP(-59,24) + _CP(1,0)*r;
    c2 = _CP(-40,24) + _CP(-1,0)*r2;
    t = c2-c;
    t.setLength(r);
    t2 = c + t;
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(SkPoint::Distance(c2, t2) - r2, 1e-2));
}

DEF_TEST(ArcsJoinPVPCircle, reporter) {
    SkPoint c, t;
    SkScalar r;

    SkStrokerPriv::PVPCircle(_CP(-71.45834,30.3068),_CP(.62873,.77762),_CP(-59.55902,24.4769),&c,&r);
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(SkPoint::Distance(_CP(-71.45834,30.3068), c) - r, 1e-2));
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(SkPoint::Distance(_CP(-59.55902,24.4769), c) - r, 1e-2));
    SkPointPriv::RotateCW(_CP(.62873,.77762),&t);
    REPORTER_ASSERT(reporter, line_point_dist(_CP(-71.45834,30.3068),t,c) < 1e-2);
}
