/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkDocument.h"
SkDocument* SkDocument::CreatePDF(SkWStream*, SkScalar) { return  nullptr; }
SkDocument* SkDocument::CreatePDF(const char path[], SkScalar) { return nullptr; }
