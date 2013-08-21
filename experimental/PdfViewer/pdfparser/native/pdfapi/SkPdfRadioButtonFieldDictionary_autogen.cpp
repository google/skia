/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfRadioButtonFieldDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfArray* SkPdfRadioButtonFieldDictionary::Opt(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Opt", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfRadioButtonFieldDictionary::has_Opt() const {
  return get("Opt", "") != NULL;
}
