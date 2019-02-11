/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkReflected.h"

SkSTArray<16, const SkReflected::Type*, true> SkReflected::gTypes;

void SkReflected::VisitTypes(std::function<void(const Type*)> visitor, const Type* baseType) {
    for (const Type* type : gTypes) {
        if (type->isDerivedFrom(baseType)) {
            visitor(type);
        }
    }
}
