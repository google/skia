/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfSourceInformationDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

bool SkPdfSourceInformationDictionary::isAUAString(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("AU", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isAnyString();
}

SkString SkPdfSourceInformationDictionary::getAUAsString(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("AU", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfSourceInformationDictionary::isAUADictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("AU", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isDictionary();
}

SkPdfDictionary* SkPdfSourceInformationDictionary::getAUAsDictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("AU", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfSourceInformationDictionary::has_AU() const {
  return get("AU", "") != NULL;
}

SkPdfDate SkPdfSourceInformationDictionary::TS(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDate()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->dateValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfDate();
}

bool SkPdfSourceInformationDictionary::has_TS() const {
  return get("TS", "") != NULL;
}

SkPdfDate SkPdfSourceInformationDictionary::E(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("E", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDate()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->dateValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfDate();
}

bool SkPdfSourceInformationDictionary::has_E() const {
  return get("E", "") != NULL;
}

int64_t SkPdfSourceInformationDictionary::S(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("S", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfSourceInformationDictionary::has_S() const {
  return get("S", "") != NULL;
}

SkPdfDictionary* SkPdfSourceInformationDictionary::C(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("C", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfSourceInformationDictionary::has_C() const {
  return get("C", "") != NULL;
}
