#include "SkPdfJbig2DecodeFilterDictionary_autogen.h"

SkPdfStream* SkPdfJbig2DecodeFilterDictionary::JBIG2Globals() const {
  SkPdfStream* ret;
  if (StreamFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "JBIG2Globals", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

