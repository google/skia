/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkTLS.h"

static void* gSpecific = nullptr;

void* SkTLS::PlatformGetSpecific(bool) {
    return gSpecific;
}

void SkTLS::PlatformSetSpecific(void* ptr) {
    gSpecific = ptr;
}
