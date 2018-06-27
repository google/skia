/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPixelSerializer.h"

#include "sk_pixelserializer.h"

#include "sk_types_priv.h"

void sk_pixelserializer_unref(sk_pixelserializer_t* cserializer)
{
    SkSafeUnref(AsPixelSerializer(cserializer));
}

bool sk_pixelserializer_use_encoded_data(sk_pixelserializer_t* cserializer, const void* data, size_t len)
{
    return AsPixelSerializer(cserializer)->useEncodedData(data, len);
}

sk_data_t* sk_pixelserializer_encode(sk_pixelserializer_t* cserializer, const sk_pixmap_t* cpixmap)
{
    return ToData(AsPixelSerializer(cserializer)->encode(AsPixmap(*cpixmap)));
}
