/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfType5HalftoneDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfType5HalftoneDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfType5HalftoneDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

double SkPdfType5HalftoneDictionary::HalftoneType(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("HalftoneType", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfType5HalftoneDictionary::has_HalftoneType() const {
  return get("HalftoneType", "") != NULL;
}

SkString SkPdfType5HalftoneDictionary::HalftoneName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("HalftoneName", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfType5HalftoneDictionary::has_HalftoneName() const {
  return get("HalftoneName", "") != NULL;
}
/*

bool SkPdfType5HalftoneDictionary::is[any_colorant_name]ADictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("[any_colorant_name]", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isDictionary();
}

SkPdfDictionary* SkPdfType5HalftoneDictionary::get[any_colorant_name]AsDictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("[any_colorant_name]", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfType5HalftoneDictionary::is[any_colorant_name]AStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("[any_colorant_name]", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->hasStream();
}

SkPdfStream* SkPdfType5HalftoneDictionary::get[any_colorant_name]AsStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("[any_colorant_name]", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfType5HalftoneDictionary::has_[any_colorant_name]() const {
  return get("[any_colorant_name]", "") != NULL;
}
*/

bool SkPdfType5HalftoneDictionary::isDefaultADictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Default", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isDictionary();
}

SkPdfDictionary* SkPdfType5HalftoneDictionary::getDefaultAsDictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Default", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfType5HalftoneDictionary::isDefaultAStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Default", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->hasStream();
}

SkPdfStream* SkPdfType5HalftoneDictionary::getDefaultAsStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Default", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfType5HalftoneDictionary::has_Default() const {
  return get("Default", "") != NULL;
}
