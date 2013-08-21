/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfRemoteGoToActionDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfRemoteGoToActionDictionary::S(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("S", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfRemoteGoToActionDictionary::has_S() const {
  return get("S", "") != NULL;
}

SkPdfFileSpec SkPdfRemoteGoToActionDictionary::F(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("F", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && false) || (doc == NULL && ret != NULL && ret->isReference())) return ret->fileSpecValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkPdfFileSpec();
}

bool SkPdfRemoteGoToActionDictionary::has_F() const {
  return get("F", "") != NULL;
}

bool SkPdfRemoteGoToActionDictionary::isDAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("D", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfRemoteGoToActionDictionary::getDAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("D", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfRemoteGoToActionDictionary::isDAString(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("D", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isAnyString();
}

SkString SkPdfRemoteGoToActionDictionary::getDAsString(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("D", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfRemoteGoToActionDictionary::isDAArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("D", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isArray();
}

SkPdfArray* SkPdfRemoteGoToActionDictionary::getDAsArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("D", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfRemoteGoToActionDictionary::has_D() const {
  return get("D", "") != NULL;
}

bool SkPdfRemoteGoToActionDictionary::NewWindow(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("NewWindow", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfRemoteGoToActionDictionary::has_NewWindow() const {
  return get("NewWindow", "") != NULL;
}
