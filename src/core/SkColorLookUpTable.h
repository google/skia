/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorLookUpTable_DEFINED
#define SkColorLookUpTable_DEFINED

#include "SkNx.h"
#include "SkRefCnt.h"
#include "SkTemplates.h"

// TODO: scope inside SkColorLookUpTable
static constexpr uint8_t kMaxColorChannels = 4;

class SkColorLookUpTable : public SkRefCnt {
public:
    static constexpr uint8_t kOutputChannels = 3;

    SkColorLookUpTable(uint8_t inputChannels, const uint8_t limits[]);

    int  inputChannels() const { return  fInputChannels; }
    int outputChannels() const { return kOutputChannels; }

    // TODO: Rename to somethingBetter(int)?
    int gridPoints(int dimension) const {
        SkASSERT(dimension >= 0 && dimension < inputChannels());
        return fLimits[dimension];
    }

    // Objects of this type are created in a custom fashion using sk_malloc_throw
    // and therefore must be sk_freed.
    void* operator new(size_t size) = delete;
    void* operator new(size_t, void* p) { return p; }
    void operator delete(void* p) { sk_free(p); }

    const float* table() const {
        return SkTAddOffset<const float>(this, sizeof(SkColorLookUpTable));
    }

private:
    uint8_t fInputChannels;
    uint8_t fLimits[kMaxColorChannels];
};

#endif
