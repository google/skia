/*
 * Copyright 2016 Bluebeam Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRegion.h"
#include "sk_region.h"
#include "sk_types_priv.h"


sk_region_t *sk_region_new() {
  return ToRegion(new SkRegion());
}

sk_region_t *sk_region_new2(const sk_region_t *region) {
  return ToRegion(new SkRegion(*AsRegion(region)));
}

void sk_region_delete(sk_region_t *cpath) { delete AsRegion(cpath); }

void sk_region_contains(sk_region_t *r, const sk_region_t *region) {
  AsRegion(r)->contains(*AsRegion(region));
}

void sk_region_contains2(sk_region_t *r, int x, int y) {
  AsRegion(r)->contains(x, y);
}

bool sk_region_intersects(sk_region_t *r, const sk_region_t *src) {
  return AsRegion(r)->intersects(*AsRegion(src));
}

bool sk_region_set_path(sk_region_t *dst, const sk_path_t *t, const sk_region_t* clip) {
  SkRegion region = *AsRegion(dst);
  return region.setPath(AsPath(*t), *AsRegion(clip));
}

bool sk_region_set_rect(sk_region_t *dst, const sk_irect_t *rect) {
  SkRegion region = *AsRegion(dst);
  return region.setRect(AsIRect(*rect));
}

bool sk_region_op(sk_region_t *dst, int left, int top, int right, int bottom, sk_region_op_t op) {
  SkRegion region = *AsRegion(dst);
  return region.op(left, top, right, bottom, (SkRegion::Op)op);
}

bool sk_region_op2(sk_region_t *dst, sk_region_t *src, sk_region_op_t op) {
  SkRegion region = *AsRegion(dst);
  return region.op(*AsRegion(src), (SkRegion::Op)op);
}

sk_irect_t sk_region_get_bounds(sk_region_t *r) {
  SkRegion region = *AsRegion(r);
  return ToIRect(AsRegion(r)->getBounds());
}