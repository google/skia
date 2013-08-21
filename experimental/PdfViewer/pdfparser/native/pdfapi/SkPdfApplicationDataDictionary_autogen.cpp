/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfApplicationDataDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfDate SkPdfApplicationDataDictionary::LastModified(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("LastModified", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDate()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->dateValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkPdfDate();
}

bool SkPdfApplicationDataDictionary::has_LastModified() const {
  return get("LastModified", "") != NULL;
}

SkPdfNativeObject* SkPdfApplicationDataDictionary::Private(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Private", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && true) || (doc == NULL && ret != NULL && ret->isReference())) return ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfApplicationDataDictionary::has_Private() const {
  return get("Private", "") != NULL;
}
