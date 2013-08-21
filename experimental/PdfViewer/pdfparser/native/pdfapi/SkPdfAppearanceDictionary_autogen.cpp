/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfAppearanceDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

bool SkPdfAppearanceDictionary::isNAStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("N", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->hasStream();
}

SkPdfStream* SkPdfAppearanceDictionary::getNAsStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("N", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfAppearanceDictionary::isNADictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("N", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isDictionary();
}

SkPdfDictionary* SkPdfAppearanceDictionary::getNAsDictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("N", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfAppearanceDictionary::has_N() const {
  return get("N", "") != NULL;
}

bool SkPdfAppearanceDictionary::isRAStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("R", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->hasStream();
}

SkPdfStream* SkPdfAppearanceDictionary::getRAsStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("R", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfAppearanceDictionary::isRADictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("R", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isDictionary();
}

SkPdfDictionary* SkPdfAppearanceDictionary::getRAsDictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("R", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfAppearanceDictionary::has_R() const {
  return get("R", "") != NULL;
}

bool SkPdfAppearanceDictionary::isDAStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("D", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->hasStream();
}

SkPdfStream* SkPdfAppearanceDictionary::getDAsStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("D", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfAppearanceDictionary::isDADictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("D", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isDictionary();
}

SkPdfDictionary* SkPdfAppearanceDictionary::getDAsDictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("D", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfAppearanceDictionary::has_D() const {
  return get("D", "") != NULL;
}
