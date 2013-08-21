/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfAlternateImageDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfStream* SkPdfAlternateImageDictionary::Image(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Image", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfAlternateImageDictionary::has_Image() const {
  return get("Image", "") != NULL;
}

bool SkPdfAlternateImageDictionary::DefaultForPrinting(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("DefaultForPrinting", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfAlternateImageDictionary::has_DefaultForPrinting() const {
  return get("DefaultForPrinting", "") != NULL;
}
