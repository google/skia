/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPdfType0FontDictionary_DEFINED
#define SkPdfType0FontDictionary_DEFINED

#include "SkPdfFontDictionary_autogen.h"

// Entries in a Type 0 font dictionary
class SkPdfType0FontDictionary : public SkPdfFontDictionary {
public:
public:
   SkPdfType0FontDictionary* asType0FontDictionary() {return this;}
   const SkPdfType0FontDictionary* asType0FontDictionary() const {return this;}

private:
   SkPdfType1FontDictionary* asType1FontDictionary() {return (SkPdfType1FontDictionary*)this;}
   const SkPdfType1FontDictionary* asType1FontDictionary() const {return (const SkPdfType1FontDictionary*)this;}

   SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() {return (SkPdfMultiMasterFontDictionary*)this;}
   const SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() const {return (const SkPdfMultiMasterFontDictionary*)this;}

   SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() {return (SkPdfTrueTypeFontDictionary*)this;}
   const SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() const {return (const SkPdfTrueTypeFontDictionary*)this;}

   SkPdfType3FontDictionary* asType3FontDictionary() {return (SkPdfType3FontDictionary*)this;}
   const SkPdfType3FontDictionary* asType3FontDictionary() const {return (const SkPdfType3FontDictionary*)this;}

public:
   bool valid() const {return true;}
  SkString Type(SkPdfNativeDoc* doc);
  bool has_Type() const;
  SkString Subtype(SkPdfNativeDoc* doc);
  bool has_Subtype() const;
  SkString BaseFont(SkPdfNativeDoc* doc);
  bool has_BaseFont() const;

  bool isEncodingAName(SkPdfNativeDoc* doc);
  SkString getEncodingAsName(SkPdfNativeDoc* doc);

  bool isEncodingAStream(SkPdfNativeDoc* doc);
  SkPdfStream* getEncodingAsStream(SkPdfNativeDoc* doc);
  bool has_Encoding() const;
  SkPdfArray* DescendantFonts(SkPdfNativeDoc* doc);
  bool has_DescendantFonts() const;
  SkPdfStream* ToUnicode(SkPdfNativeDoc* doc);
  bool has_ToUnicode() const;
};

#endif  // SkPdfType0FontDictionary_DEFINED
