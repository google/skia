/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsTestCommon.h"
#include "SkPathOpsCubic.h"
#include "SkPathOpsLine.h"
#include "SkPathOpsQuad.h"
#include "SkPathOpsRect.h"
#include "Test.h"

static const SkDLine lineTests[] = {
    {{{2, 1}, {2, 1}}},
    {{{2, 1}, {1, 1}}},
    {{{2, 1}, {2, 2}}},
    {{{1, 1}, {2, 2}}},
    {{{3, 0}, {2, 1}}},
    {{{3, 2}, {1, 1}}},
};

static const SkDQuad quadTests[] = {
    {{{1, 1}, {2, 1}, {0, 2}}},
    {{{0, 0}, {1, 1}, {3, 1}}},
    {{{2, 0}, {1, 1}, {2, 2}}},
    {{{4, 0}, {0, 1}, {4, 2}}},
    {{{0, 0}, {0, 1}, {1, 1}}},
};

static const SkDCubic cubicTests[] = {
    {{{2, 0}, {3, 1}, {2, 2}, {1, 1}}},
    {{{3, 1}, {2, 2}, {1, 1}, {2, 0}}},
    {{{3, 0}, {2, 1}, {3, 2}, {1, 1}}},
};

static const size_t lineTests_count = SK_ARRAY_COUNT(lineTests);
static const size_t quadTests_count = SK_ARRAY_COUNT(quadTests);
static const size_t cubicTests_count = SK_ARRAY_COUNT(cubicTests);

DEF_TEST(PathOpsDRect, reporter) {
    size_t index;
    SkDRect rect, rect2;
    for (index = 0; index < lineTests_count; ++index) {
        const SkDLine& line = lineTests[index];
        SkASSERT(ValidLine(line));
        rect.setBounds(line);
        REPORTER_ASSERT(reporter, rect.fLeft == SkTMin(line[0].fX, line[1].fX));
        REPORTER_ASSERT(reporter, rect.fTop == SkTMin(line[0].fY, line[1].fY));
        REPORTER_ASSERT(reporter, rect.fRight == SkTMax(line[0].fX, line[1].fX));
        REPORTER_ASSERT(reporter, rect.fBottom == SkTMax(line[0].fY, line[1].fY));
        rect2.set(line[0]);
        rect2.add(line[1]);
        REPORTER_ASSERT(reporter, rect2.fLeft == SkTMin(line[0].fX, line[1].fX));
        REPORTER_ASSERT(reporter, rect2.fTop == SkTMin(line[0].fY, line[1].fY));
        REPORTER_ASSERT(reporter, rect2.fRight == SkTMax(line[0].fX, line[1].fX));
        REPORTER_ASSERT(reporter, rect2.fBottom == SkTMax(line[0].fY, line[1].fY));
        REPORTER_ASSERT(reporter, rect.contains(line[0]));
        REPORTER_ASSERT(reporter, rect.intersects(&rect2));
    }
    for (index = 0; index < quadTests_count; ++index) {
        const SkDQuad& quad = quadTests[index];
        SkASSERT(ValidQuad(quad));
        rect.setRawBounds(quad);
        REPORTER_ASSERT(reporter, rect.fLeft == SkTMin(quad[0].fX,
                SkTMin(quad[1].fX, quad[2].fX)));
        REPORTER_ASSERT(reporter, rect.fTop == SkTMin(quad[0].fY,
                SkTMin(quad[1].fY, quad[2].fY)));
        REPORTER_ASSERT(reporter, rect.fRight == SkTMax(quad[0].fX,
                SkTMax(quad[1].fX, quad[2].fX)));
        REPORTER_ASSERT(reporter, rect.fBottom == SkTMax(quad[0].fY,
                SkTMax(quad[1].fY, quad[2].fY)));
        rect2.setBounds(quad);
        REPORTER_ASSERT(reporter, rect.intersects(&rect2));
        // FIXME: add a recursive box subdivision method to verify that tight bounds is correct
        SkDPoint leftTop = {rect2.fLeft, rect2.fTop};
        REPORTER_ASSERT(reporter, rect.contains(leftTop));
        SkDPoint rightBottom = {rect2.fRight, rect2.fBottom};
        REPORTER_ASSERT(reporter, rect.contains(rightBottom));
    }
    for (index = 0; index < cubicTests_count; ++index) {
        const SkDCubic& cubic = cubicTests[index];
        SkASSERT(ValidCubic(cubic));
        rect.setRawBounds(cubic);
        REPORTER_ASSERT(reporter, rect.fLeft == SkTMin(cubic[0].fX,
                SkTMin(cubic[1].fX, SkTMin(cubic[2].fX, cubic[3].fX))));
        REPORTER_ASSERT(reporter, rect.fTop == SkTMin(cubic[0].fY,
                SkTMin(cubic[1].fY, SkTMin(cubic[2].fY, cubic[3].fY))));
        REPORTER_ASSERT(reporter, rect.fRight == SkTMax(cubic[0].fX,
                SkTMax(cubic[1].fX, SkTMax(cubic[2].fX, cubic[3].fX))));
        REPORTER_ASSERT(reporter, rect.fBottom == SkTMax(cubic[0].fY,
                SkTMax(cubic[1].fY, SkTMax(cubic[2].fY, cubic[3].fY))));
        rect2.setBounds(cubic);
        REPORTER_ASSERT(reporter, rect.intersects(&rect2));
        // FIXME: add a recursive box subdivision method to verify that tight bounds is correct
        SkDPoint leftTop = {rect2.fLeft, rect2.fTop};
        REPORTER_ASSERT(reporter, rect.contains(leftTop));
        SkDPoint rightBottom = {rect2.fRight, rect2.fBottom};
        REPORTER_ASSERT(reporter, rect.contains(rightBottom));
    }
}
