/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfOpiVersionDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfDictionary* SkPdfOpiVersionDictionary::version_number(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("version_number", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfOpiVersionDictionary::has_version_number() const {
  return get("version_number", "") != NULL;
}
