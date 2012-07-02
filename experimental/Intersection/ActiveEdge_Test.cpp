#include "Simplify.h"

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
    {{10,  0}, {10, 50},   {20, 10}, {10 + 0.000001f, 40}},
// left top lower
    {{10, 20}, {10, 50},   {20, 10}, {20, 50}},
    {{10, 20}, {10, 50},   {10, 10}, {20, 50}},
    {{10, 20}, {10, 50},   {20, 10}, {10, 50}},
    {{10, 20}, {10, 50},   {20, 10}, {10 + 0.000001f, 40}},
    {{10, 20}, {10, 50},   { 0,  0}, {50, 50}},
// left bottom higher
    {{10, 10}, {10, 40},   {20, 10}, {20, 50}},
    {{10, 10}, {10, 40},   {10, 10}, {20, 50}},
    {{10, 10}, {10, 40},   {20, 10}, {10, 50}},
    {{10, 10}, {10, 40},   {20, 10}, { 0 + 0.000001f, 70}},
// left bottom lower
    {{10, 10}, {10, 60},   {20, 10}, {20, 50}},
    {{10, 10}, {10, 60},   {10, 10}, {20, 50}},
    {{10, 10}, {10, 60},   {20, 10}, {10 + 0.000001f, 50}},
    {{10, 10}, {10, 60},   {20, 10}, {10 + 0.000001f, 40}},
    {{10, 10}, {10, 60},   { 0,  0}, {20 + 0.000001f, 20}},
};

size_t leftRightCount = sizeof(leftRight) / sizeof(leftRight[0]);

// older code that worked mostly
static bool operator_less_than(const UnitTest::ActiveEdge& lh,
        const UnitTest::ActiveEdge& rh) {
    if (rh.fAbove.fY - lh.fAbove.fY > lh.fBelow.fY - rh.fAbove.fY
            && lh.fBelow.fY < rh.fBelow.fY
            || lh.fAbove.fY - rh.fAbove.fY < rh.fBelow.fY - lh.fAbove.fY
            && rh.fBelow.fY < lh.fBelow.fY) {
        const SkPoint& check = rh.fBelow.fY <= lh.fBelow.fY
                && lh.fBelow != rh.fBelow ? rh.fBelow :
                rh.fAbove;
        return (check.fY - lh.fAbove.fY) * (lh.fBelow.fX - lh.fAbove.fX)
                < (lh.fBelow.fY - lh.fAbove.fY) * (check.fX - lh.fAbove.fX);
    }
    const SkPoint& check = lh.fBelow.fY <= rh.fBelow.fY 
            && lh.fBelow != rh.fBelow ? lh.fBelow : lh.fAbove;
    return (rh.fBelow.fY - rh.fAbove.fY) * (check.fX - rh.fAbove.fX)
            < (check.fY - rh.fAbove.fY) * (rh.fBelow.fX - rh.fAbove.fX);
}


void ActiveEdge_Test() {
    UnitTest::InEdge leftIn, rightIn;
    UnitTest::ActiveEdge left, right;
    left.fWorkEdge.fEdge = &leftIn;
    right.fWorkEdge.fEdge = &rightIn;
    for (size_t x = 0; x < leftRightCount; ++x) {
        left.fAbove = leftRight[x][0];
        left.fTangent = left.fBelow = leftRight[x][1]; 
        right.fAbove = leftRight[x][2];
        right.fTangent = right.fBelow = leftRight[x][3];
        SkASSERT(left < right);
        SkASSERT(operator_less_than(left, right));
        SkASSERT(!(right < left));
        SkASSERT(!operator_less_than(right, left));
    }
}




