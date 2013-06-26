#include "SkPdfOpiVersionDictionary_autogen.h"

SkPdfDictionary* SkPdfOpiVersionDictionary::version_number() const {
  SkPdfDictionary* ret;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "version_number", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

