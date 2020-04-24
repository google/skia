/*
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/xamarin/SkCompatPaint.h"

#include "include/xamarin/sk_compatpaint.h"
#include "src/c/sk_types_priv.h"


static inline const SkCompatPaint* AsCompatPaint(const sk_compatpaint_t* c) {
    return reinterpret_cast<const SkCompatPaint*>(c);
}
static inline SkCompatPaint* AsCompatPaint(sk_compatpaint_t* c) {
    return reinterpret_cast<SkCompatPaint*>(c);
}
static inline sk_compatpaint_t* ToCompatPaint(SkCompatPaint* c) {
    return reinterpret_cast<sk_compatpaint_t*>(c);
}


sk_compatpaint_t* sk_compatpaint_new(void) {
    return ToCompatPaint(new SkCompatPaint());
}

sk_compatpaint_t* sk_compatpaint_new_with_font(const sk_font_t* font) {
    return ToCompatPaint(new SkCompatPaint(AsFont(font)));
}

void sk_compatpaint_delete(sk_compatpaint_t* paint) {
    delete AsCompatPaint(paint);
}

sk_compatpaint_t* sk_compatpaint_clone(const sk_compatpaint_t* paint) {
    return ToCompatPaint(new SkCompatPaint(*AsCompatPaint(paint)));
}

void sk_compatpaint_reset(sk_compatpaint_t* paint) {
    AsCompatPaint(paint)->reset();
}

sk_font_t* sk_compatpaint_make_font(sk_compatpaint_t* paint) {
    return ToFont(AsCompatPaint(paint)->makeFont());
}

sk_font_t* sk_compatpaint_get_font(sk_compatpaint_t* paint) {
    return ToFont(AsCompatPaint(paint)->getFont());
}

void sk_compatpaint_set_text_align(sk_compatpaint_t* paint, sk_text_align_t align) {
    AsCompatPaint(paint)->setTextAlign((SkTextUtils::Align)align);
}

sk_text_align_t sk_compatpaint_get_text_align(const sk_compatpaint_t* paint) {
    return (sk_text_align_t)AsCompatPaint(paint)->getTextAlign();
}

void sk_compatpaint_set_text_encoding(sk_compatpaint_t* paint, sk_text_encoding_t encoding) {
    AsCompatPaint(paint)->setTextEncoding((SkTextEncoding)encoding);
}

sk_text_encoding_t sk_compatpaint_get_text_encoding(const sk_compatpaint_t* paint) {
    return (sk_text_encoding_t)AsCompatPaint(paint)->getTextEncoding();
}
