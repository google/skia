/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfEncryptionCommonDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfEncryptionCommonDictionary::Filter(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Filter", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfEncryptionCommonDictionary::has_Filter() const {
  return get("Filter", "") != NULL;
}

double SkPdfEncryptionCommonDictionary::V(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("V", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfEncryptionCommonDictionary::has_V() const {
  return get("V", "") != NULL;
}

int64_t SkPdfEncryptionCommonDictionary::Length(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Length", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfEncryptionCommonDictionary::has_Length() const {
  return get("Length", "") != NULL;
}
