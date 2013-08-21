/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfInlineLevelStructureElementsDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

bool SkPdfInlineLevelStructureElementsDictionary::isLineHeightANumber(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("LineHeight", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isNumber();
}

double SkPdfInlineLevelStructureElementsDictionary::getLineHeightAsNumber(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("LineHeight", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfInlineLevelStructureElementsDictionary::isLineHeightAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("LineHeight", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfInlineLevelStructureElementsDictionary::getLineHeightAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("LineHeight", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfInlineLevelStructureElementsDictionary::has_LineHeight() const {
  return get("LineHeight", "") != NULL;
}
