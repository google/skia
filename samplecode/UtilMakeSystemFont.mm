#include "include/core/SkTypeface.h"
#include "include/core/SkRefCnt.h"
#include "include/ports/SkTypeface_mac.h"

#include <string>
#include <CoreText/CoreText.h>
#include <stdio.h>
#include <AppKit/AppKit.h>

sk_sp<SkTypeface> makeMacSystemFont(float size) {
  NSFont* ns_system_font = [NSFont systemFontOfSize:size weight:0 /* regular */];
  SkASSERT(ns_system_font);
  CTFontRef system_font = (CTFontRef)ns_system_font;
  SkASSERT(system_font);
  sk_sp<SkTypeface> system_font_skia = SkMakeTypefaceFromCTFont(system_font);
  SkString family;
  system_font_skia->getFamilyName(&family);
  // printf("%s\n", family.c_str());
  SkASSERT(family.contains("SF NS") || family.contains("AppleSystemUIFont"));
  return system_font_skia;
}
