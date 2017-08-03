/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDeferredDisplayList_DEFINED
#define SkDeferredDisplayList_DEFINED

#include "SkSurfaceCharacterization.h"

class SkDeferredDisplayList {
public:
    SkDeferredDisplayList(const SkSurfaceCharacterization& characterization)
            : fCharacterization(characterization) {
    }

    const SkSurfaceCharacterization& characterization() const {
        return fCharacterization;
    }

private:
    SkSurfaceCharacterization fCharacterization;
};

#endif
