#include "SkPdfDctdecodeFilterDictionary_autogen.h"

long SkPdfDctdecodeFilterDictionary::ColorTransform() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ColorTransform", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}
