/*
 * Copyright 2017 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkExecutor.h"

#include "src/core/SkExecutors.h"

SkExecutor::~SkExecutor() {}

void SkExecutor::SetDefault(SkExecutor* executor) {
    SkExecutors::SetDefault(executor);
}
