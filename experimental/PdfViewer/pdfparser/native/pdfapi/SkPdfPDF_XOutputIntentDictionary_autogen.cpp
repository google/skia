/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfPDF_XOutputIntentDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfPDF_XOutputIntentDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfPDF_XOutputIntentDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkString SkPdfPDF_XOutputIntentDictionary::S(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("S", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfPDF_XOutputIntentDictionary::has_S() const {
  return get("S", "") != NULL;
}

SkString SkPdfPDF_XOutputIntentDictionary::OutputCondition(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("OutputCondition", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfPDF_XOutputIntentDictionary::has_OutputCondition() const {
  return get("OutputCondition", "") != NULL;
}

SkString SkPdfPDF_XOutputIntentDictionary::OutputConditionIdentifier(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("OutputConditionIdentifier", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfPDF_XOutputIntentDictionary::has_OutputConditionIdentifier() const {
  return get("OutputConditionIdentifier", "") != NULL;
}

SkString SkPdfPDF_XOutputIntentDictionary::RegistryName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("RegistryName", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfPDF_XOutputIntentDictionary::has_RegistryName() const {
  return get("RegistryName", "") != NULL;
}

SkString SkPdfPDF_XOutputIntentDictionary::Info(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Info", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfPDF_XOutputIntentDictionary::has_Info() const {
  return get("Info", "") != NULL;
}

SkPdfStream* SkPdfPDF_XOutputIntentDictionary::DestOutputProfile(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("DestOutputProfile", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfPDF_XOutputIntentDictionary::has_DestOutputProfile() const {
  return get("DestOutputProfile", "") != NULL;
}
