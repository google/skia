/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfEmbeddedFontStreamDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

int64_t SkPdfEmbeddedFontStreamDictionary::Length1(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Length1", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfEmbeddedFontStreamDictionary::has_Length1() const {
  return get("Length1", "") != NULL;
}

int64_t SkPdfEmbeddedFontStreamDictionary::Length2(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Length2", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfEmbeddedFontStreamDictionary::has_Length2() const {
  return get("Length2", "") != NULL;
}

int64_t SkPdfEmbeddedFontStreamDictionary::Length3(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Length3", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfEmbeddedFontStreamDictionary::has_Length3() const {
  return get("Length3", "") != NULL;
}

SkString SkPdfEmbeddedFontStreamDictionary::Subtype(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Subtype", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfEmbeddedFontStreamDictionary::has_Subtype() const {
  return get("Subtype", "") != NULL;
}

SkPdfStream* SkPdfEmbeddedFontStreamDictionary::Metadata(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Metadata", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfEmbeddedFontStreamDictionary::has_Metadata() const {
  return get("Metadata", "") != NULL;
}
