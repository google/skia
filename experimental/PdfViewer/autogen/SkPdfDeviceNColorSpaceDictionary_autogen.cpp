#include "SkPdfDeviceNColorSpaceDictionary_autogen.h"

SkPdfDictionary* SkPdfDeviceNColorSpaceDictionary::Colorants() const {
  SkPdfDictionary* ret;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Colorants", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

