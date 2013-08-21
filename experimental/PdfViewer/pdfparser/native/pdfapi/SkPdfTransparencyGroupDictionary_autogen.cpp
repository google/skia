/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfTransparencyGroupDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfTransparencyGroupDictionary::S(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("S", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfTransparencyGroupDictionary::has_S() const {
  return get("S", "") != NULL;
}

bool SkPdfTransparencyGroupDictionary::isCSAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfTransparencyGroupDictionary::getCSAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfTransparencyGroupDictionary::isCSAArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isArray();
}

SkPdfArray* SkPdfTransparencyGroupDictionary::getCSAsArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfTransparencyGroupDictionary::has_CS() const {
  return get("CS", "") != NULL;
}

bool SkPdfTransparencyGroupDictionary::I(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("I", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfTransparencyGroupDictionary::has_I() const {
  return get("I", "") != NULL;
}

bool SkPdfTransparencyGroupDictionary::K(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("K", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfTransparencyGroupDictionary::has_K() const {
  return get("K", "") != NULL;
}
