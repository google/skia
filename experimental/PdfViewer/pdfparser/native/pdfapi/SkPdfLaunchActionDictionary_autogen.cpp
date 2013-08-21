/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfLaunchActionDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfLaunchActionDictionary::S(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("S", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfLaunchActionDictionary::has_S() const {
  return get("S", "") != NULL;
}

SkPdfFileSpec SkPdfLaunchActionDictionary::F(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("F", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && false) || (doc == NULL && ret != NULL && ret->isReference())) return ret->fileSpecValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfFileSpec();
}

bool SkPdfLaunchActionDictionary::has_F() const {
  return get("F", "") != NULL;
}

SkPdfDictionary* SkPdfLaunchActionDictionary::Win(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Win", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfLaunchActionDictionary::has_Win() const {
  return get("Win", "") != NULL;
}

SkPdfNativeObject* SkPdfLaunchActionDictionary::Mac(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Mac", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && true) || (doc == NULL && ret != NULL && ret->isReference())) return ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfLaunchActionDictionary::has_Mac() const {
  return get("Mac", "") != NULL;
}

SkPdfNativeObject* SkPdfLaunchActionDictionary::Unix(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Unix", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && true) || (doc == NULL && ret != NULL && ret->isReference())) return ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfLaunchActionDictionary::has_Unix() const {
  return get("Unix", "") != NULL;
}

bool SkPdfLaunchActionDictionary::NewWindow(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("NewWindow", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfLaunchActionDictionary::has_NewWindow() const {
  return get("NewWindow", "") != NULL;
}
