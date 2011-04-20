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

#include "SkPaintFlagsDrawFilter.h"
#include "SkPaint.h"

SkPaintFlagsDrawFilter::SkPaintFlagsDrawFilter(uint32_t clearFlags,
                                               uint32_t setFlags) {
    fClearFlags = SkToU16(clearFlags & SkPaint::kAllFlags);
    fSetFlags = SkToU16(setFlags & SkPaint::kAllFlags);
}

void SkPaintFlagsDrawFilter::filter(SkPaint* paint, Type) {
    paint->setFlags((paint->getFlags() & ~fClearFlags) | fSetFlags);
}

