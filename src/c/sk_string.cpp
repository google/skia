/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkString.h"

#include "include/c/sk_string.h"

#include "src/c/sk_types_priv.h"

sk_string_t* sk_string_new_empty() {
    return ToString(new SkString());
}

sk_string_t* sk_string_new_with_copy(const char* src, size_t length) {
    return ToString(new SkString(src, length));
}

void sk_string_destructor(const sk_string_t* cstring) {
    delete AsString(cstring);
}

size_t sk_string_get_size(const sk_string_t* cstring) {
    return AsString(cstring)->size();
}

const char* sk_string_get_c_str(const sk_string_t* cstring) {
    return AsString(cstring)->c_str();
}
