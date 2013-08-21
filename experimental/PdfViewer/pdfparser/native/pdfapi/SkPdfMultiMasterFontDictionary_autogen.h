/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPdfMultiMasterFontDictionary_DEFINED
#define SkPdfMultiMasterFontDictionary_DEFINED

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

#endif  // SkPdfMultiMasterFontDictionary_DEFINED
