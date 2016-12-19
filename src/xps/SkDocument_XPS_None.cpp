/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#if !defined(SK_BUILD_FOR_WIN32)

#include "SkDocument.h"
sk_sp<SkDocument> SkDocument::MakeXPS(SkWStream*, SkDocument::XPSParameters) { return nullptr; }

#endif//!defined(SK_BUILD_FOR_WIN32)
