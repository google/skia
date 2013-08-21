/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPdfType1ShadingDictionary_DEFINED
#define SkPdfType1ShadingDictionary_DEFINED

#include "SkPdfShadingDictionary_autogen.h"

// Additional entries specific to a type 1 shading dictionary
class SkPdfType1ShadingDictionary : public SkPdfShadingDictionary {
public:
public:
   SkPdfType1ShadingDictionary* asType1ShadingDictionary() {return this;}
   const SkPdfType1ShadingDictionary* asType1ShadingDictionary() const {return this;}

private:
   SkPdfType2ShadingDictionary* asType2ShadingDictionary() {return (SkPdfType2ShadingDictionary*)this;}
   const SkPdfType2ShadingDictionary* asType2ShadingDictionary() const {return (const SkPdfType2ShadingDictionary*)this;}

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
  SkPdfArray* Domain(SkPdfNativeDoc* doc);
  bool has_Domain() const;
  SkPdfArray* Matrix(SkPdfNativeDoc* doc);
  bool has_Matrix() const;
  SkPdfFunction Function(SkPdfNativeDoc* doc);
  bool has_Function() const;
};

#endif  // SkPdfType1ShadingDictionary_DEFINED
