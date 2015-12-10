/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"

#define WIDTH 400
#define HEIGHT 600

namespace {
// Concave test
void test_concave(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    canvas->translate(0, 0);
    path.moveTo(SkIntToScalar(20), SkIntToScalar(20));
    path.lineTo(SkIntToScalar(80), SkIntToScalar(20));
    path.lineTo(SkIntToScalar(30), SkIntToScalar(30));
    path.lineTo(SkIntToScalar(20), SkIntToScalar(80));
    canvas->drawPath(path, paint);
}

// Reverse concave test
void test_reverse_concave(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    canvas->save();
    canvas->translate(100, 0);
    path.moveTo(SkIntToScalar(20), SkIntToScalar(20));
    path.lineTo(SkIntToScalar(20), SkIntToScalar(80));
    path.lineTo(SkIntToScalar(30), SkIntToScalar(30));
    path.lineTo(SkIntToScalar(80), SkIntToScalar(20));
    canvas->drawPath(path, paint);
    canvas->restore();
}

// Bowtie (intersection)
void test_bowtie(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    canvas->save();
    canvas->translate(200, 0);
    path.moveTo(SkIntToScalar(20), SkIntToScalar(20));
    path.lineTo(SkIntToScalar(80), SkIntToScalar(80));
    path.lineTo(SkIntToScalar(80), SkIntToScalar(20));
    path.lineTo(SkIntToScalar(20), SkIntToScalar(80));
    canvas->drawPath(path, paint);
    canvas->restore();
}

// "fake" bowtie (concave, but no intersection)
void test_fake_bowtie(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    canvas->save();
    canvas->translate(300, 0);
    path.moveTo(SkIntToScalar(20), SkIntToScalar(20));
    path.lineTo(SkIntToScalar(50), SkIntToScalar(40));
    path.lineTo(SkIntToScalar(80), SkIntToScalar(20));
    path.lineTo(SkIntToScalar(80), SkIntToScalar(80));
    path.lineTo(SkIntToScalar(50), SkIntToScalar(60));
    path.lineTo(SkIntToScalar(20), SkIntToScalar(80));
    canvas->drawPath(path, paint);
    canvas->restore();
}

// Fish test (intersection/concave)
void test_fish(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    canvas->save();
    canvas->translate(0, 100);
    path.moveTo(SkIntToScalar(20), SkIntToScalar(20));
    path.lineTo(SkIntToScalar(80), SkIntToScalar(80));
    path.lineTo(SkIntToScalar(70), SkIntToScalar(50));
    path.lineTo(SkIntToScalar(80), SkIntToScalar(20));
    path.lineTo(SkIntToScalar(20), SkIntToScalar(80));
    path.lineTo(SkIntToScalar(0), SkIntToScalar(50));
    canvas->drawPath(path, paint);
    canvas->restore();
}

// Collinear edges
void test_collinear_edges(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    canvas->save();
    canvas->translate(100, 100);
    path.moveTo(SkIntToScalar(20), SkIntToScalar(20));
    path.lineTo(SkIntToScalar(50), SkIntToScalar(20));
    path.lineTo(SkIntToScalar(80), SkIntToScalar(20));
    path.lineTo(SkIntToScalar(50), SkIntToScalar(80));
    canvas->drawPath(path, paint);
    canvas->restore();
}

// Square polygon with a square hole.
void test_hole(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    canvas->save();
    canvas->translate(200, 100);
    path.moveTo(SkIntToScalar(20), SkIntToScalar(20));
    path.lineTo(SkIntToScalar(80), SkIntToScalar(20));
    path.lineTo(SkIntToScalar(80), SkIntToScalar(80));
    path.lineTo(SkIntToScalar(20), SkIntToScalar(80));
    path.moveTo(SkIntToScalar(30), SkIntToScalar(30));
    path.lineTo(SkIntToScalar(30), SkIntToScalar(70));
    path.lineTo(SkIntToScalar(70), SkIntToScalar(70));
    path.lineTo(SkIntToScalar(70), SkIntToScalar(30));
    canvas->drawPath(path, paint);
    canvas->restore();
}

// Star test (self-intersecting)
void test_star(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    canvas->save();
    canvas->translate(300, 100);
    path.moveTo(30, 20);
    path.lineTo(50, 80);
    path.lineTo(70, 20);
    path.lineTo(20, 57);
    path.lineTo(80, 57);
    path.close();
    canvas->drawPath(path, paint);
    canvas->restore();
}

// Stairstep with repeated vert (intersection)
void test_stairstep(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    canvas->save();
    canvas->translate(0, 200);
    path.moveTo(SkIntToScalar(50), SkIntToScalar(50));
    path.lineTo(SkIntToScalar(50), SkIntToScalar(20));
    path.lineTo(SkIntToScalar(80), SkIntToScalar(20));
    path.lineTo(SkIntToScalar(50), SkIntToScalar(50));
    path.lineTo(SkIntToScalar(20), SkIntToScalar(50));
    path.lineTo(SkIntToScalar(20), SkIntToScalar(80));
    canvas->drawPath(path, paint);
    canvas->restore();
}

void test_stairstep2(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    canvas->save();
    canvas->translate(100, 200);
    path.moveTo(20, 60);
    path.lineTo(35, 80);
    path.lineTo(50, 60);
    path.lineTo(65, 80);
    path.lineTo(80, 60);
    canvas->drawPath(path, paint);
    canvas->restore();
}

// Overlapping segments
void test_overlapping(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    canvas->save();
    canvas->translate(200, 200);
    path.moveTo(SkIntToScalar(20), SkIntToScalar(80));
    path.lineTo(SkIntToScalar(80), SkIntToScalar(80));
    path.lineTo(SkIntToScalar(80), SkIntToScalar(20));
    path.lineTo(SkIntToScalar(80), SkIntToScalar(30));
    canvas->drawPath(path, paint);
    canvas->restore();
}

// Monotone test 1 (point in the middle)
void test_monotone_1(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    canvas->save();
    canvas->translate(0, 300);
    path.moveTo(SkIntToScalar(20), SkIntToScalar(20));
    path.quadTo(SkIntToScalar(20), SkIntToScalar(50),
                SkIntToScalar(80), SkIntToScalar(50));
    path.quadTo(SkIntToScalar(20), SkIntToScalar(50),
                SkIntToScalar(20), SkIntToScalar(80));
    canvas->drawPath(path, paint);
    canvas->restore();
}

// Monotone test 2 (point at the top)
void test_monotone_2(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    canvas->save();
    canvas->translate(100, 300);
    path.moveTo(SkIntToScalar(20), SkIntToScalar(20));
    path.lineTo(SkIntToScalar(80), SkIntToScalar(30));
    path.quadTo(SkIntToScalar(20), SkIntToScalar(20),
                SkIntToScalar(20), SkIntToScalar(80));
    canvas->drawPath(path, paint);
    canvas->restore();
}

// Monotone test 3 (point at the bottom)
void test_monotone_3(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    canvas->save();
    canvas->translate(200, 300);
    path.moveTo(SkIntToScalar(20), SkIntToScalar(80));
    path.lineTo(SkIntToScalar(80), SkIntToScalar(70));
    path.quadTo(SkIntToScalar(20), SkIntToScalar(80),
                SkIntToScalar(20), SkIntToScalar(20));
    canvas->drawPath(path, paint);
    canvas->restore();
}

// Monotone test 4 (merging of two monotones)
void test_monotone_4(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    canvas->save();
    canvas->translate(300, 300);
    path.moveTo(80, 25);
    path.lineTo(50, 39);
    path.lineTo(20, 25);
    path.lineTo(40, 45);
    path.lineTo(70, 50);
    path.lineTo(80, 80);
    canvas->drawPath(path, paint);
    canvas->restore();
}

// Monotone test 5 (aborted merging of two monotones)
void test_monotone_5(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    canvas->save();
    canvas->translate(0, 400);
    path.moveTo(50, 20);
    path.lineTo(80, 80);
    path.lineTo(50, 50);
    path.lineTo(20, 80);
    canvas->drawPath(path, paint);
    canvas->restore();
}
// Degenerate intersection test
void test_degenerate(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    canvas->save();
    canvas->translate(100, 400);
    path.moveTo(50, 20);
    path.lineTo(70, 30);
    path.lineTo(20, 50);
    path.moveTo(50, 20);
    path.lineTo(80, 80);
    path.lineTo(50, 80);
    canvas->drawPath(path, paint);
    canvas->restore();
}
// Two triangles with a coincident edge.
void test_coincident_edge(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    canvas->save();
    canvas->translate(200, 400);

    path.moveTo(80, 20);
    path.lineTo(80, 80);
    path.lineTo(20, 80);

    path.moveTo(20, 20);
    path.lineTo(80, 80);
    path.lineTo(20, 80);

    canvas->drawPath(path, paint);
    canvas->restore();
}
// Bowtie with a coincident triangle (one triangle vertex coincident with the
// bowtie's intersection).
void test_bowtie_coincident_triangle(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    canvas->save();
    canvas->translate(300, 400);
    path.moveTo(SkIntToScalar(20), SkIntToScalar(20));
    path.lineTo(SkIntToScalar(80), SkIntToScalar(80));
    path.lineTo(SkIntToScalar(80), SkIntToScalar(20));
    path.lineTo(SkIntToScalar(20), SkIntToScalar(80));
    path.moveTo(SkIntToScalar(50), SkIntToScalar(50));
    path.lineTo(SkIntToScalar(80), SkIntToScalar(20));
    path.lineTo(SkIntToScalar(80), SkIntToScalar(80));
    canvas->drawPath(path, paint);
    canvas->restore();
}

// Coincident edges (big ones first, coincident vert on top).
void test_coincident_edges_1(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    canvas->save();
    canvas->translate(0, 500);
    path.moveTo(SkIntToScalar(20), SkIntToScalar(20));
    path.lineTo(SkIntToScalar(80), SkIntToScalar(80));
    path.lineTo(SkIntToScalar(20), SkIntToScalar(80));
    path.moveTo(SkIntToScalar(20), SkIntToScalar(20));
    path.lineTo(SkIntToScalar(50), SkIntToScalar(50));
    path.lineTo(SkIntToScalar(20), SkIntToScalar(50));
    canvas->drawPath(path, paint);
    canvas->restore();
}
// Coincident edges (small ones first, coincident vert on top).
void test_coincident_edges_2(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    canvas->save();
    canvas->translate(100, 500);
    path.moveTo(SkIntToScalar(20), SkIntToScalar(20));
    path.lineTo(SkIntToScalar(50), SkIntToScalar(50));
    path.lineTo(SkIntToScalar(20), SkIntToScalar(50));
    path.moveTo(SkIntToScalar(20), SkIntToScalar(20));
    path.lineTo(SkIntToScalar(80), SkIntToScalar(80));
    path.lineTo(SkIntToScalar(20), SkIntToScalar(80));
    canvas->drawPath(path, paint);
    canvas->restore();
}
// Coincident edges (small ones first, coincident vert on bottom).
void test_coincident_edges_3(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    canvas->save();
    canvas->translate(200, 500);
    path.moveTo(SkIntToScalar(20), SkIntToScalar(80));
    path.lineTo(SkIntToScalar(20), SkIntToScalar(50));
    path.lineTo(SkIntToScalar(50), SkIntToScalar(50));
    path.moveTo(SkIntToScalar(20), SkIntToScalar(80));
    path.lineTo(SkIntToScalar(20), SkIntToScalar(20));
    path.lineTo(SkIntToScalar(80), SkIntToScalar(20));
    canvas->drawPath(path, paint);
    canvas->restore();
}
// Coincident edges (big ones first, coincident vert on bottom).
void test_coincident_edges_4(SkCanvas* canvas, const SkPaint& paint) {
    SkPath path;
    canvas->save();
    canvas->translate(300, 500);
    path.moveTo(SkIntToScalar(20), SkIntToScalar(80));
    path.lineTo(SkIntToScalar(20), SkIntToScalar(20));
    path.lineTo(SkIntToScalar(80), SkIntToScalar(20));
    path.moveTo(SkIntToScalar(20), SkIntToScalar(80));
    path.lineTo(SkIntToScalar(20), SkIntToScalar(50));
    path.lineTo(SkIntToScalar(50), SkIntToScalar(50));
    canvas->drawPath(path, paint);
    canvas->restore();
}

};

class ConcavePathsGM : public skiagm::GM {
public:
    ConcavePathsGM() {}

protected:
    SkString onShortName() override {
        return SkString("concavepaths");
    }

    SkISize onISize() override {
        return SkISize::Make(WIDTH, HEIGHT);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;

        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kFill_Style);

        test_concave(canvas, paint);
        test_reverse_concave(canvas, paint);
        test_bowtie(canvas, paint);
        test_fake_bowtie(canvas, paint);
        test_fish(canvas, paint);
        test_collinear_edges(canvas, paint);
        test_hole(canvas, paint);
        test_star(canvas, paint);
        test_stairstep(canvas, paint);
        test_stairstep2(canvas, paint);
        test_overlapping(canvas, paint);
        test_monotone_1(canvas, paint);
        test_monotone_2(canvas, paint);
        test_monotone_3(canvas, paint);
        test_monotone_4(canvas, paint);
        test_monotone_5(canvas, paint);
        test_degenerate(canvas, paint);
        test_coincident_edge(canvas, paint);
        test_bowtie_coincident_triangle(canvas, paint);
        test_coincident_edges_1(canvas, paint);
        test_coincident_edges_2(canvas, paint);
        test_coincident_edges_3(canvas, paint);
        test_coincident_edges_4(canvas, paint);
    }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM( return new ConcavePathsGM; )
