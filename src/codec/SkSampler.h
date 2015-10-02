/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkSampler_DEFINED
#define SkSampler_DEFINED

#include "SkTypes.h"

class SkSampler : public SkNoncopyable {
public:
    /**
     *  Update the sampler to sample every sampleX'th pixel. Returns the
     *  width after sampling.
     */
    int setSampleX(int sampleX) {
        return this->onSetSampleX(sampleX);
    }

    virtual ~SkSampler() {}
private:
    virtual int onSetSampleX(int) = 0;
};

#endif // SkSampler_DEFINED
