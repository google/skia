/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "SkRecord.h"
#include "SkRecords.h"

// Sums the area of any DrawRect command it sees.
class AreaSummer {
public:
    AreaSummer() : fArea(0) {}

    template <typename T> void operator()(const T&) { }

    void operator()(const SkRecords::DrawRect& draw) {
        fArea += (int)(draw.rect.width() * draw.rect.height());
    }

    int area() const { return fArea; }

    void apply(const SkRecord& record) {
        for (unsigned i = 0; i < record.count(); i++) {
            record.visit<void>(i, *this);
        }
    }

private:
    int fArea;
};

// Scales out the bottom-right corner of any DrawRect command it sees by 2x.
struct Stretch {
    template <typename T> void operator()(T*) {}
    void operator()(SkRecords::DrawRect* draw) {
        draw->rect.fRight *= 2;
        draw->rect.fBottom *= 2;
    }

    void apply(SkRecord* record) {
        for (unsigned i = 0; i < record->count(); i++) {
            record->mutate<void>(i, *this);
        }
    }
};

// Basic tests for the low-level SkRecord code.
DEF_TEST(Record, r) {
    SkRecord record;

    // Add a simple DrawRect command.
    SkRect rect = SkRect::MakeWH(10, 10);
    SkPaint paint;
    SkNEW_PLACEMENT_ARGS(record.append<SkRecords::DrawRect>(), SkRecords::DrawRect, (paint, rect));

    // Its area should be 100.
    AreaSummer summer;
    summer.apply(record);
    REPORTER_ASSERT(r, summer.area() == 100);

    // Scale 2x.
    Stretch stretch;
    stretch.apply(&record);

    // Now its area should be 100 + 400.
    summer.apply(record);
    REPORTER_ASSERT(r, summer.area() == 500);
}
