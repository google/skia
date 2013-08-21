/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfIccProfileStreamDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

int64_t SkPdfIccProfileStreamDictionary::N(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("N", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfIccProfileStreamDictionary::has_N() const {
  return get("N", "") != NULL;
}

bool SkPdfIccProfileStreamDictionary::isAlternateAArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Alternate", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isArray();
}

SkPdfArray* SkPdfIccProfileStreamDictionary::getAlternateAsArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Alternate", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfIccProfileStreamDictionary::isAlternateAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Alternate", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfIccProfileStreamDictionary::getAlternateAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Alternate", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfIccProfileStreamDictionary::has_Alternate() const {
  return get("Alternate", "") != NULL;
}

SkPdfArray* SkPdfIccProfileStreamDictionary::Range(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Range", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfIccProfileStreamDictionary::has_Range() const {
  return get("Range", "") != NULL;
}

SkPdfStream* SkPdfIccProfileStreamDictionary::Metadata(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Metadata", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfIccProfileStreamDictionary::has_Metadata() const {
  return get("Metadata", "") != NULL;
}
