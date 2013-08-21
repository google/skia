/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfSoftMaskDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfSoftMaskDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfSoftMaskDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkString SkPdfSoftMaskDictionary::S(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("S", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfSoftMaskDictionary::has_S() const {
  return get("S", "") != NULL;
}

SkPdfStream* SkPdfSoftMaskDictionary::G(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("G", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfSoftMaskDictionary::has_G() const {
  return get("G", "") != NULL;
}

SkPdfArray* SkPdfSoftMaskDictionary::BC(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BC", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfSoftMaskDictionary::has_BC() const {
  return get("BC", "") != NULL;
}

bool SkPdfSoftMaskDictionary::isTRAFunction(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TR", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isFunction();
}

SkPdfFunction SkPdfSoftMaskDictionary::getTRAsFunction(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TR", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isFunction()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->functionValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfFunction();
}

bool SkPdfSoftMaskDictionary::isTRAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TR", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfSoftMaskDictionary::getTRAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TR", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfSoftMaskDictionary::has_TR() const {
  return get("TR", "") != NULL;
}
