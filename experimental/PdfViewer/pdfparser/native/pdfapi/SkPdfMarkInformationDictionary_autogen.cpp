/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfMarkInformationDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

bool SkPdfMarkInformationDictionary::Marked(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Marked", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfMarkInformationDictionary::has_Marked() const {
  return get("Marked", "") != NULL;
}
