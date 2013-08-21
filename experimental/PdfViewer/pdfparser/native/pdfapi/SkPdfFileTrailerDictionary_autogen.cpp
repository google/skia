/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfFileTrailerDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

int64_t SkPdfFileTrailerDictionary::Size(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Size", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfFileTrailerDictionary::has_Size() const {
  return get("Size", "") != NULL;
}

int64_t SkPdfFileTrailerDictionary::Prev(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Prev", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfFileTrailerDictionary::has_Prev() const {
  return get("Prev", "") != NULL;
}

SkPdfDictionary* SkPdfFileTrailerDictionary::Root(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Root", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFileTrailerDictionary::has_Root() const {
  return get("Root", "") != NULL;
}

SkPdfDictionary* SkPdfFileTrailerDictionary::Encrypt(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Encrypt", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFileTrailerDictionary::has_Encrypt() const {
  return get("Encrypt", "") != NULL;
}

SkPdfDictionary* SkPdfFileTrailerDictionary::Info(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Info", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFileTrailerDictionary::has_Info() const {
  return get("Info", "") != NULL;
}

SkPdfArray* SkPdfFileTrailerDictionary::ID(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ID", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFileTrailerDictionary::has_ID() const {
  return get("ID", "") != NULL;
}
