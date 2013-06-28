#include "SkPdfRadioButtonFieldDictionary_autogen.h"

SkPdfArray* SkPdfRadioButtonFieldDictionary::Opt() const {
  SkPdfArray* ret;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Opt", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}
