/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Sampler.h"

namespace skgpu {

Sampler::Sampler(const Gpu* gpu) : Resource(gpu) {}

Sampler::~Sampler() {}

} // namespace skgpu
