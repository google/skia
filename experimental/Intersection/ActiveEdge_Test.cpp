#include "CurveIntersection.h"
#include "Intersections.h"
#include "LineIntersection.h"
#include "SkPath.h"
#include "SkRect.h"
#include "SkTArray.h"
#include "SkTDArray.h"
#include "ShapeOps.h"
#include "TSearch.h"

namespace UnitTest {

#include "EdgeWalker.cpp"

} // end of UnitTest namespace

#include "Intersection_Tests.h"

SkPoint leftRight[][4] = {
// equal length
    {{10, 10}, {10, 50},   {20, 10}, {20, 50}},
    {{10, 10}, {10, 50},   {10, 10}, {20, 50}},
    {{10, 10}, {10, 50},   {20, 10}, {10, 50}},
// left top higher
    {{10,  0}, {10, 50},   {20, 10}, {20, 50}},
    {{10,  0}, {10, 50},   {10, 10}, {20, 50}},
    {{10,  0}, {10, 50},   {20, 10}, {10, 50}},
    {{10,  0}, {10, 50},   {20, 10}, {10 + 0.000001, 40}},
// left top lower
    {{10, 20}, {10, 50},   {20, 10}, {20, 50}},
    {{10, 20}, {10, 50},   {10, 10}, {20, 50}},
    {{10, 20}, {10, 50},   {20, 10}, {10, 50}},
    {{10, 20}, {10, 50},   {20, 10}, {10 + 0.000001, 40}},
    {{10, 20}, {10, 50},   { 0,  0}, {50, 50}},
// left bottom higher
    {{10, 10}, {10, 40},   {20, 10}, {20, 50}},
    {{10, 10}, {10, 40},   {10, 10}, {20, 50}},
    {{10, 10}, {10, 40},   {20, 10}, {10, 50}},
    {{10, 10}, {10, 40},   {20, 10}, { 0 + 0.000001, 70}},
// left bottom lower
    {{10, 10}, {10, 60},   {20, 10}, {20, 50}},
    {{10, 10}, {10, 60},   {10, 10}, {20, 50}},
    {{10, 10}, {10, 60},   {20, 10}, {10 + 0.000001, 50}},
    {{10, 10}, {10, 60},   {20, 10}, {10 + 0.000001, 40}},
    {{10, 10}, {10, 60},   { 0,  0}, {20 + 0.000001, 20}},
};

size_t leftRightCount = sizeof(leftRight) / sizeof(leftRight[0]);

void ActiveEdge_Test() {
    UnitTest::InEdge leftIn, rightIn;
    UnitTest::ActiveEdge left, right;
    left.fWorkEdge.fEdge = &leftIn;
    right.fWorkEdge.fEdge = &rightIn;
    for (size_t x = 0; x < leftRightCount; ++x) {
        left.fAbove = leftRight[x][0];
        left.fBelow = leftRight[x][1]; 
        right.fAbove = leftRight[x][2];
        right.fBelow = leftRight[x][3];
        SkASSERT(left < right);
        SkASSERT(left.operator_less_than(right));
        SkASSERT(!(right < left));
        SkASSERT(!right.operator_less_than(left));
    }
}




