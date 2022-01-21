/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Sampler_DEFINED
#define skgpu_Sampler_DEFINED

#include "experimental/graphite/src/Resource.h"

namespace skgpu {

class Sampler : public Resource {
public:
    ~Sampler() override;

protected:
    Sampler(const Gpu*);

private:
};

} // namepsace skgpu

#endif // skgpu_Sampler_DEFINED
