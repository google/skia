/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfListAttributeDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfListAttributeDictionary::ListNumbering(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ListNumbering", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfListAttributeDictionary::has_ListNumbering() const {
  return get("ListNumbering", "") != NULL;
}
