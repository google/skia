/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Sampler_DEFINED
#define skgpu_graphite_Sampler_DEFINED

#include "src/gpu/graphite/Resource.h"

namespace skgpu::graphite {

class Sampler : public Resource {
public:
    ~Sampler() override;

    const char* getResourceType() const override { return "Sampler"; }

protected:
    Sampler(const SharedContext*);

private:
};

} // namepsace skgpu::graphite

#endif // skgpu_graphite_Sampler_DEFINED
