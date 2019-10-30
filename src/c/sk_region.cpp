/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2016 Bluebeam Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRegion.h"

#include "include/c/sk_region.h"

#include "src/c/sk_types_priv.h"


sk_region_t *sk_region_new() {
    return ToRegion(new SkRegion());
}

sk_region_t *sk_region_new2(const sk_region_t *region) {
    return ToRegion(new SkRegion(*AsRegion(region)));
}

void sk_region_delete(sk_region_t *region) {
    delete AsRegion(region);
}

bool sk_region_contains(sk_region_t *r, const sk_region_t *region) {
    return AsRegion(r)->contains(*AsRegion(region));
}

bool sk_region_contains2(sk_region_t *r, int x, int y) {
    return AsRegion(r)->contains(x, y);
}

bool sk_region_intersects_rect(sk_region_t* r, const sk_irect_t* rect) {
    return AsRegion(r)->intersects(*AsIRect(rect));
}

bool sk_region_intersects(sk_region_t *r, const sk_region_t *src) {
    return AsRegion(r)->intersects(*AsRegion(src));
}

bool sk_region_set_path(sk_region_t *dst, const sk_path_t *t, const sk_region_t* clip) {
    return AsRegion(dst)->setPath(*AsPath(t), *AsRegion(clip));
}

bool sk_region_set_rect(sk_region_t *dst, const sk_irect_t *rect) {
    return AsRegion(dst)->setRect(*AsIRect(rect));
}

bool sk_region_set_region(sk_region_t* dst, const sk_region_t* region) {
    return AsRegion(dst)->setRegion(*AsRegion(region));
}

bool sk_region_op(sk_region_t *dst, int left, int top, int right, int bottom, sk_region_op_t op) {
    return AsRegion(dst)->op(SkIRect::MakeLTRB(left, top, right, bottom), (SkRegion::Op)op);
}

bool sk_region_op2(sk_region_t *dst, sk_region_t *src, sk_region_op_t op) {
    return AsRegion(dst)->op(*AsRegion(src), (SkRegion::Op)op);
}

void sk_region_get_bounds(sk_region_t* r, sk_irect_t* rect) {
    *rect = ToIRect(AsRegion(r)->getBounds());
}
