/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */

#include "SkPixelXorXfermode.h"
#include "SkColorPriv.h"

// we always return an opaque color, 'cause I don't know what to do with
// the alpha-component and still return a valid premultiplied color.
SkPMColor SkPixelXorXfermode::xferColor(SkPMColor src, SkPMColor dst) {
    SkPMColor res = src ^ dst ^ fOpColor;
    res |= (SK_A32_MASK << SK_A32_SHIFT);   // force it to be opaque
    return res;
}

void SkPixelXorXfermode::flatten(SkFlattenableWriteBuffer& wb) {
    this->INHERITED::flatten(wb);
    wb.write32(fOpColor);
}

SkPixelXorXfermode::SkPixelXorXfermode(SkFlattenableReadBuffer& rb)
        : SkXfermode(rb) {
    fOpColor = rb.readU32();
}

SkFlattenable::Factory SkPixelXorXfermode::getFactory() {
    return Create;
}

SkFlattenable* SkPixelXorXfermode::Create(SkFlattenableReadBuffer& rb) {
    return SkNEW_ARGS(SkPixelXorXfermode, (rb));
}

static SkFlattenable::Registrar
    gSkPixelXorXfermodeReg("SkPixelXorXfermode",
                           SkPixelXorXfermode::CreateProc);
