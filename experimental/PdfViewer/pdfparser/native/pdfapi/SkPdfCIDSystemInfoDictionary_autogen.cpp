/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfCIDSystemInfoDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfCIDSystemInfoDictionary::Registry(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Registry", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfCIDSystemInfoDictionary::has_Registry() const {
  return get("Registry", "") != NULL;
}

SkString SkPdfCIDSystemInfoDictionary::Ordering(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Ordering", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfCIDSystemInfoDictionary::has_Ordering() const {
  return get("Ordering", "") != NULL;
}

int64_t SkPdfCIDSystemInfoDictionary::Supplement(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Supplement", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfCIDSystemInfoDictionary::has_Supplement() const {
  return get("Supplement", "") != NULL;
}
