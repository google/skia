#ifndef __DEFINED__SkPdfMultiMasterFontDictionary
#define __DEFINED__SkPdfMultiMasterFontDictionary

#include "SkPdfType1FontDictionary_autogen.h"

class SkPdfMultiMasterFontDictionary : public SkPdfType1FontDictionary {
public:
public:
   SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() {return this;}
   const SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() const {return this;}

private:
   SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() {return (SkPdfTrueTypeFontDictionary*)this;}
   const SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() const {return (const SkPdfTrueTypeFontDictionary*)this;}

   SkPdfType3FontDictionary* asType3FontDictionary() {return (SkPdfType3FontDictionary*)this;}
   const SkPdfType3FontDictionary* asType3FontDictionary() const {return (const SkPdfType3FontDictionary*)this;}

public:
   bool valid() const {return true;}
  SkString Subtype(SkPdfNativeDoc* doc);
  bool has_Subtype() const;
};

#endif  // __DEFINED__NATIVE_SkPdfMultiMasterFontDictionary
