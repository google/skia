/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "SkBitmap.h"
#include "SkImageInfo.h"
#include "SkShader.h"
#include "SkRecord.h"
#include "SkRecordAnalysis.h"
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

#define APPEND(record, type, ...) SkNEW_PLACEMENT_ARGS(record.append<type>(), type, (__VA_ARGS__))

// Basic tests for the low-level SkRecord code.
DEF_TEST(Record, r) {
    SkRecord record;

    // Add a simple DrawRect command.
    SkRect rect = SkRect::MakeWH(10, 10);
    SkPaint paint;
    APPEND(record, SkRecords::DrawRect, paint, rect);

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

DEF_TEST(RecordAnalysis, r) {
    SkRecord record;

    SkRect rect = SkRect::MakeWH(10, 10);
    SkPaint paint;
    APPEND(record, SkRecords::DrawRect, paint, rect);
    REPORTER_ASSERT(r, !SkRecordWillPlaybackBitmaps(record));

    SkBitmap bitmap;
    APPEND(record, SkRecords::DrawBitmap, &paint, bitmap, 0.0f, 0.0f);
    REPORTER_ASSERT(r, SkRecordWillPlaybackBitmaps(record));

    SkNEW_PLACEMENT_ARGS(record.replace<SkRecords::DrawRect>(1),
                         SkRecords::DrawRect, (paint, rect));
    REPORTER_ASSERT(r, !SkRecordWillPlaybackBitmaps(record));

    SkPaint paint2;
    // CreateBitmapShader is too smart for us; an empty (or 1x1) bitmap shader
    // gets optimized into a non-bitmap form, so we create a 2x2 bitmap here.
    SkBitmap bitmap2;
    bitmap2.allocPixels(SkImageInfo::MakeN32Premul(2, 2));
    bitmap2.eraseColor(SK_ColorBLUE);
    *(bitmap2.getAddr32(0, 0)) = SK_ColorGREEN;
    SkShader* shader = SkShader::CreateBitmapShader(bitmap2, SkShader::kClamp_TileMode,
                                                    SkShader::kClamp_TileMode);
    paint2.setShader(shader);
    REPORTER_ASSERT(r, shader->asABitmap(NULL, NULL, NULL) == SkShader::kDefault_BitmapType);

    APPEND(record, SkRecords::DrawRect, paint2, rect);
    REPORTER_ASSERT(r, SkRecordWillPlaybackBitmaps(record));
}

#undef APPEND

