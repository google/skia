/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkColorTable.h"

#include "include/c/sk_colortable.h"

#include "src/c/sk_types_priv.h"

void sk_colortable_unref(sk_colortable_t* ctable) {
    SkSafeUnref(AsColorTable(ctable));
}

sk_colortable_t* sk_colortable_new(const sk_pmcolor_t* colors, int count) {
    return ToColorTable(new SkColorTable(colors, count));
}

int sk_colortable_count(const sk_colortable_t* ctable) {
    return AsColorTable(ctable)->count();
}

void sk_colortable_read_colors(const sk_colortable_t* ctable, sk_pmcolor_t** colors) {
    *colors = (SkColor*)AsColorTable(ctable)->readColors();
}
