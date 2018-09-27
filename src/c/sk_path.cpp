/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkMaskFilter.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPictureRecorder.h"
#include "SkSurface.h"

#include "sk_canvas.h"
#include "sk_data.h"
#include "sk_image.h"
#include "sk_paint.h"
#include "sk_path.h"
#include "sk_surface.h"
#include "sk_types_priv.h"

const struct {
    sk_path_direction_t fC;
    SkPath::Direction   fSk;
} gPathDirMap[] = {
    { CW_SK_PATH_DIRECTION,  SkPath::kCW_Direction },
    { CCW_SK_PATH_DIRECTION, SkPath::kCCW_Direction },
};

static bool from_c_path_direction(sk_path_direction_t cdir, SkPath::Direction* dir) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gPathDirMap); ++i) {
        if (gPathDirMap[i].fC == cdir) {
            if (dir) {
                *dir = gPathDirMap[i].fSk;
            }
            return true;
        }
    }
    return false;
}

typedef SkPath SkPathBuilder;

static SkPathBuilder* as_pathbuilder(sk_pathbuilder_t* cbuilder) {
    return reinterpret_cast<SkPathBuilder*>(cbuilder);
}

sk_pathbuilder_t* sk_pathbuilder_new() {
    return reinterpret_cast<sk_pathbuilder_t*>(new SkPathBuilder);
}

void sk_pathbuilder_delete(sk_pathbuilder_t* cbuilder) {
    delete as_pathbuilder(cbuilder);
}

void sk_pathbuilder_move_to(sk_pathbuilder_t* cbuilder, float x, float y) {
    as_pathbuilder(cbuilder)->moveTo(x, y);
}

void sk_pathbuilder_line_to(sk_pathbuilder_t* cbuilder, float x, float y) {
    as_pathbuilder(cbuilder)->lineTo(x, y);
}

void sk_pathbuilder_quad_to(sk_pathbuilder_t* cbuilder, float x0, float y0, float x1, float y1) {
    as_pathbuilder(cbuilder)->quadTo(x0, y0, x1, y1);
}

void sk_pathbuilder_conic_to(sk_pathbuilder_t* cbuilder, float x0, float y0, float x1, float y1, float w) {
    as_pathbuilder(cbuilder)->conicTo(x0, y0, x1, y1, w);
}

void sk_pathbuilder_cubic_to(sk_pathbuilder_t* cbuilder, float x0, float y0, float x1, float y1, float x2, float y2) {
    as_pathbuilder(cbuilder)->cubicTo(x0, y0, x1, y1, x2, y2);
}

void sk_pathbuilder_close(sk_pathbuilder_t* cbuilder) {
    as_pathbuilder(cbuilder)->close();
}

void sk_pathbuilder_add_rect(sk_pathbuilder_t* cbuilder, const sk_rect_t* crect, sk_path_direction_t cdir) {
    SkPath::Direction dir;
    if (!from_c_path_direction(cdir, &dir)) {
        return;
    }
    as_pathbuilder(cbuilder)->addRect(AsRect(*crect), dir);
}

void sk_pathbuilder_add_oval(sk_pathbuilder_t* cbuilder, const sk_rect_t* crect, sk_path_direction_t cdir) {
    SkPath::Direction dir;
    if (!from_c_path_direction(cdir, &dir)) {
        return;
    }
    as_pathbuilder(cbuilder)->addOval(AsRect(*crect), dir);
}

sk_path_t* sk_pathbuilder_detach_path(sk_pathbuilder_t* cbuilder) {
    SkPathBuilder* builder = as_pathbuilder(cbuilder);
    SkPath* path = new SkPath(*builder);
    builder->reset();
    return reinterpret_cast<sk_path_t*>(path);
}

///////////////////////////////////////////////////////////////////////////////////////////

void sk_path_delete(sk_path_t* cpath) {
    delete reinterpret_cast<SkPath*>(cpath);
}

bool sk_path_get_bounds(const sk_path_t* cpath, sk_rect_t* crect) {
    const SkPath& path = AsPath(*cpath);

    if (path.isEmpty()) {
        if (crect) {
            *crect = ToRect(SkRect::MakeEmpty());
        }
        return false;
    }

    if (crect) {
        *crect = ToRect(path.getBounds());
    }
    return true;
}
