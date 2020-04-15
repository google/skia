/*
* Copyright 2020 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkMatrixProvider_DEFINED
#define SkMatrixProvider_DEFINED

#include "include/core/SkTypes.h"

class SkM44;

class SkMatrixProvider {
public:
    virtual ~SkMatrixProvider() {}
    virtual bool getLocalToMarker(uint32_t id, SkM44* localToMarker) const = 0;
};

#endif
