/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfEmbeddedFileParameterDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

int64_t SkPdfEmbeddedFileParameterDictionary::Size(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Size", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfEmbeddedFileParameterDictionary::has_Size() const {
  return get("Size", "") != NULL;
}

SkPdfDate SkPdfEmbeddedFileParameterDictionary::CreationDate(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CreationDate", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDate()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->dateValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfDate();
}

bool SkPdfEmbeddedFileParameterDictionary::has_CreationDate() const {
  return get("CreationDate", "") != NULL;
}

SkPdfDate SkPdfEmbeddedFileParameterDictionary::ModDate(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ModDate", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDate()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->dateValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfDate();
}

bool SkPdfEmbeddedFileParameterDictionary::has_ModDate() const {
  return get("ModDate", "") != NULL;
}

SkPdfDictionary* SkPdfEmbeddedFileParameterDictionary::Mac(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Mac", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfEmbeddedFileParameterDictionary::has_Mac() const {
  return get("Mac", "") != NULL;
}

SkString SkPdfEmbeddedFileParameterDictionary::CheckSum(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CheckSum", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfEmbeddedFileParameterDictionary::has_CheckSum() const {
  return get("CheckSum", "") != NULL;
}
