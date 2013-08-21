/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfDeviceNColorSpaceDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfDictionary* SkPdfDeviceNColorSpaceDictionary::Colorants(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Colorants", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfDeviceNColorSpaceDictionary::has_Colorants() const {
  return get("Colorants", "") != NULL;
}
