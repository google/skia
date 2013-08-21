/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfHideActionDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfHideActionDictionary::S(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("S", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfHideActionDictionary::has_S() const {
  return get("S", "") != NULL;
}

bool SkPdfHideActionDictionary::isTADictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("T", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isDictionary();
}

SkPdfDictionary* SkPdfHideActionDictionary::getTAsDictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("T", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfHideActionDictionary::isTAString(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("T", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isAnyString();
}

SkString SkPdfHideActionDictionary::getTAsString(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("T", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfHideActionDictionary::isTAArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("T", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isArray();
}

SkPdfArray* SkPdfHideActionDictionary::getTAsArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("T", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfHideActionDictionary::has_T() const {
  return get("T", "") != NULL;
}

bool SkPdfHideActionDictionary::H(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("H", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfHideActionDictionary::has_H() const {
  return get("H", "") != NULL;
}
