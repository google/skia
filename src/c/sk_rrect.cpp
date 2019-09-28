/*
 * Copyright 2014 Google Inc.
 * Copyright 2016 Xamarin Inc.
 * Copyright 2018 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRRect.h"

#include "include/c/sk_rrect.h"

#include "src/c/sk_types_priv.h"


sk_rrect_t* sk_rrect_new(void) {
    return ToRRect(new SkRRect());
}

sk_rrect_t* sk_rrect_new_copy(const sk_rrect_t* rrect) {
    return ToRRect(new SkRRect(*AsRRect(rrect)));
}

void sk_rrect_delete(const sk_rrect_t* rrect) {
  delete AsRRect(rrect);
}

sk_rrect_type_t sk_rrect_get_type(const sk_rrect_t* rrect) {
    return (sk_rrect_type_t)AsRRect(rrect)->getType();
}

void sk_rrect_get_rect(const sk_rrect_t* rrect, sk_rect_t* rect) {
    *rect = ToRect(AsRRect(rrect)->rect());
}

void sk_rrect_get_radii(const sk_rrect_t* rrect, sk_rrect_corner_t corner, sk_vector_t* radii) {
    *radii = ToPoint(AsRRect(rrect)->radii((SkRRect::Corner)corner));
}

float sk_rrect_get_width(const sk_rrect_t* rrect) {
    return AsRRect(rrect)->width();
}

float sk_rrect_get_height(const sk_rrect_t* rrect) {
    return AsRRect(rrect)->height();
}

void sk_rrect_set_empty(sk_rrect_t* rrect) {
    AsRRect(rrect)->setEmpty();
}

void sk_rrect_set_rect(sk_rrect_t* rrect, const sk_rect_t* rect) {
    AsRRect(rrect)->setRect(*AsRect(rect));
}

void sk_rrect_set_oval(sk_rrect_t* rrect, const sk_rect_t* rect) {
    AsRRect(rrect)->setOval(*AsRect(rect));
}

void sk_rrect_set_rect_xy(sk_rrect_t* rrect, const sk_rect_t* rect, float xRad, float yRad) {
    AsRRect(rrect)->setRectXY(*AsRect(rect), xRad, yRad);
}

void sk_rrect_set_nine_patch(sk_rrect_t* rrect, const sk_rect_t* rect, float leftRad, float topRad, float rightRad, float bottomRad) {
    AsRRect(rrect)->setNinePatch(*AsRect(rect), leftRad, topRad, rightRad, bottomRad);
}

void sk_rrect_set_rect_radii(sk_rrect_t* rrect, const sk_rect_t* rect, const sk_vector_t* radii) {
    AsRRect(rrect)->setRectRadii(*AsRect(rect), AsPoint(radii));
}

void sk_rrect_inset(sk_rrect_t* rrect, float dx, float dy) {
    AsRRect(rrect)->inset(dx, dy);
}

void sk_rrect_outset(sk_rrect_t* rrect, float dx, float dy) {
    AsRRect(rrect)->outset(dx, dy);
}

void sk_rrect_offset(sk_rrect_t* rrect, float dx, float dy) {
    AsRRect(rrect)->offset(dx, dy);
}

bool sk_rrect_contains(const sk_rrect_t* rrect, const sk_rect_t* rect) {
    return AsRRect(rrect)->contains(*AsRect(rect));
}

bool sk_rrect_is_valid(const sk_rrect_t* rrect) {
    return AsRRect(rrect)->isValid();
}

bool sk_rrect_transform(sk_rrect_t* rrect, const sk_matrix_t* matrix, sk_rrect_t* dest) {
    return AsRRect(rrect)->transform(AsMatrix(matrix), AsRRect(dest));
}
