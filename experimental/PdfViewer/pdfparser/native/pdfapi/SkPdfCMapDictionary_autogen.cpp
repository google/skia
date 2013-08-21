/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfCMapDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfCMapDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfCMapDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkString SkPdfCMapDictionary::CMapName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CMapName", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfCMapDictionary::has_CMapName() const {
  return get("CMapName", "") != NULL;
}

bool SkPdfCMapDictionary::isCIDSystemInfoADictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CIDSystemInfo", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isDictionary();
}

SkPdfDictionary* SkPdfCMapDictionary::getCIDSystemInfoAsDictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CIDSystemInfo", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfCMapDictionary::isCIDSystemInfoAArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CIDSystemInfo", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isArray();
}

SkPdfArray* SkPdfCMapDictionary::getCIDSystemInfoAsArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CIDSystemInfo", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfCMapDictionary::has_CIDSystemInfo() const {
  return get("CIDSystemInfo", "") != NULL;
}

int64_t SkPdfCMapDictionary::WMode(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("WMode", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfCMapDictionary::has_WMode() const {
  return get("WMode", "") != NULL;
}

bool SkPdfCMapDictionary::isUseCMapAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("UseCMap", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfCMapDictionary::getUseCMapAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("UseCMap", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfCMapDictionary::isUseCMapAStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("UseCMap", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->hasStream();
}

SkPdfStream* SkPdfCMapDictionary::getUseCMapAsStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("UseCMap", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCMapDictionary::has_UseCMap() const {
  return get("UseCMap", "") != NULL;
}
