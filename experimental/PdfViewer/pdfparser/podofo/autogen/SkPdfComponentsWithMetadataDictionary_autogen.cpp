#include "SkPdfComponentsWithMetadataDictionary_autogen.h"

SkPdfStream* SkPdfComponentsWithMetadataDictionary::Metadata() const {
  SkPdfStream* ret;
  if (StreamFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Metadata", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}
