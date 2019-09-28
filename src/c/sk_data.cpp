/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"

#include "include/c/sk_data.h"

#include "src/c/sk_types_priv.h"

sk_data_t* sk_data_new_from_file(const char* path) {
    return ToData(SkData::MakeFromFileName(path).release());
}

sk_data_t* sk_data_new_from_stream(sk_stream_t* stream, size_t length) {
    return ToData(SkData::MakeFromStream(AsStream(stream), length).release());
}

const uint8_t* sk_data_get_bytes(const sk_data_t* cdata) {
    return AsData(cdata)->bytes();
}

sk_data_t* sk_data_new_empty() {
    return ToData(SkData::MakeEmpty().release());
}

sk_data_t* sk_data_new_with_copy(const void* src, size_t length) {
    return ToData(SkData::MakeWithCopy(src, length).release());
}

sk_data_t* sk_data_new_subset(const sk_data_t* csrc, size_t offset, size_t length) {
    return ToData(SkData::MakeSubset(AsData(csrc), offset, length).release());
}

sk_data_t* sk_data_new_with_proc(const void* ptr, size_t length, sk_data_release_proc proc, void* ctx) {
    return ToData(SkData::MakeWithProc(ptr, length, proc, ctx).release());
}

sk_data_t* sk_data_new_uninitialized(size_t size) {
    return ToData(SkData::MakeUninitialized(size).release());
}

void sk_data_ref(const sk_data_t* cdata) {
    SkSafeRef(AsData(cdata));
}

void sk_data_unref(const sk_data_t* cdata) {
    SkSafeUnref(AsData(cdata));
}

size_t sk_data_get_size(const sk_data_t* cdata) {
    return AsData(cdata)->size();
}

const void* sk_data_get_data(const sk_data_t* cdata) {
    return AsData(cdata)->data();
}
