/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPdfType3ShadingDictionary_DEFINED
#define SkPdfType3ShadingDictionary_DEFINED

#include "SkPdfShadingDictionary_autogen.h"

// Additional entries specific to a type 3 shading dictionary
class SkPdfType3ShadingDictionary : public SkPdfShadingDictionary {
public:
public:
   SkPdfType3ShadingDictionary* asType3ShadingDictionary() {return this;}
   const SkPdfType3ShadingDictionary* asType3ShadingDictionary() const {return this;}

private:
   SkPdfType1ShadingDictionary* asType1ShadingDictionary() {return (SkPdfType1ShadingDictionary*)this;}
   const SkPdfType1ShadingDictionary* asType1ShadingDictionary() const {return (const SkPdfType1ShadingDictionary*)this;}

   SkPdfType2ShadingDictionary* asType2ShadingDictionary() {return (SkPdfType2ShadingDictionary*)this;}
   const SkPdfType2ShadingDictionary* asType2ShadingDictionary() const {return (const SkPdfType2ShadingDictionary*)this;}

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

#endif  // SkPdfType3ShadingDictionary_DEFINED
