/*
 * Copyright © 2011,2012  Google, Inc.
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

#include "options.hh"

#ifndef HB_SHAPE_CONSUMER_HH
#define HB_SHAPE_CONSUMER_HH


template <typename output_t>
struct shape_consumer_t
{
  shape_consumer_t (option_parser_t *parser)
		  : failed (false),
		    shaper (parser),
		    output (parser),
		    font (NULL) {}

  void init (const font_options_t *font_opts)
  {
    font = hb_font_reference (font_opts->get_font ());
    output.init (font_opts);
    failed = false;
  }
  void consume_line (hb_buffer_t  *buffer,
		     const char   *text,
		     unsigned int  text_len,
		     const char   *text_before,
		     const char   *text_after)
  {
    output.new_line ();

    for (unsigned int n = shaper.num_iterations; n; n--)
    {
      shaper.populate_buffer (buffer, text, text_len, text_before, text_after);
      if (n == 1)
	output.consume_text (buffer, text, text_len, shaper.utf8_clusters);
      if (!shaper.shape (font, buffer)) {
	failed = true;
	hb_buffer_set_length (buffer, 0);
	output.shape_failed (buffer, text, text_len, shaper.utf8_clusters);
	return;
      }
    }

    output.consume_glyphs (buffer, text, text_len, shaper.utf8_clusters);
  }
  void finish (const font_options_t *font_opts)
  {
    output.finish (font_opts);
    hb_font_destroy (font);
    font = NULL;
  }

  public:
  bool failed;

  protected:
  shape_options_t shaper;
  output_t output;

  hb_font_t *font;
};


#endif
