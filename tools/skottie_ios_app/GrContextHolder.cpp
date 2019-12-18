// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "tools/skottie_ios_app/GrContextHolder.h"

#include "include/gpu/GrContext.h"

void GrContextRelease::operator()(GrContext* ptr) { SkSafeUnref(ptr); }
