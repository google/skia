#include "SkPdfEncryptedEmbeddedFileStreamDictionary_autogen.h"

long SkPdfEncryptedEmbeddedFileStreamDictionary::EncryptionRevision() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "EncryptionRevision", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}
