/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfURLAliasDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfURLAliasDictionary::U(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("U", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfURLAliasDictionary::has_U() const {
  return get("U", "") != NULL;
}

SkPdfArray* SkPdfURLAliasDictionary::C(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("C", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfURLAliasDictionary::has_C() const {
  return get("C", "") != NULL;
}
