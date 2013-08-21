/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfFieldDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfFieldDictionary::FT(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FT", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfFieldDictionary::has_FT() const {
  return get("FT", "") != NULL;
}

SkPdfDictionary* SkPdfFieldDictionary::Parent(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Parent", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFieldDictionary::has_Parent() const {
  return get("Parent", "") != NULL;
}

SkPdfArray* SkPdfFieldDictionary::Kids(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Kids", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFieldDictionary::has_Kids() const {
  return get("Kids", "") != NULL;
}

SkString SkPdfFieldDictionary::T(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("T", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfFieldDictionary::has_T() const {
  return get("T", "") != NULL;
}

SkString SkPdfFieldDictionary::TU(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TU", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfFieldDictionary::has_TU() const {
  return get("TU", "") != NULL;
}

SkString SkPdfFieldDictionary::TM(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TM", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfFieldDictionary::has_TM() const {
  return get("TM", "") != NULL;
}

int64_t SkPdfFieldDictionary::Ff(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Ff", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfFieldDictionary::has_Ff() const {
  return get("Ff", "") != NULL;
}

SkPdfNativeObject* SkPdfFieldDictionary::V(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("V", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && true) || (doc == NULL && ret != NULL && ret->isReference())) return ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFieldDictionary::has_V() const {
  return get("V", "") != NULL;
}

SkPdfNativeObject* SkPdfFieldDictionary::DV(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("DV", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && true) || (doc == NULL && ret != NULL && ret->isReference())) return ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFieldDictionary::has_DV() const {
  return get("DV", "") != NULL;
}

SkPdfDictionary* SkPdfFieldDictionary::AA(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("AA", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFieldDictionary::has_AA() const {
  return get("AA", "") != NULL;
}
