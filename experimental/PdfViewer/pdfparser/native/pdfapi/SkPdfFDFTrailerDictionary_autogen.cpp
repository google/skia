#include "SkPdfFDFTrailerDictionary_autogen.h"


#include "SkPdfNativeDoc.h"
SkPdfDictionary* SkPdfFDFTrailerDictionary::Root(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Root", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFDFTrailerDictionary::has_Root() const {
  return get("Root", "") != NULL;
}
