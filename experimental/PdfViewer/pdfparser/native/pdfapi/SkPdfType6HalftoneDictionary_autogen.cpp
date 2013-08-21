/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfType6HalftoneDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfType6HalftoneDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfType6HalftoneDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

int64_t SkPdfType6HalftoneDictionary::HalftoneType(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("HalftoneType", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfType6HalftoneDictionary::has_HalftoneType() const {
  return get("HalftoneType", "") != NULL;
}

SkString SkPdfType6HalftoneDictionary::HalftoneName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("HalftoneName", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfType6HalftoneDictionary::has_HalftoneName() const {
  return get("HalftoneName", "") != NULL;
}

int64_t SkPdfType6HalftoneDictionary::Width(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Width", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfType6HalftoneDictionary::has_Width() const {
  return get("Width", "") != NULL;
}

int64_t SkPdfType6HalftoneDictionary::Height(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Height", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfType6HalftoneDictionary::has_Height() const {
  return get("Height", "") != NULL;
}

bool SkPdfType6HalftoneDictionary::isTransferFunctionAFunction(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TransferFunction", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isFunction();
}

SkPdfFunction SkPdfType6HalftoneDictionary::getTransferFunctionAsFunction(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TransferFunction", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isFunction()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->functionValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfFunction();
}

bool SkPdfType6HalftoneDictionary::isTransferFunctionAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TransferFunction", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfType6HalftoneDictionary::getTransferFunctionAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TransferFunction", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfType6HalftoneDictionary::has_TransferFunction() const {
  return get("TransferFunction", "") != NULL;
}
