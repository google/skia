/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPath.h"

#include "xamarin/sk_x_path.h"

#include "../sk_types_priv.h"
#include "sk_x_types_priv.h"

void sk_path_rmove_to(sk_path_t* cpath, float dx, float dy) {
    as_path(cpath)->rMoveTo(dx, dy);
}

void sk_path_rline_to(sk_path_t* cpath, float dx, float dy) {
    as_path(cpath)->rLineTo(dx, dy);
}

void sk_path_rquad_to(sk_path_t* cpath, float dx0, float dy0, float dx1, float dy1) {
    as_path(cpath)->rQuadTo(dx0, dy0, dx1, dy1);
}

void sk_path_rconic_to(sk_path_t* cpath, float dx0, float dy0, float dx1, float dy1, float w) {
    as_path(cpath)->rConicTo(dx0, dy0, dx1, dy1, w);
}

void sk_path_rcubic_to(sk_path_t* cpath, float dx0, float dy0, float dx1, float dy1, float dx2, float dy2) {
    as_path(cpath)->rCubicTo(dx0, dy0, dx1, dy1, dx2, dy2);
}

void sk_path_add_rect_start(sk_path_t* cpath, const sk_rect_t* crect, sk_path_direction_t cdir, uint32_t startIndex) {
    SkPath::Direction dir;
    if (!find_sk(cdir, &dir)) {
        return;
    }
    as_path(cpath)->addRect(AsRect(*crect), dir, startIndex);
}

void sk_path_add_arc(sk_path_t* cpath, const sk_rect_t* crect, float startAngle, float sweepAngle) {
    as_path(cpath)->addArc(AsRect(*crect), startAngle, sweepAngle);
}

void sk_path_set_filltype(sk_path_t* cpath, sk_path_filltype_t cfilltype) {
    SkPath::FillType filltype;
    if (!find_sk(cfilltype, &filltype)) {
        return;
    }
    as_path(cpath)->setFillType(filltype);
}

sk_path_filltype_t sk_path_get_filltype(sk_path_t *cpath) {
    sk_path_filltype_t cfilltype; 
    if (!find_c(as_path(cpath)->getFillType(), &cfilltype)) {
        cfilltype = WINDING_SK_PATH_FILLTYPE;
    }
    return cfilltype;
}
