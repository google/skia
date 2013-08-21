/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfShadingDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

int64_t SkPdfShadingDictionary::ShadingType(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ShadingType", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfShadingDictionary::has_ShadingType() const {
  return get("ShadingType", "") != NULL;
}

bool SkPdfShadingDictionary::isColorSpaceAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ColorSpace", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfShadingDictionary::getColorSpaceAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ColorSpace", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfShadingDictionary::isColorSpaceAArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ColorSpace", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isArray();
}

SkPdfArray* SkPdfShadingDictionary::getColorSpaceAsArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ColorSpace", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfShadingDictionary::has_ColorSpace() const {
  return get("ColorSpace", "") != NULL;
}

SkPdfArray* SkPdfShadingDictionary::Background(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Background", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfShadingDictionary::has_Background() const {
  return get("Background", "") != NULL;
}

SkRect SkPdfShadingDictionary::BBox(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BBox", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isRectangle()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->rectangleValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkRect::MakeEmpty();
}

bool SkPdfShadingDictionary::has_BBox() const {
  return get("BBox", "") != NULL;
}

bool SkPdfShadingDictionary::AntiAlias(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("AntiAlias", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfShadingDictionary::has_AntiAlias() const {
  return get("AntiAlias", "") != NULL;
}
