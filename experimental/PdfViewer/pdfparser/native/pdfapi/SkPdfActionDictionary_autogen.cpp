/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfActionDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfActionDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfActionDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkString SkPdfActionDictionary::S(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("S", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfActionDictionary::has_S() const {
  return get("S", "") != NULL;
}

bool SkPdfActionDictionary::isNextADictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Next", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isDictionary();
}

SkPdfDictionary* SkPdfActionDictionary::getNextAsDictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Next", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfActionDictionary::isNextAArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Next", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isArray();
}

SkPdfArray* SkPdfActionDictionary::getNextAsArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Next", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfActionDictionary::has_Next() const {
  return get("Next", "") != NULL;
}
