/*
 * Copyright © 2010  Behdad Esfahbod
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

#include "main-font-text.hh"
#include "shape-consumer.hh"

struct output_buffer_t
{
  output_buffer_t (option_parser_t *parser)
		  : options (parser, hb_buffer_serialize_list_formats ()),
		    format (parser),
		    gs (NULL),
		    line_no (0),
		    font (NULL) {}

  void init (const font_options_t *font_opts)
  {
    options.get_file_handle ();
    gs = g_string_new (NULL);
    line_no = 0;
    font = hb_font_reference (font_opts->get_font ());

    if (!options.output_format)
      output_format = HB_BUFFER_SERIALIZE_FORMAT_TEXT;
    else
      output_format = hb_buffer_serialize_format_from_string (options.output_format, -1);
    /* An empty "output_format" parameter basically skips output generating.
     * Useful for benchmarking. */
    if ((!options.output_format || *options.output_format) &&
	!hb_buffer_serialize_format_to_string (output_format))
    {
      if (options.explicit_output_format)
	fail (false, "Unknown output format `%s'; supported formats are: %s",
	      options.output_format,
	      g_strjoinv ("/", const_cast<char**> (options.supported_formats)));
      else
	/* Just default to TEXT if not explicitly requested and the
	 * file extension is not recognized. */
	output_format = HB_BUFFER_SERIALIZE_FORMAT_TEXT;
    }

    unsigned int flags = HB_BUFFER_SERIALIZE_FLAG_DEFAULT;
    if (!format.show_glyph_names)
      flags |= HB_BUFFER_SERIALIZE_FLAG_NO_GLYPH_NAMES;
    if (!format.show_clusters)
      flags |= HB_BUFFER_SERIALIZE_FLAG_NO_CLUSTERS;
    if (!format.show_positions)
      flags |= HB_BUFFER_SERIALIZE_FLAG_NO_POSITIONS;
    format_flags = (hb_buffer_serialize_flags_t) flags;
  }
  void new_line (void)
  {
    line_no++;
  }
  void consume_text (hb_buffer_t  *buffer,
		     const char   *text,
		     unsigned int  text_len,
		     hb_bool_t     utf8_clusters)
  {
    g_string_set_size (gs, 0);
    format.serialize_buffer_of_text (buffer, line_no, text, text_len, font, gs);
    fprintf (options.fp, "%s", gs->str);
  }
  void shape_failed (hb_buffer_t  *buffer,
		     const char   *text,
		     unsigned int  text_len,
		     hb_bool_t     utf8_clusters)
  {
    g_string_set_size (gs, 0);
    format.serialize_message (line_no, "msg: all shapers failed", gs);
    fprintf (options.fp, "%s", gs->str);
  }
  void consume_glyphs (hb_buffer_t  *buffer,
		       const char   *text,
		       unsigned int  text_len,
		       hb_bool_t     utf8_clusters)
  {
    g_string_set_size (gs, 0);
    format.serialize_buffer_of_glyphs (buffer, line_no, text, text_len, font,
				       output_format, format_flags, gs);
    fprintf (options.fp, "%s", gs->str);
  }
  void finish (const font_options_t *font_opts)
  {
    hb_font_destroy (font);
    g_string_free (gs, true);
    gs = NULL;
    font = NULL;
  }

  protected:
  output_options_t options;
  format_options_t format;

  GString *gs;
  unsigned int line_no;
  hb_font_t *font;
  hb_buffer_serialize_format_t output_format;
  hb_buffer_serialize_flags_t format_flags;
};

int
main (int argc, char **argv)
{
  main_font_text_t<shape_consumer_t<output_buffer_t> > driver;
  return driver.main (argc, argv);
}
