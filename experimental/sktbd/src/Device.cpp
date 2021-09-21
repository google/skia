/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/sktbd/src/Device.h"

#include "include/core/SkImageInfo.h"

namespace sktbd {

Device::Device(const SkImageInfo& ii) : SkBaseDevice(ii, SkSurfaceProps()) {}

} // namespace sktbd
