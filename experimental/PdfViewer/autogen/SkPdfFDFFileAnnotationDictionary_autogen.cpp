#include "SkPdfFDFFileAnnotationDictionary_autogen.h"

long SkPdfFDFFileAnnotationDictionary::Page() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Page", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

