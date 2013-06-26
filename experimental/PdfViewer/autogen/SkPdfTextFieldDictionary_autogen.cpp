#include "SkPdfTextFieldDictionary_autogen.h"

long SkPdfTextFieldDictionary::MaxLen() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "MaxLen", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

