/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfFileSpecificationDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfFileSpecificationDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfFileSpecificationDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkString SkPdfFileSpecificationDictionary::FS(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfFileSpecificationDictionary::has_FS() const {
  return get("FS", "") != NULL;
}

SkString SkPdfFileSpecificationDictionary::F(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("F", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfFileSpecificationDictionary::has_F() const {
  return get("F", "") != NULL;
}

SkString SkPdfFileSpecificationDictionary::DOS(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("DOS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfFileSpecificationDictionary::has_DOS() const {
  return get("DOS", "") != NULL;
}

SkString SkPdfFileSpecificationDictionary::Mac(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Mac", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfFileSpecificationDictionary::has_Mac() const {
  return get("Mac", "") != NULL;
}

SkString SkPdfFileSpecificationDictionary::Unix(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Unix", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfFileSpecificationDictionary::has_Unix() const {
  return get("Unix", "") != NULL;
}

SkPdfArray* SkPdfFileSpecificationDictionary::ID(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ID", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFileSpecificationDictionary::has_ID() const {
  return get("ID", "") != NULL;
}

bool SkPdfFileSpecificationDictionary::V(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("V", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfFileSpecificationDictionary::has_V() const {
  return get("V", "") != NULL;
}

SkPdfDictionary* SkPdfFileSpecificationDictionary::EF(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("EF", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFileSpecificationDictionary::has_EF() const {
  return get("EF", "") != NULL;
}

SkPdfDictionary* SkPdfFileSpecificationDictionary::RF(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("RF", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFileSpecificationDictionary::has_RF() const {
  return get("RF", "") != NULL;
}
