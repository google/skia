/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfSoundActionDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfSoundActionDictionary::S(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("S", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfSoundActionDictionary::has_S() const {
  return get("S", "") != NULL;
}

SkPdfStream* SkPdfSoundActionDictionary::Sound(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Sound", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfSoundActionDictionary::has_Sound() const {
  return get("Sound", "") != NULL;
}

double SkPdfSoundActionDictionary::Volume(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Volume", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfSoundActionDictionary::has_Volume() const {
  return get("Volume", "") != NULL;
}

bool SkPdfSoundActionDictionary::Synchronous(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Synchronous", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfSoundActionDictionary::has_Synchronous() const {
  return get("Synchronous", "") != NULL;
}

bool SkPdfSoundActionDictionary::Repeat(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Repeat", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfSoundActionDictionary::has_Repeat() const {
  return get("Repeat", "") != NULL;
}

bool SkPdfSoundActionDictionary::Mix(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Mix", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfSoundActionDictionary::has_Mix() const {
  return get("Mix", "") != NULL;
}
