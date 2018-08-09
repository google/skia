/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "dawn/dawn.h"
#include "dawn/dawncpp.h"

void InitNXTMTLSystemDefaultDevice(nxtProcTable*, nxtDevice*);
void* CreateAutoreleasePool();
void DrainAutoreleasePool(void*);
void DestroyAutoreleasePool(void*);
