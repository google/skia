/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfFDFFieldDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfArray* SkPdfFDFFieldDictionary::Kids(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Kids", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFDFFieldDictionary::has_Kids() const {
  return get("Kids", "") != NULL;
}

SkString SkPdfFDFFieldDictionary::T(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("T", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfFDFFieldDictionary::has_T() const {
  return get("T", "") != NULL;
}

SkPdfNativeObject* SkPdfFDFFieldDictionary::V(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("V", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && true) || (doc == NULL && ret != NULL && ret->isReference())) return ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFDFFieldDictionary::has_V() const {
  return get("V", "") != NULL;
}

int64_t SkPdfFDFFieldDictionary::Ff(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Ff", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfFDFFieldDictionary::has_Ff() const {
  return get("Ff", "") != NULL;
}

int64_t SkPdfFDFFieldDictionary::SetFf(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("SetFf", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfFDFFieldDictionary::has_SetFf() const {
  return get("SetFf", "") != NULL;
}

int64_t SkPdfFDFFieldDictionary::ClrFf(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ClrFf", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfFDFFieldDictionary::has_ClrFf() const {
  return get("ClrFf", "") != NULL;
}

int64_t SkPdfFDFFieldDictionary::F(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("F", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfFDFFieldDictionary::has_F() const {
  return get("F", "") != NULL;
}

int64_t SkPdfFDFFieldDictionary::SetF(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("SetF", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfFDFFieldDictionary::has_SetF() const {
  return get("SetF", "") != NULL;
}

int64_t SkPdfFDFFieldDictionary::ClrF(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ClrF", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfFDFFieldDictionary::has_ClrF() const {
  return get("ClrF", "") != NULL;
}

SkPdfDictionary* SkPdfFDFFieldDictionary::AP(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("AP", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFDFFieldDictionary::has_AP() const {
  return get("AP", "") != NULL;
}

SkPdfDictionary* SkPdfFDFFieldDictionary::APRef(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("APRef", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFDFFieldDictionary::has_APRef() const {
  return get("APRef", "") != NULL;
}

SkPdfDictionary* SkPdfFDFFieldDictionary::IF(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("IF", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFDFFieldDictionary::has_IF() const {
  return get("IF", "") != NULL;
}

SkPdfArray* SkPdfFDFFieldDictionary::Opt(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Opt", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFDFFieldDictionary::has_Opt() const {
  return get("Opt", "") != NULL;
}

SkPdfDictionary* SkPdfFDFFieldDictionary::A(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("A", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFDFFieldDictionary::has_A() const {
  return get("A", "") != NULL;
}

SkPdfDictionary* SkPdfFDFFieldDictionary::AA(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("AA", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFDFFieldDictionary::has_AA() const {
  return get("AA", "") != NULL;
}
