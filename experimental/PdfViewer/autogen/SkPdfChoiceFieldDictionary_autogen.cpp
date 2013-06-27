#include "SkPdfChoiceFieldDictionary_autogen.h"

SkPdfArray* SkPdfChoiceFieldDictionary::Opt() const {
  SkPdfArray* ret;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Opt", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

long SkPdfChoiceFieldDictionary::TI() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "TI", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

SkPdfArray* SkPdfChoiceFieldDictionary::I() const {
  SkPdfArray* ret;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "I", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

