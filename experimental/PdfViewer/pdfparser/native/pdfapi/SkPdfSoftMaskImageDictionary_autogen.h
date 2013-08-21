/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPdfSoftMaskImageDictionary_DEFINED
#define SkPdfSoftMaskImageDictionary_DEFINED

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

#endif  // SkPdfSoftMaskImageDictionary_DEFINED
