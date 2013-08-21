/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfCalrgbColorSpaceDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfArray* SkPdfCalrgbColorSpaceDictionary::WhitePoint(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("WhitePoint", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfCalrgbColorSpaceDictionary::has_WhitePoint() const {
  return get("WhitePoint", "") != NULL;
}

SkPdfArray* SkPdfCalrgbColorSpaceDictionary::BlackPoint(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BlackPoint", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCalrgbColorSpaceDictionary::has_BlackPoint() const {
  return get("BlackPoint", "") != NULL;
}

SkPdfArray* SkPdfCalrgbColorSpaceDictionary::Gamma(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Gamma", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCalrgbColorSpaceDictionary::has_Gamma() const {
  return get("Gamma", "") != NULL;
}

SkPdfArray* SkPdfCalrgbColorSpaceDictionary::Matrix(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Matrix", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCalrgbColorSpaceDictionary::has_Matrix() const {
  return get("Matrix", "") != NULL;
}
