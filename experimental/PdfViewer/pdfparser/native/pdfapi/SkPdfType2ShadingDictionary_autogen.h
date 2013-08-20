#ifndef __DEFINED__SkPdfType2ShadingDictionary
#define __DEFINED__SkPdfType2ShadingDictionary

#include "SkPdfShadingDictionary_autogen.h"

// Additional entries specific to a type 2 shading dictionary
class SkPdfType2ShadingDictionary : public SkPdfShadingDictionary {
public:
public:
   SkPdfType2ShadingDictionary* asType2ShadingDictionary() {return this;}
   const SkPdfType2ShadingDictionary* asType2ShadingDictionary() const {return this;}

private:
   SkPdfType1ShadingDictionary* asType1ShadingDictionary() {return (SkPdfType1ShadingDictionary*)this;}
   const SkPdfType1ShadingDictionary* asType1ShadingDictionary() const {return (const SkPdfType1ShadingDictionary*)this;}

   SkPdfType3ShadingDictionary* asType3ShadingDictionary() {return (SkPdfType3ShadingDictionary*)this;}
   const SkPdfType3ShadingDictionary* asType3ShadingDictionary() const {return (const SkPdfType3ShadingDictionary*)this;}

   SkPdfType4ShadingDictionary* asType4ShadingDictionary() {return (SkPdfType4ShadingDictionary*)this;}
   const SkPdfType4ShadingDictionary* asType4ShadingDictionary() const {return (const SkPdfType4ShadingDictionary*)this;}

   SkPdfType5ShadingDictionary* asType5ShadingDictionary() {return (SkPdfType5ShadingDictionary*)this;}
   const SkPdfType5ShadingDictionary* asType5ShadingDictionary() const {return (const SkPdfType5ShadingDictionary*)this;}

   SkPdfType6ShadingDictionary* asType6ShadingDictionary() {return (SkPdfType6ShadingDictionary*)this;}
   const SkPdfType6ShadingDictionary* asType6ShadingDictionary() const {return (const SkPdfType6ShadingDictionary*)this;}

public:
   bool valid() const {return true;}
  SkPdfArray* Coords(SkPdfNativeDoc* doc);
  bool has_Coords() const;
  SkPdfArray* Domain(SkPdfNativeDoc* doc);
  bool has_Domain() const;
  SkPdfFunction Function(SkPdfNativeDoc* doc);
  bool has_Function() const;
  SkPdfArray* Extend(SkPdfNativeDoc* doc);
  bool has_Extend() const;
};

#endif  // __DEFINED__NATIVE_SkPdfType2ShadingDictionary
