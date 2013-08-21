/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfMovieDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfFileSpec SkPdfMovieDictionary::F(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("F", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && false) || (doc == NULL && ret != NULL && ret->isReference())) return ret->fileSpecValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkPdfFileSpec();
}

bool SkPdfMovieDictionary::has_F() const {
  return get("F", "") != NULL;
}

SkPdfArray* SkPdfMovieDictionary::Aspect(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Aspect", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfMovieDictionary::has_Aspect() const {
  return get("Aspect", "") != NULL;
}

int64_t SkPdfMovieDictionary::Rotate(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Rotate", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfMovieDictionary::has_Rotate() const {
  return get("Rotate", "") != NULL;
}

bool SkPdfMovieDictionary::isPosterABoolean(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Poster", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isBoolean();
}

bool SkPdfMovieDictionary::getPosterAsBoolean(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Poster", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfMovieDictionary::isPosterAStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Poster", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->hasStream();
}

SkPdfStream* SkPdfMovieDictionary::getPosterAsStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Poster", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfMovieDictionary::has_Poster() const {
  return get("Poster", "") != NULL;
}
