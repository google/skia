/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfAppearanceCharacteristicsDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

int64_t SkPdfAppearanceCharacteristicsDictionary::R(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("R", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfAppearanceCharacteristicsDictionary::has_R() const {
  return get("R", "") != NULL;
}

SkPdfArray* SkPdfAppearanceCharacteristicsDictionary::BC(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BC", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfAppearanceCharacteristicsDictionary::has_BC() const {
  return get("BC", "") != NULL;
}

SkPdfArray* SkPdfAppearanceCharacteristicsDictionary::BG(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BG", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfAppearanceCharacteristicsDictionary::has_BG() const {
  return get("BG", "") != NULL;
}

SkString SkPdfAppearanceCharacteristicsDictionary::CA(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CA", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfAppearanceCharacteristicsDictionary::has_CA() const {
  return get("CA", "") != NULL;
}

SkString SkPdfAppearanceCharacteristicsDictionary::RC(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("RC", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfAppearanceCharacteristicsDictionary::has_RC() const {
  return get("RC", "") != NULL;
}

SkString SkPdfAppearanceCharacteristicsDictionary::AC(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("AC", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfAppearanceCharacteristicsDictionary::has_AC() const {
  return get("AC", "") != NULL;
}

SkPdfStream* SkPdfAppearanceCharacteristicsDictionary::I(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("I", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfAppearanceCharacteristicsDictionary::has_I() const {
  return get("I", "") != NULL;
}

SkPdfStream* SkPdfAppearanceCharacteristicsDictionary::RI(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("RI", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfAppearanceCharacteristicsDictionary::has_RI() const {
  return get("RI", "") != NULL;
}

SkPdfStream* SkPdfAppearanceCharacteristicsDictionary::IX(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("IX", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfAppearanceCharacteristicsDictionary::has_IX() const {
  return get("IX", "") != NULL;
}

SkPdfDictionary* SkPdfAppearanceCharacteristicsDictionary::IF(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("IF", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfAppearanceCharacteristicsDictionary::has_IF() const {
  return get("IF", "") != NULL;
}

int64_t SkPdfAppearanceCharacteristicsDictionary::TP(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TP", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfAppearanceCharacteristicsDictionary::has_TP() const {
  return get("TP", "") != NULL;
}
