/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfEncryptedEmbeddedFileStreamDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

int64_t SkPdfEncryptedEmbeddedFileStreamDictionary::EncryptionRevision(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("EncryptionRevision", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfEncryptedEmbeddedFileStreamDictionary::has_EncryptionRevision() const {
  return get("EncryptionRevision", "") != NULL;
}
