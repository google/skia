/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfFunctionCommonDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

int64_t SkPdfFunctionCommonDictionary::FunctionType(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FunctionType", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfFunctionCommonDictionary::has_FunctionType() const {
  return get("FunctionType", "") != NULL;
}

SkPdfArray* SkPdfFunctionCommonDictionary::Domain(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Domain", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfFunctionCommonDictionary::has_Domain() const {
  return get("Domain", "") != NULL;
}

SkPdfArray* SkPdfFunctionCommonDictionary::Range(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Range", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFunctionCommonDictionary::has_Range() const {
  return get("Range", "") != NULL;
}
