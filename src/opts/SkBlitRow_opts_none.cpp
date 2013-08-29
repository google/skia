/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlitRow.h"

// Platform impl of Platform_procs with no overrides

SkBlitRow::Proc SkBlitRow::PlatformProcs565(unsigned flags) {
    return NULL;
}

SkBlitRow::Proc32 SkBlitRow::PlatformProcs32(unsigned flags) {
    return NULL;
}

SkBlitRow::ColorProc SkBlitRow::PlatformColorProc() {
    return NULL;
}

SkBlitRow::ColorRectProc PlatformColorRectProcFactory() {
    return NULL;
}
