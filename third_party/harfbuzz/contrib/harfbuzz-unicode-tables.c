#include <stdlib.h>
#include <stdint.h>

#include <harfbuzz-external.h>

#include "tables/category-properties.h"
#include "tables/combining-properties.h"

HB_LineBreakClass
HB_GetLineBreakClass(HB_UChar32 ch) {
  abort();
  return 0;
}

static int
combining_property_cmp(const void *vkey, const void *vcandidate) {
  const uint32_t key = (uint32_t) (intptr_t) vkey;
  const struct combining_property *candidate = vcandidate;

  if (key < candidate->range_start) {
    return -1;
  } else if (key > candidate->range_end) {
    return 1;
  } else {
    return 0;
  }
}

static int
code_point_to_combining_class(HB_UChar32 cp) {
  const void *vprop = bsearch((void *) (intptr_t) cp, combining_properties,
                              combining_properties_count,
                              sizeof(struct combining_property),
                              combining_property_cmp);
  if (!vprop)
    return 0;

  return ((const struct combining_property *) vprop)->klass;
}

int
HB_GetUnicodeCharCombiningClass(HB_UChar32 ch) {
  return code_point_to_combining_class(ch);
  return 0;
}

static int
category_property_cmp(const void *vkey, const void *vcandidate) {
  const uint32_t key = (uint32_t) (intptr_t) vkey;
  const struct category_property *candidate = vcandidate;

  if (key < candidate->range_start) {
    return -1;
  } else if (key > candidate->range_end) {
    return 1;
  } else {
    return 0;
  }
}

static HB_CharCategory
code_point_to_category(HB_UChar32 cp) {
  const void *vprop = bsearch((void *) (intptr_t) cp, category_properties,
                              category_properties_count,
                              sizeof(struct category_property),
                              category_property_cmp);
  if (!vprop)
    return HB_NoCategory;

  return ((const struct category_property *) vprop)->category;
}

void
HB_GetUnicodeCharProperties(HB_UChar32 ch,
                            HB_CharCategory *category,
                            int *combiningClass) {
  *category = code_point_to_category(ch);
  *combiningClass = code_point_to_combining_class(ch);
}

HB_CharCategory
HB_GetUnicodeCharCategory(HB_UChar32 ch) {
  return code_point_to_category(ch);
}
