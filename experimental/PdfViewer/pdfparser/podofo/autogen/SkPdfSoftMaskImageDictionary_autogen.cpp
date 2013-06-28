#include "SkPdfSoftMaskImageDictionary_autogen.h"

SkPdfArray* SkPdfSoftMaskImageDictionary::Matte() const {
  SkPdfArray* ret;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Matte", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}
