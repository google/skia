/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfType2ShadingDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfArray* SkPdfType2ShadingDictionary::Coords(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Coords", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfType2ShadingDictionary::has_Coords() const {
  return get("Coords", "") != NULL;
}

SkPdfArray* SkPdfType2ShadingDictionary::Domain(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Domain", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfType2ShadingDictionary::has_Domain() const {
  return get("Domain", "") != NULL;
}

SkPdfFunction SkPdfType2ShadingDictionary::Function(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Function", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isFunction()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->functionValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkPdfFunction();
}

bool SkPdfType2ShadingDictionary::has_Function() const {
  return get("Function", "") != NULL;
}

SkPdfArray* SkPdfType2ShadingDictionary::Extend(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Extend", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfType2ShadingDictionary::has_Extend() const {
  return get("Extend", "") != NULL;
}
