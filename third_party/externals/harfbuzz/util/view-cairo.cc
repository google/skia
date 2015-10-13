/*
 * Copyright © 2011  Google, Inc.
 *
 *  This is part of HarfBuzz, a text shaping library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 * Google Author(s): Behdad Esfahbod
 */

#include "view-cairo.hh"

void
view_cairo_t::get_surface_size (cairo_scaled_font_t *scaled_font,
				double *w, double *h)
{
  cairo_font_extents_t font_extents;

  cairo_scaled_font_extents (scaled_font, &font_extents);

  bool vertical = HB_DIRECTION_IS_VERTICAL (direction);
  (vertical ? *w : *h) = (int) lines->len * (font_extents.height + view_options.line_space) - view_options.line_space;
  (vertical ? *h : *w) = 0;
  for (unsigned int i = 0; i < lines->len; i++) {
    helper_cairo_line_t &line = g_array_index (lines, helper_cairo_line_t, i);
    double x_advance, y_advance;
    line.get_advance (&x_advance, &y_advance);
    if (vertical)
      *h =  MAX (*h, y_advance);
    else
      *w =  MAX (*w, x_advance);
  }

  *w += view_options.margin.l + view_options.margin.r;
  *h += view_options.margin.t + view_options.margin.b;
}

void
view_cairo_t::render (const font_options_t *font_opts)
{
  cairo_scaled_font_t *scaled_font = helper_cairo_create_scaled_font (font_opts, view_options.font_size);
  double w, h;
  get_surface_size (scaled_font, &w, &h);
  cairo_t *cr = helper_cairo_create_context (w, h, &view_options, &output_options);
  cairo_set_scaled_font (cr, scaled_font);
  cairo_scaled_font_destroy (scaled_font);

  draw (cr);

  helper_cairo_destroy_context (cr);
}

void
view_cairo_t::draw (cairo_t *cr)
{
  cairo_save (cr);

  bool vertical = HB_DIRECTION_IS_VERTICAL (direction);
  int v = vertical ? 1 : 0;
  int h = vertical ? 0 : 1;
  cairo_font_extents_t font_extents;
  cairo_font_extents (cr, &font_extents);
  cairo_translate (cr, view_options.margin.l, view_options.margin.t);
  double descent;
  if (vertical)
    descent = font_extents.height * (lines->len + .5);
  else
    descent = font_extents.height - font_extents.ascent;
  cairo_translate (cr, v * descent, h * -descent);
  for (unsigned int i = 0; i < lines->len; i++)
  {
    helper_cairo_line_t &l = g_array_index (lines, helper_cairo_line_t, i);

    if (i)
      cairo_translate (cr, v * -view_options.line_space, h * view_options.line_space);

    cairo_translate (cr, v * -font_extents.height, h * font_extents.height);

    if (view_options.annotate) {
      cairo_save (cr);

      /* Draw actual glyph origins */
      cairo_set_source_rgba (cr, 1., 0., 0., .5);
      cairo_set_line_width (cr, 5);
      cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
      for (unsigned i = 0; i < l.num_glyphs; i++) {
	cairo_move_to (cr, l.glyphs[i].x, l.glyphs[i].y);
	cairo_rel_line_to (cr, 0, 0);
      }
      cairo_stroke (cr);

      cairo_restore (cr);
    }

    if (0 && cairo_surface_get_type (cairo_get_target (cr)) == CAIRO_SURFACE_TYPE_IMAGE) {
      /* cairo_show_glyphs() doesn't support subpixel positioning */
      cairo_glyph_path (cr, l.glyphs, l.num_glyphs);
      cairo_fill (cr);
    } else if (l.num_clusters)
      cairo_show_text_glyphs (cr,
			      l.utf8, l.utf8_len,
			      l.glyphs, l.num_glyphs,
			      l.clusters, l.num_clusters,
			      l.cluster_flags);
    else
      cairo_show_glyphs (cr, l.glyphs, l.num_glyphs);
  }

  cairo_restore (cr);
}
