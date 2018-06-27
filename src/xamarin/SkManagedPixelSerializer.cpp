/*
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkManagedPixelSerializer.h"

static use_delegate fUse = nullptr;
static encode_delegate fEncode = nullptr;

SkManagedPixelSerializer::SkManagedPixelSerializer() {
}

void SkManagedPixelSerializer::setDelegates(const use_delegate pUse, const encode_delegate pEncode)
{
    ::fUse = pUse;
    ::fEncode = pEncode;
}

bool SkManagedPixelSerializer::onUseEncodedData(const void* data, size_t len) {
    return ::fUse(this, data, len);
}

SkData* SkManagedPixelSerializer::onEncode(const SkPixmap& pixmap) {
    return ::fEncode(this, pixmap);
}
