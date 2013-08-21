/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfNamedActionsDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfNamedActionsDictionary::S(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("S", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfNamedActionsDictionary::has_S() const {
  return get("S", "") != NULL;
}

SkString SkPdfNamedActionsDictionary::N(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("N", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfNamedActionsDictionary::has_N() const {
  return get("N", "") != NULL;
}
