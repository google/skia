/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfWebCaptureDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfWebCaptureDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfWebCaptureDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkString SkPdfWebCaptureDictionary::S(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("S", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfWebCaptureDictionary::has_S() const {
  return get("S", "") != NULL;
}

SkString SkPdfWebCaptureDictionary::ID(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ID", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfWebCaptureDictionary::has_ID() const {
  return get("ID", "") != NULL;
}

SkPdfArray* SkPdfWebCaptureDictionary::O(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("O", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfWebCaptureDictionary::has_O() const {
  return get("O", "") != NULL;
}

bool SkPdfWebCaptureDictionary::isSIADictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("SI", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isDictionary();
}

SkPdfDictionary* SkPdfWebCaptureDictionary::getSIAsDictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("SI", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfWebCaptureDictionary::isSIAArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("SI", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isArray();
}

SkPdfArray* SkPdfWebCaptureDictionary::getSIAsArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("SI", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfWebCaptureDictionary::has_SI() const {
  return get("SI", "") != NULL;
}

SkString SkPdfWebCaptureDictionary::CT(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CT", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfWebCaptureDictionary::has_CT() const {
  return get("CT", "") != NULL;
}

SkPdfDate SkPdfWebCaptureDictionary::TS(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDate()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->dateValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfDate();
}

bool SkPdfWebCaptureDictionary::has_TS() const {
  return get("TS", "") != NULL;
}
