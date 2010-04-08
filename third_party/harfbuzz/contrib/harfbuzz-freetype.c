#include <stdint.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H

#if 0
#include <freetype/freetype.h>
#include <freetype/tttables.h>
#endif

#include <harfbuzz-shaper.h>
#include "harfbuzz-unicode.h"

static HB_Bool
hb_freetype_string_to_glyphs(HB_Font font,
                             const HB_UChar16 *chars, hb_uint32 len,
                             HB_Glyph *glyphs, hb_uint32 *numGlyphs,
                             HB_Bool is_rtl) {
  FT_Face face = (FT_Face) font->userData;
  if (len > *numGlyphs)
    return 0;

  size_t i = 0, j = 0;
  while (i < len) {
    const uint32_t cp = utf16_to_code_point(chars, len, &i);
    glyphs[j++] = FT_Get_Char_Index(face, cp);
  }

  *numGlyphs = j;

  return 1;
}

static void
hb_freetype_advances_get(HB_Font font, const HB_Glyph *glyphs, hb_uint32 len,
                         HB_Fixed *advances, int flags) {
  FT_Face face = (FT_Face) font->userData;

  hb_uint32 i;
  for (i = 0; i < len; ++i) {
    const FT_Error error = FT_Load_Glyph(face, glyphs[i], FT_LOAD_DEFAULT);
    if (error) {
      advances[i] = 0;
      continue;
    }

    advances[i] = face->glyph->advance.x;
  }
}

static HB_Bool
hb_freetype_can_render(HB_Font font, const HB_UChar16 *chars, hb_uint32 len) {
  FT_Face face = (FT_Face)font->userData;

  size_t i = 0;
  while (i < len) {
    const uint32_t cp = utf16_to_code_point(chars, len, &i);
    if (FT_Get_Char_Index(face, cp) == 0)
      return 0;
  }

  return 1;
}

static HB_Error
hb_freetype_outline_point_get(HB_Font font, HB_Glyph glyph, int flags,
                              hb_uint32 point, HB_Fixed *xpos, HB_Fixed *ypos,
                              hb_uint32 *n_points) {
  HB_Error error = HB_Err_Ok;
  FT_Face face = (FT_Face) font->userData;

  int load_flags = (flags & HB_ShaperFlag_UseDesignMetrics) ? FT_LOAD_NO_HINTING : FT_LOAD_DEFAULT;

  if ((error = (HB_Error) FT_Load_Glyph(face, glyph, load_flags)))
    return error;

  if (face->glyph->format != ft_glyph_format_outline)
    return (HB_Error)HB_Err_Invalid_SubTable;

  *n_points = face->glyph->outline.n_points;
  if (!(*n_points))
    return HB_Err_Ok;

  if (point > *n_points)
    return (HB_Error)HB_Err_Invalid_SubTable;

  *xpos = face->glyph->outline.points[point].x;
  *ypos = face->glyph->outline.points[point].y;

  return HB_Err_Ok;
}

static void
hb_freetype_glyph_metrics_get(HB_Font font, HB_Glyph glyph,
                              HB_GlyphMetrics *metrics) {
  FT_Face face = (FT_Face) font->userData;

  const FT_Error error = FT_Load_Glyph(face, glyph, FT_LOAD_DEFAULT);
  if (error) {
    metrics->x = metrics->y = metrics->width = metrics->height = 0;
    metrics->xOffset = metrics->yOffset = 0;
    return;
  }

  const FT_Glyph_Metrics *ftmetrics = &face->glyph->metrics;
  metrics->width = ftmetrics->width;
  metrics->height = ftmetrics->height;
  metrics->x = ftmetrics->horiAdvance;
  metrics->y = 0;  // unclear what this is
  metrics->xOffset = ftmetrics->horiBearingX;
  metrics->yOffset = ftmetrics->horiBearingY;
}

static HB_Fixed
hb_freetype_font_metric_get(HB_Font font, HB_FontMetric metric) {
  FT_Face face = (FT_Face) font->userData;

  switch (metric) {
  case HB_FontAscent:
    // Note that we aren't scanning the VDMX table which we probably would in
    // an ideal world.
    return face->ascender;
  default:
    return 0;
  }
}

const HB_FontClass hb_freetype_class = {
  hb_freetype_string_to_glyphs,
  hb_freetype_advances_get,
  hb_freetype_can_render,
  hb_freetype_outline_point_get,
  hb_freetype_glyph_metrics_get,
  hb_freetype_font_metric_get,
};

HB_Error
hb_freetype_table_sfnt_get(void *voidface, const HB_Tag tag, HB_Byte *buffer, HB_UInt *len) {
  FT_Face face = (FT_Face) voidface;
  FT_ULong ftlen = *len;

  if (!FT_IS_SFNT(face))
    return HB_Err_Invalid_Argument;

  const FT_Error error = FT_Load_Sfnt_Table(face, tag, 0, buffer, &ftlen);
  *len = ftlen;
  return (HB_Error) error;
}
