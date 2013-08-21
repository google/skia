/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfSeparationDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfArray* SkPdfSeparationDictionary::Pages(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Pages", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfSeparationDictionary::has_Pages() const {
  return get("Pages", "") != NULL;
}

bool SkPdfSeparationDictionary::isDeviceColorantAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("DeviceColorant", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfSeparationDictionary::getDeviceColorantAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("DeviceColorant", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfSeparationDictionary::isDeviceColorantAString(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("DeviceColorant", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isAnyString();
}

SkString SkPdfSeparationDictionary::getDeviceColorantAsString(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("DeviceColorant", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfSeparationDictionary::has_DeviceColorant() const {
  return get("DeviceColorant", "") != NULL;
}

SkPdfArray* SkPdfSeparationDictionary::ColorSpace(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ColorSpace", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfSeparationDictionary::has_ColorSpace() const {
  return get("ColorSpace", "") != NULL;
}
