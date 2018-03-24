//
//  SkManagedPixelSerializer.cpp
//
//  Created by Matthew on 2017/08/18.
//  Copyright © 2017 Xamarin. All rights reserved.
//

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
