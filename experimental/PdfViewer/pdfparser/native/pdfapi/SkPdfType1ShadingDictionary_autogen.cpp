/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfType1ShadingDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfArray* SkPdfType1ShadingDictionary::Domain(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Domain", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfType1ShadingDictionary::has_Domain() const {
  return get("Domain", "") != NULL;
}

SkPdfArray* SkPdfType1ShadingDictionary::Matrix(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Matrix", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfType1ShadingDictionary::has_Matrix() const {
  return get("Matrix", "") != NULL;
}

SkPdfFunction SkPdfType1ShadingDictionary::Function(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Function", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isFunction()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->functionValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkPdfFunction();
}

bool SkPdfType1ShadingDictionary::has_Function() const {
  return get("Function", "") != NULL;
}
