/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorLookUpTable_DEFINED
#define SkColorLookUpTable_DEFINED

#include "SkRefCnt.h"
#include "SkTemplates.h"

class SkColorLookUpTable : public SkRefCnt {
public:
    static constexpr uint8_t kOutputChannels = 3;

    SkColorLookUpTable(uint8_t inputChannels, const uint8_t gridPoints[3]) {
        SkASSERT(3 == inputChannels);
        memcpy(fGridPoints, gridPoints, 3 * sizeof(uint8_t));   
    }

    void interp3D(float dst[3], float src[3]) const;

private:
    const float* table() const {
        return SkTAddOffset<const float>(this, sizeof(SkColorLookUpTable));
    }

    uint8_t fGridPoints[3];

public:
    // Objects of this type are created in a custom fashion using sk_malloc_throw
    // and therefore must be sk_freed.
    void* operator new(size_t size) = delete;
    void* operator new(size_t, void* p) { return p; }
    void operator delete(void* p) { sk_free(p); }
};

#endif
