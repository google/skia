/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfURIDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfURIDictionary::Base(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Base", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfURIDictionary::has_Base() const {
  return get("Base", "") != NULL;
}
