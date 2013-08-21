/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPdfTrueTypeFontDictionary_DEFINED
#define SkPdfTrueTypeFontDictionary_DEFINED

#include "SkPdfType1FontDictionary_autogen.h"

class SkPdfTrueTypeFontDictionary : public SkPdfType1FontDictionary {
public:
public:
   SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() {return this;}
   const SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() const {return this;}

private:
   SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() {return (SkPdfMultiMasterFontDictionary*)this;}
   const SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() const {return (const SkPdfMultiMasterFontDictionary*)this;}

   SkPdfType3FontDictionary* asType3FontDictionary() {return (SkPdfType3FontDictionary*)this;}
   const SkPdfType3FontDictionary* asType3FontDictionary() const {return (const SkPdfType3FontDictionary*)this;}

public:
   bool valid() const {return true;}
  SkString Subtype(SkPdfNativeDoc* doc);
  bool has_Subtype() const;
};

#endif  // SkPdfTrueTypeFontDictionary_DEFINED
