/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfDctdecodeFilterDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

int64_t SkPdfDctdecodeFilterDictionary::ColorTransform(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ColorTransform", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfDctdecodeFilterDictionary::has_ColorTransform() const {
  return get("ColorTransform", "") != NULL;
}
