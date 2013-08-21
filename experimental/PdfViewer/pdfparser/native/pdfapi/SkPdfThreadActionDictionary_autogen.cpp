/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfThreadActionDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfThreadActionDictionary::S(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("S", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfThreadActionDictionary::has_S() const {
  return get("S", "") != NULL;
}

SkPdfFileSpec SkPdfThreadActionDictionary::F(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("F", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && false) || (doc == NULL && ret != NULL && ret->isReference())) return ret->fileSpecValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfFileSpec();
}

bool SkPdfThreadActionDictionary::has_F() const {
  return get("F", "") != NULL;
}

bool SkPdfThreadActionDictionary::isDADictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("D", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isDictionary();
}

SkPdfDictionary* SkPdfThreadActionDictionary::getDAsDictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("D", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfThreadActionDictionary::isDAInteger(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("D", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isInteger();
}

int64_t SkPdfThreadActionDictionary::getDAsInteger(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("D", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfThreadActionDictionary::isDAString(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("D", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isAnyString();
}

SkString SkPdfThreadActionDictionary::getDAsString(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("D", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfThreadActionDictionary::has_D() const {
  return get("D", "") != NULL;
}

bool SkPdfThreadActionDictionary::isBADictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("B", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isDictionary();
}

SkPdfDictionary* SkPdfThreadActionDictionary::getBAsDictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("B", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfThreadActionDictionary::isBAInteger(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("B", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isInteger();
}

int64_t SkPdfThreadActionDictionary::getBAsInteger(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("B", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfThreadActionDictionary::has_B() const {
  return get("B", "") != NULL;
}
