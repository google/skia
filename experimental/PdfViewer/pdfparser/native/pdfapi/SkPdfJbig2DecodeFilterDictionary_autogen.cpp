#include "SkPdfJbig2DecodeFilterDictionary_autogen.h"


#include "SkPdfNativeDoc.h"
SkPdfStream* SkPdfJbig2DecodeFilterDictionary::JBIG2Globals(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("JBIG2Globals", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfJbig2DecodeFilterDictionary::has_JBIG2Globals() const {
  return get("JBIG2Globals", "") != NULL;
}
