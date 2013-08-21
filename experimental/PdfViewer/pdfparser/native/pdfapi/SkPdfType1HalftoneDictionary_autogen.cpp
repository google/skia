/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfType1HalftoneDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfType1HalftoneDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfType1HalftoneDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

int64_t SkPdfType1HalftoneDictionary::HalftoneType(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("HalftoneType", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfType1HalftoneDictionary::has_HalftoneType() const {
  return get("HalftoneType", "") != NULL;
}

SkString SkPdfType1HalftoneDictionary::HalftoneName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("HalftoneName", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfType1HalftoneDictionary::has_HalftoneName() const {
  return get("HalftoneName", "") != NULL;
}

double SkPdfType1HalftoneDictionary::Frequency(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Frequency", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfType1HalftoneDictionary::has_Frequency() const {
  return get("Frequency", "") != NULL;
}

double SkPdfType1HalftoneDictionary::Angle(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Angle", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfType1HalftoneDictionary::has_Angle() const {
  return get("Angle", "") != NULL;
}

bool SkPdfType1HalftoneDictionary::isSpotFunctionAFunction(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("SpotFunction", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isFunction();
}

SkPdfFunction SkPdfType1HalftoneDictionary::getSpotFunctionAsFunction(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("SpotFunction", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isFunction()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->functionValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkPdfFunction();
}

bool SkPdfType1HalftoneDictionary::isSpotFunctionAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("SpotFunction", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfType1HalftoneDictionary::getSpotFunctionAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("SpotFunction", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfType1HalftoneDictionary::has_SpotFunction() const {
  return get("SpotFunction", "") != NULL;
}

bool SkPdfType1HalftoneDictionary::AccurateScreens(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("AccurateScreens", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfType1HalftoneDictionary::has_AccurateScreens() const {
  return get("AccurateScreens", "") != NULL;
}

bool SkPdfType1HalftoneDictionary::isTransferFunctionAFunction(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TransferFunction", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isFunction();
}

SkPdfFunction SkPdfType1HalftoneDictionary::getTransferFunctionAsFunction(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TransferFunction", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isFunction()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->functionValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfFunction();
}

bool SkPdfType1HalftoneDictionary::isTransferFunctionAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TransferFunction", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfType1HalftoneDictionary::getTransferFunctionAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TransferFunction", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfType1HalftoneDictionary::has_TransferFunction() const {
  return get("TransferFunction", "") != NULL;
}
