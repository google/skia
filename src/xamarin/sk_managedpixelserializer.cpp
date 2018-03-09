/*
 * Copyright 2017 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkManagedPixelSerializer.h"

#include "sk_managedpixelserializer.h"
#include "sk_types_priv.h"


static sk_managedpixelserializer_use_delegate    gUse;
static sk_managedpixelserializer_encode_delegate gEncode;


static inline sk_managedpixelserializer_t* ToManagedPixelSerializer(SkManagedPixelSerializer* cserializer) {
    return reinterpret_cast<sk_managedpixelserializer_t*>(cserializer);
}


bool dUse(SkManagedPixelSerializer* cserializer, const void* data, size_t len)
{
    return gUse(ToManagedPixelSerializer(cserializer), data, len);
}

SkData* dEncode(SkManagedPixelSerializer* cserializer, const SkPixmap& pixmap)
{
    return AsData(gEncode(ToManagedPixelSerializer(cserializer), ToPixmap(&pixmap)));
}


sk_managedpixelserializer_t* sk_managedpixelserializer_new ()
{
    return ToManagedPixelSerializer (new SkManagedPixelSerializer ());
}

void sk_managedpixelserializer_set_delegates (
    const sk_managedpixelserializer_use_delegate pUse, 
    const sk_managedpixelserializer_encode_delegate pEncode)
{
    gUse = pUse;
    gEncode = pEncode;

    SkManagedPixelSerializer::setDelegates(dUse, dEncode);
}

