/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfType10HalftoneDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfType10HalftoneDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfType10HalftoneDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

int64_t SkPdfType10HalftoneDictionary::HalftoneType(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("HalftoneType", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfType10HalftoneDictionary::has_HalftoneType() const {
  return get("HalftoneType", "") != NULL;
}

SkString SkPdfType10HalftoneDictionary::HalftoneName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("HalftoneName", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfType10HalftoneDictionary::has_HalftoneName() const {
  return get("HalftoneName", "") != NULL;
}

int64_t SkPdfType10HalftoneDictionary::Xsquare(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Xsquare", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfType10HalftoneDictionary::has_Xsquare() const {
  return get("Xsquare", "") != NULL;
}

int64_t SkPdfType10HalftoneDictionary::Ysquare(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Ysquare", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfType10HalftoneDictionary::has_Ysquare() const {
  return get("Ysquare", "") != NULL;
}

bool SkPdfType10HalftoneDictionary::isTransferFunctionAFunction(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TransferFunction", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isFunction();
}

SkPdfFunction SkPdfType10HalftoneDictionary::getTransferFunctionAsFunction(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TransferFunction", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isFunction()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->functionValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfFunction();
}

bool SkPdfType10HalftoneDictionary::isTransferFunctionAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TransferFunction", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfType10HalftoneDictionary::getTransferFunctionAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TransferFunction", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfType10HalftoneDictionary::has_TransferFunction() const {
  return get("TransferFunction", "") != NULL;
}
