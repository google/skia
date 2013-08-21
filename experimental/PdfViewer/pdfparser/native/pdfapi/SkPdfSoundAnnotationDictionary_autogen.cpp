/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfSoundAnnotationDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfSoundAnnotationDictionary::Subtype(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Subtype", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfSoundAnnotationDictionary::has_Subtype() const {
  return get("Subtype", "") != NULL;
}

SkPdfStream* SkPdfSoundAnnotationDictionary::Sound(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Sound", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfSoundAnnotationDictionary::has_Sound() const {
  return get("Sound", "") != NULL;
}

SkString SkPdfSoundAnnotationDictionary::Contents(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Contents", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfSoundAnnotationDictionary::has_Contents() const {
  return get("Contents", "") != NULL;
}

SkString SkPdfSoundAnnotationDictionary::Name(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Name", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfSoundAnnotationDictionary::has_Name() const {
  return get("Name", "") != NULL;
}
