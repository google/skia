#include <stdint.h>
#include <stdlib.h>

#include <harfbuzz-external.h>
#include <harfbuzz-impl.h>
#include <harfbuzz-shaper.h>
#include "harfbuzz-unicode.h"

#include "tables/grapheme-break-properties.h"
#include "tables/mirroring-properties.h"
#include "tables/script-properties.h"

uint32_t
utf16_to_code_point(const uint16_t *chars, size_t len, ssize_t *iter) {
  const uint16_t v = chars[(*iter)++];
  if (HB_IsHighSurrogate(v)) {
    // surrogate pair
    if (*iter >= len) {
      // the surrogate is incomplete.
      return HB_InvalidCodePoint;
    }
    const uint16_t v2 = chars[(*iter)++];
    if (!HB_IsLowSurrogate(v2)) {
      // invalidate surrogate pair.
      return HB_InvalidCodePoint;
    }

    return HB_SurrogateToUcs4(v, v2);
  }

  if (HB_IsLowSurrogate(v)) {
    // this isn't a valid code point
    return HB_InvalidCodePoint;
  }

  return v;
}

uint32_t
utf16_to_code_point_prev(const uint16_t *chars, size_t len, ssize_t *iter) {
  const uint16_t v = chars[(*iter)--];
  if (HB_IsLowSurrogate(v)) {
    // surrogate pair
    if (*iter < 0) {
      // the surrogate is incomplete.
      return HB_InvalidCodePoint;
    }
    const uint16_t v2 = chars[(*iter)--];
    if (!HB_IsHighSurrogate(v2)) {
      // invalidate surrogate pair.
      return HB_InvalidCodePoint;
    }

    return HB_SurrogateToUcs4(v2, v);
  }

  if (HB_IsHighSurrogate(v)) {
    // this isn't a valid code point
    return HB_InvalidCodePoint;
  }

  return v;
}

static int
script_property_cmp(const void *vkey, const void *vcandidate) {
  const uint32_t key = (uint32_t) (intptr_t) vkey;
  const struct script_property *candidate = vcandidate;

  if (key < candidate->range_start) {
    return -1;
  } else if (key > candidate->range_end) {
    return 1;
  } else {
    return 0;
  }
}

HB_Script
code_point_to_script(uint32_t cp) {
  const void *vprop = bsearch((void *) (intptr_t) cp, script_properties,
                              script_properties_count,
                              sizeof(struct script_property),
                              script_property_cmp);
  if (!vprop)
    return HB_Script_Common;

  return ((const struct script_property *) vprop)->script;
}

char
hb_utf16_script_run_next(unsigned *num_code_points, HB_ScriptItem *output,
                         const uint16_t *chars, size_t len, ssize_t *iter) {
  if (*iter == len)
    return 0;

  output->pos = *iter;
  const uint32_t init_cp = utf16_to_code_point(chars, len, iter);
  unsigned cps = 1;
  if (init_cp == HB_InvalidCodePoint)
    return 0;
  const HB_Script init_script = code_point_to_script(init_cp);
  HB_Script current_script = init_script;
  output->script = init_script;

  for (;;) {
    if (*iter == len)
      break;
    const ssize_t prev_iter = *iter;
    const uint32_t cp = utf16_to_code_point(chars, len, iter);
    if (cp == HB_InvalidCodePoint)
      return 0;
    cps++;
    const HB_Script script = code_point_to_script(cp);

    if (script != current_script) {
      if (current_script == init_script == HB_Script_Inherited) {
        // If we started off as inherited, we take whatever we can find.
        output->script = script;
        current_script = script;
        continue;
      } else if (script == HB_Script_Inherited) {
        continue;
      } else {
        *iter = prev_iter;
        cps--;
        break;
      }
    }
  }

  if (output->script == HB_Script_Inherited)
    output->script = HB_Script_Common;

  output->length = *iter - output->pos;
  if (num_code_points)
    *num_code_points = cps;
  return 1;
}

char
hb_utf16_script_run_prev(unsigned *num_code_points, HB_ScriptItem *output,
                         const uint16_t *chars, size_t len, ssize_t *iter) {
  if (*iter == (size_t) -1)
    return 0;

  const size_t ending_index = *iter;
  const uint32_t init_cp = utf16_to_code_point_prev(chars, len, iter);
  unsigned cps = 1;
  if (init_cp == HB_InvalidCodePoint)
    return 0;
  const HB_Script init_script = code_point_to_script(init_cp);
  HB_Script current_script = init_script;
  output->script = init_script;

  for (;;) {
    if (*iter < 0)
      break;
    const ssize_t prev_iter = *iter;
    const uint32_t cp = utf16_to_code_point_prev(chars, len, iter);
    if (cp == HB_InvalidCodePoint)
      return 0;
    cps++;
    const HB_Script script = code_point_to_script(cp);

    if (script != current_script) {
      if (current_script == init_script == HB_Script_Inherited) {
        // If we started off as inherited, we take whatever we can find.
        output->script = script;
        current_script = script;
        continue;
      } else if (script == HB_Script_Inherited) {
        // Just assume that whatever follows this combining character is within
        // the same script.  This is incorrect if you had language1 + combining
        // char + language 2, but that is rare and this code is suspicious
        // anyway.
        continue;
      } else {
        *iter = prev_iter;
        cps--;
        break;
      }
    }
  }

  if (output->script == HB_Script_Inherited)
    output->script = HB_Script_Common;

  output->pos = *iter + 1;
  output->length = ending_index - *iter;
  if (num_code_points)
    *num_code_points = cps;
  return 1;
}

static int
grapheme_break_property_cmp(const void *vkey, const void *vcandidate) {
  const uint32_t key = (uint32_t) (intptr_t) vkey;
  const struct grapheme_break_property *candidate = vcandidate;

  if (key < candidate->range_start) {
    return -1;
  } else if (key > candidate->range_end) {
    return 1;
  } else {
    return 0;
  }
}

HB_GraphemeClass
HB_GetGraphemeClass(HB_UChar32 ch) {
  const void *vprop = bsearch((void *) (intptr_t) ch, grapheme_break_properties,
                              grapheme_break_properties_count,
                              sizeof(struct grapheme_break_property),
                              grapheme_break_property_cmp);
  if (!vprop)
    return HB_Grapheme_Other;

  return ((const struct grapheme_break_property *) vprop)->klass;
}

HB_WordClass
HB_GetWordClass(HB_UChar32 ch) {
  abort();
  return 0;
}

HB_SentenceClass
HB_GetSentenceClass(HB_UChar32 ch) {
  abort();
  return 0;
}

void
HB_GetGraphemeAndLineBreakClass(HB_UChar32 ch, HB_GraphemeClass *gclass, HB_LineBreakClass *breakclass) {
  *gclass = HB_GetGraphemeClass(ch);
  *breakclass = HB_GetLineBreakClass(ch);
}

static int
mirroring_property_cmp(const void *vkey, const void *vcandidate) {
  const uint32_t key = (uint32_t) (intptr_t) vkey;
  const struct mirroring_property *candidate = vcandidate;

  if (key < candidate->a) {
    return -1;
  } else if (key > candidate->a) {
    return 1;
  } else {
    return 0;
  }
}

HB_UChar16
HB_GetMirroredChar(HB_UChar16 ch) {
  const void *mprop = bsearch((void *) (intptr_t) ch, mirroring_properties,
                              mirroring_properties_count,
                              sizeof(struct mirroring_property),
                              mirroring_property_cmp);
  if (!mprop)
    return ch;

  return ((const struct mirroring_property *) mprop)->b;
}

void *
HB_Library_Resolve(const char *library, const char *symbol) {
  abort();
  return NULL;
}

void *
HB_TextCodecForMib(int mib) {
  abort();
  return NULL;
}

char *
HB_TextCodec_ConvertFromUnicode(void *codec, const HB_UChar16 *unicode, hb_uint32 length, hb_uint32 *outputLength) {
  abort();
  return NULL;
}

void
HB_TextCodec_FreeResult(char *v) {
  abort();
}
