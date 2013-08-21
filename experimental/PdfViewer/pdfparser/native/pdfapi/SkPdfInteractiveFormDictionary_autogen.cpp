/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfInteractiveFormDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfArray* SkPdfInteractiveFormDictionary::Fields(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Fields", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfInteractiveFormDictionary::has_Fields() const {
  return get("Fields", "") != NULL;
}

bool SkPdfInteractiveFormDictionary::NeedAppearances(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("NeedAppearances", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfInteractiveFormDictionary::has_NeedAppearances() const {
  return get("NeedAppearances", "") != NULL;
}

int64_t SkPdfInteractiveFormDictionary::SigFlags(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("SigFlags", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfInteractiveFormDictionary::has_SigFlags() const {
  return get("SigFlags", "") != NULL;
}

SkPdfArray* SkPdfInteractiveFormDictionary::CO(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CO", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfInteractiveFormDictionary::has_CO() const {
  return get("CO", "") != NULL;
}

SkPdfDictionary* SkPdfInteractiveFormDictionary::DR(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("DR", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfInteractiveFormDictionary::has_DR() const {
  return get("DR", "") != NULL;
}

SkString SkPdfInteractiveFormDictionary::DA(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("DA", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfInteractiveFormDictionary::has_DA() const {
  return get("DA", "") != NULL;
}

int64_t SkPdfInteractiveFormDictionary::Q(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Q", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfInteractiveFormDictionary::has_Q() const {
  return get("Q", "") != NULL;
}
