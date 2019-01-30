/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrImageContext.h"

GrImageContext::GrImageContext(GrBackendApi backend, uint32_t uniqueID)
            : INHERITED(backend, uniqueID) {
}

GrImageContext::~GrImageContext() {}
