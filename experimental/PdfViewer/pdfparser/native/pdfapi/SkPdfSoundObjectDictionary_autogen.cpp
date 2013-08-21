/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfSoundObjectDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfSoundObjectDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfSoundObjectDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

double SkPdfSoundObjectDictionary::R(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("R", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfSoundObjectDictionary::has_R() const {
  return get("R", "") != NULL;
}

int64_t SkPdfSoundObjectDictionary::C(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("C", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfSoundObjectDictionary::has_C() const {
  return get("C", "") != NULL;
}

int64_t SkPdfSoundObjectDictionary::B(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("B", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfSoundObjectDictionary::has_B() const {
  return get("B", "") != NULL;
}

SkString SkPdfSoundObjectDictionary::E(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("E", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfSoundObjectDictionary::has_E() const {
  return get("E", "") != NULL;
}

SkString SkPdfSoundObjectDictionary::CO(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CO", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfSoundObjectDictionary::has_CO() const {
  return get("CO", "") != NULL;
}

SkPdfNativeObject* SkPdfSoundObjectDictionary::CP(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CP", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && true) || (doc == NULL && ret != NULL && ret->isReference())) return ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfSoundObjectDictionary::has_CP() const {
  return get("CP", "") != NULL;
}
