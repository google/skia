/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfBoxColorInformationDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfDictionary* SkPdfBoxColorInformationDictionary::CropBox(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CropBox", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfBoxColorInformationDictionary::has_CropBox() const {
  return get("CropBox", "") != NULL;
}

SkPdfDictionary* SkPdfBoxColorInformationDictionary::BleedBox(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BleedBox", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfBoxColorInformationDictionary::has_BleedBox() const {
  return get("BleedBox", "") != NULL;
}

SkPdfDictionary* SkPdfBoxColorInformationDictionary::TrimBox(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TrimBox", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfBoxColorInformationDictionary::has_TrimBox() const {
  return get("TrimBox", "") != NULL;
}

SkPdfDictionary* SkPdfBoxColorInformationDictionary::ArtBox(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ArtBox", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfBoxColorInformationDictionary::has_ArtBox() const {
  return get("ArtBox", "") != NULL;
}
