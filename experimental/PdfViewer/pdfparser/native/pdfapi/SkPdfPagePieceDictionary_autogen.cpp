/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfPagePieceDictionary_autogen.h"
#include "SkPdfNativeDoc.h"
/*

SkPdfDictionary* SkPdfPagePieceDictionary::[any_application_name_or_well_known_data_type](SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("[any_application_name_or_well_known_data_type]", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfPagePieceDictionary::has_[any_application_name_or_well_known_data_type]() const {
  return get("[any_application_name_or_well_known_data_type]", "") != NULL;
}
*/
