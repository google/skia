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


bool opszOverrideEffective(sk_sp<SkTypeface> typeface, SkScalar current_opsz, float new_opsz) {
  SkFontArguments::VariationPosition::Coordinate opsz_coordinate[1] = {{ kOpszTag, new_opsz }};

  SkFontArguments::VariationPosition opsz_position{
    opsz_coordinate, 1 };

  sk_sp<SkTypeface> cloned_probe_typeface(typeface->makeClone(
      SkFontArguments().setVariationDesignPosition(opsz_position)));

 int existing_axes = cloned_probe_typeface->getVariationDesignPosition(nullptr, 0);
 SkASSERT(existing_axes > 0);
 std::vector<SkFontArguments::VariationPosition::Coordinate> coordinates_retrieved;
 coordinates_retrieved.resize(existing_axes);
 SkASSERT(cloned_probe_typeface->getVariationDesignPosition(coordinates_retrieved.data(),
                                               existing_axes) == existing_axes);


 for (auto& coordinate : coordinates_retrieved) {
   if (coordinate.axis == kOpszTag) {
     return coordinate.value != current_opsz;
   }
 }
 return false;
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
     printf("Source typeface has opsz value %f, font size: %f, override_effective: %d, %d, %d\n", coordinate.value, size, opszOverrideEffective(source_typeface, coordinate.value, size), opszOverrideEffective(source_typeface, coordinate.value, 6), opszOverrideEffective(source_typeface, coordinate.value, 20));
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
