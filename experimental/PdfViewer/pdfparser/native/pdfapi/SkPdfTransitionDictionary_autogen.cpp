/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfTransitionDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfTransitionDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfTransitionDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

double SkPdfTransitionDictionary::D(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("D", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfTransitionDictionary::has_D() const {
  return get("D", "") != NULL;
}

SkString SkPdfTransitionDictionary::S(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("S", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfTransitionDictionary::has_S() const {
  return get("S", "") != NULL;
}

SkString SkPdfTransitionDictionary::Dm(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Dm", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfTransitionDictionary::has_Dm() const {
  return get("Dm", "") != NULL;
}

SkString SkPdfTransitionDictionary::M(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("M", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfTransitionDictionary::has_M() const {
  return get("M", "") != NULL;
}

double SkPdfTransitionDictionary::Di(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Di", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfTransitionDictionary::has_Di() const {
  return get("Di", "") != NULL;
}
