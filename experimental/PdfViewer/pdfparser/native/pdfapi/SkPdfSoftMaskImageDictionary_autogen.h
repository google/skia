#ifndef __DEFINED__SkPdfSoftMaskImageDictionary
#define __DEFINED__SkPdfSoftMaskImageDictionary

#include "SkPdfImageDictionary_autogen.h"

// Additional entry in a soft-mask image dictionary
class SkPdfSoftMaskImageDictionary : public SkPdfImageDictionary {
public:
public:
   SkPdfSoftMaskImageDictionary* asSoftMaskImageDictionary() {return this;}
   const SkPdfSoftMaskImageDictionary* asSoftMaskImageDictionary() const {return this;}

private:
public:
   bool valid() const {return true;}
  SkPdfArray* Matte(SkPdfNativeDoc* doc);
  bool has_Matte() const;
  SkString Subtype(SkPdfNativeDoc* doc);
  bool has_Subtype() const;
  SkString ColorSpace(SkPdfNativeDoc* doc);
  bool has_ColorSpace() const;
};

#endif  // __DEFINED__NATIVE_SkPdfSoftMaskImageDictionary
