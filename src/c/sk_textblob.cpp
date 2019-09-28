/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTextBlob.h"

#include "include/c/sk_textblob.h"

#include "src/c/sk_types_priv.h"


void sk_textblob_ref(const sk_textblob_t* blob) {
    SkSafeRef(AsTextBlob(blob));
}

void sk_textblob_unref(const sk_textblob_t* blob) {
    SkSafeUnref(AsTextBlob(blob));
}

uint32_t sk_textblob_get_unique_id(const sk_textblob_t* blob) {
    return AsTextBlob(blob)->uniqueID();
}

void sk_textblob_get_bounds(const sk_textblob_t* blob, sk_rect_t* bounds) {
    *bounds = ToRect(AsTextBlob(blob)->bounds());
}


sk_textblob_builder_t* sk_textblob_builder_new() {
    return ToTextBlobBuilder(new SkTextBlobBuilder());
}

void sk_textblob_builder_delete(sk_textblob_builder_t* builder) {
    delete AsTextBlobBuilder(builder);
}

sk_textblob_t* sk_textblob_builder_make(sk_textblob_builder_t* builder) {
    return ToTextBlob(AsTextBlobBuilder(builder)->make().release());
}

void sk_textblob_builder_alloc_run_text(sk_textblob_builder_t* builder, const sk_paint_t* font, int count, float x, float y, int textByteCount, const sk_string_t* lang, const sk_rect_t* bounds, sk_textblob_builder_runbuffer_t* runbuffer) {
    *runbuffer = ToTextBlobBuilderRunBuffer(AsTextBlobBuilder(builder)->allocRunText(AsPaint(*font), count, x, y, textByteCount, *AsString(lang), AsRect(bounds)));
}

void sk_textblob_builder_alloc_run_text_pos_h(sk_textblob_builder_t* builder, const sk_paint_t* font, int count, float y, int textByteCount, const sk_string_t* lang, const sk_rect_t* bounds, sk_textblob_builder_runbuffer_t* runbuffer) {
    *runbuffer = ToTextBlobBuilderRunBuffer(AsTextBlobBuilder(builder)->allocRunTextPosH(AsPaint(*font), count, y, textByteCount, *AsString(lang), AsRect(bounds)));
}

void sk_textblob_builder_alloc_run_text_pos(sk_textblob_builder_t* builder, const sk_paint_t* font, int count, int textByteCount, const sk_string_t* lang, const sk_rect_t* bounds, sk_textblob_builder_runbuffer_t* runbuffer) {
    *runbuffer = ToTextBlobBuilderRunBuffer(AsTextBlobBuilder(builder)->allocRunTextPos(AsPaint(*font), count, textByteCount, *AsString(lang), AsRect(bounds)));
}
