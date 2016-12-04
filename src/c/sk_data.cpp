/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"

#include "sk_data.h"

#include "sk_types_priv.h"

sk_data_t* sk_data_new_from_file(const char* path) {
    return ToData(SkData::NewFromFileName(path));
}

sk_data_t* sk_data_new_from_stream(sk_stream_t* stream, size_t length) {
    return ToData(SkData::NewFromStream(AsStream(stream), length));
}

const uint8_t* sk_data_get_bytes(const sk_data_t* cdata) {
    return AsData(cdata)->bytes();
}

sk_data_t* sk_data_new_empty() {
    return ToData(SkData::NewEmpty());
}

sk_data_t* sk_data_new_with_copy(const void* src, size_t length) {
    return ToData(SkData::MakeWithCopy(src, length).release());
}

sk_data_t* sk_data_new_from_malloc(const void* memory, size_t length) {
    return ToData(SkData::MakeFromMalloc(memory, length).release());
}

sk_data_t* sk_data_new_subset(const sk_data_t* csrc, size_t offset, size_t length) {
    return ToData(SkData::MakeSubset(AsData(csrc), offset, length).release());
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
