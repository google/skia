/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"

#include "xamarin\sk_x_data.h"

#include "..\sk_types_priv.h"
#include "sk_x_types_priv.h"

sk_data_t* sk_data_new_from_file(const char* path) {
    return ToData(SkData::NewFromFileName(path));
}

sk_data_t* sk_data_new_from_stream(sk_stream_t* stream, size_t length) {
    return ToData(SkData::NewFromStream(AsStream(stream), length));
}

const uint8_t* sk_data_get_bytes(const sk_data_t* cdata) {
    return AsData(cdata)->bytes();
}
