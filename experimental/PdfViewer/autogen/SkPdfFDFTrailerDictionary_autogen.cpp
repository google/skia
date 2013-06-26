#include "SkPdfFDFTrailerDictionary_autogen.h"

SkPdfDictionary* SkPdfFDFTrailerDictionary::Root() const {
  SkPdfDictionary* ret;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Root", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

