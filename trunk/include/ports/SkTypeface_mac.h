
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef SkTypeface_mac_DEFINED
#define SkTypeface_mac_DEFINED

#include "SkTypeface.h"
#ifdef SK_BUILD_FOR_MAC
#import <ApplicationServices/ApplicationServices.h>
#endif

#ifdef SK_BUILD_FOR_IOS
#include <CoreText/CoreText.h>
#endif
/**
 *  Like the other Typeface create methods, this returns a new reference to the
 *  corresponding typeface for the specified CTFontRef. The caller must call
 *  unref() when it is finished.
 */
SK_API extern SkTypeface* SkCreateTypefaceFromCTFont(CTFontRef);

#endif

