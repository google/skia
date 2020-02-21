/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDDLTmpRedirect_DEFINED
#define SkDDLTmpRedirect_DEFINED

// A client is directly referencing include/private/SkDeferredDisplayList.h. This temporary
// header is required to move them over to referencing include/core/SkDeferredDisplayList.h.
#include "include/private/SkDeferredDisplayList.h"

#endif
