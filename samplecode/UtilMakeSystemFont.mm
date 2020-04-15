#include "include/core/SkTypeface.h"
#include "include/core/SkRefCnt.h"
#include "include/ports/SkTypeface_mac.h"

#include <string>
#include <CoreText/CoreText.h>
#include <stdio.h>
#include <AppKit/AppKit.h>
#include <vector>

namespace {
  constexpr SkFourByteTag kOpszTag = SkSetFourByteTag('o', 'p', 's', 'z');
  constexpr SkScalar kOpszCycleTest = 20;
}


sk_sp<SkTypeface> cycleThroughMakeClone(float size, sk_sp<SkTypeface> source_typeface) {

 int existing_axes = source_typeface->getVariationDesignPosition(nullptr, 0);
 SkASSERT(existing_axes > 0);
 std::vector<SkFontArguments::VariationPosition::Coordinate> coordinates_to_set;
 coordinates_to_set.resize(existing_axes);
 SkASSERT(source_typeface->getVariationDesignPosition(coordinates_to_set.data(),
                                               existing_axes) == existing_axes);

 for (auto& coordinate : coordinates_to_set) {
   if (coordinate.axis == kOpszTag) {
     coordinate.value = kOpszCycleTest;
   }
 }

 SkFontArguments::VariationPosition variation_design_position{
   coordinates_to_set.data(), static_cast<int>(coordinates_to_set.size())};
 sk_sp<SkTypeface> cloned_typeface = source_typeface->makeClone(SkFontArguments().setVariationDesignPosition(variation_design_position));
 SkASSERT(cloned_typeface);

 for (auto& coordinate : coordinates_to_set) {
   if (coordinate.axis == kOpszTag) {
     coordinate.value = size;
   }
 }

 SkFontArguments::VariationPosition variation_design_position_original{
   coordinates_to_set.data(), static_cast<int>(coordinates_to_set.size())};
 sk_sp<SkTypeface> cycled_typeface = cloned_typeface->makeClone(SkFontArguments().setVariationDesignPosition(variation_design_position_original));
 SkASSERT(cycled_typeface);

 return cycled_typeface;
}


sk_sp<SkTypeface> makeMacSystemFont(float size, bool cycle_through_make_clone) {
  NSFont* ns_system_font = [NSFont systemFontOfSize:size weight:0 /* regular */];
  SkASSERT(ns_system_font);
  CTFontRef system_font = (CTFontRef)ns_system_font;
  SkASSERT(system_font);
  sk_sp<SkTypeface> system_font_skia = SkMakeTypefaceFromCTFont(system_font);
  SkString family;
  system_font_skia->getFamilyName(&family);
  SkASSERT(family.contains("SF NS") || family.contains("AppleSystemUIFont"));

  if (!cycle_through_make_clone)
    return system_font_skia;
  else
    return cycleThroughMakeClone(size, system_font_skia);
}
