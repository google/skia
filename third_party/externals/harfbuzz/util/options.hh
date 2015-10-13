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

#ifndef OPTIONS_HH
#define OPTIONS_HH


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <locale.h>
#include <errno.h>
#include <fcntl.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h> /* for isatty() */
#endif
#if defined(_WIN32) || defined(__CYGWIN__)
#include <io.h> /* for setmode() under Windows */
#endif

#include <hb.h>
#ifdef HAVE_OT
#include <hb-ot.h>
#endif
#include <glib.h>
#include <glib/gprintf.h>

#if !GLIB_CHECK_VERSION (2, 22, 0)
# define g_mapped_file_unref g_mapped_file_free
#endif


/* A few macros copied from hb-private.hh. */

#if __GNUC__ >= 4
#define HB_UNUSED	__attribute__((unused))
#else
#define HB_UNUSED
#endif

#undef MIN
template <typename Type> static inline Type MIN (const Type &a, const Type &b) { return a < b ? a : b; }

#undef MAX
template <typename Type> static inline Type MAX (const Type &a, const Type &b) { return a > b ? a : b; }

#undef  ARRAY_LENGTH
template <typename Type, unsigned int n>
static inline unsigned int ARRAY_LENGTH (const Type (&)[n]) { return n; }
/* A const version, but does not detect erratically being called on pointers. */
#define ARRAY_LENGTH_CONST(__array) ((signed int) (sizeof (__array) / sizeof (__array[0])))

#define _ASSERT_STATIC1(_line, _cond)	HB_UNUSED typedef int _static_assert_on_line_##_line##_failed[(_cond)?1:-1]
#define _ASSERT_STATIC0(_line, _cond)	_ASSERT_STATIC1 (_line, (_cond))
#define ASSERT_STATIC(_cond)		_ASSERT_STATIC0 (__LINE__, (_cond))


void fail (hb_bool_t suggest_help, const char *format, ...) G_GNUC_NORETURN G_GNUC_PRINTF (2, 3);


extern hb_bool_t debug;

struct option_group_t
{
  virtual void add_options (struct option_parser_t *parser) = 0;

  virtual void pre_parse (GError **error G_GNUC_UNUSED) {};
  virtual void post_parse (GError **error G_GNUC_UNUSED) {};
};


struct option_parser_t
{
  option_parser_t (const char *usage) {
    memset (this, 0, sizeof (*this));
    usage_str = usage;
    context = g_option_context_new (usage);
    to_free = g_ptr_array_new ();

    add_main_options ();
  }
  ~option_parser_t (void) {
    g_option_context_free (context);
    g_ptr_array_foreach (to_free, (GFunc) g_free, NULL);
    g_ptr_array_free (to_free, TRUE);
  }

  void add_main_options (void);

  void add_group (GOptionEntry   *entries,
		  const gchar    *name,
		  const gchar    *description,
		  const gchar    *help_description,
		  option_group_t *option_group);

  void free_later (char *p) {
    g_ptr_array_add (to_free, p);
  }

  void parse (int *argc, char ***argv);

  G_GNUC_NORETURN void usage (void) {
    g_printerr ("Usage: %s [OPTION...] %s\n", g_get_prgname (), usage_str);
    exit (1);
  }

  private:
  const char *usage_str;
  GOptionContext *context;
  GPtrArray *to_free;
};


#define DEFAULT_MARGIN 16
#define DEFAULT_FORE "#000000"
#define DEFAULT_BACK "#FFFFFF"
#define DEFAULT_FONT_SIZE 256

struct view_options_t : option_group_t
{
  view_options_t (option_parser_t *parser) {
    annotate = false;
    fore = DEFAULT_FORE;
    back = DEFAULT_BACK;
    line_space = 0;
    margin.t = margin.r = margin.b = margin.l = DEFAULT_MARGIN;
    font_size = DEFAULT_FONT_SIZE;

    add_options (parser);
  }

  void add_options (option_parser_t *parser);

  hb_bool_t annotate;
  const char *fore;
  const char *back;
  double line_space;
  struct margin_t {
    double t, r, b, l;
  } margin;
  double font_size;
};


struct shape_options_t : option_group_t
{
  shape_options_t (option_parser_t *parser)
  {
    direction = language = script = NULL;
    bot = eot = preserve_default_ignorables = false;
    features = NULL;
    num_features = 0;
    shapers = NULL;
    utf8_clusters = false;
    normalize_glyphs = false;
    num_iterations = 1;

    add_options (parser);
  }
  ~shape_options_t (void)
  {
    free (features);
    g_strfreev (shapers);
  }

  void add_options (option_parser_t *parser);

  void setup_buffer (hb_buffer_t *buffer)
  {
    hb_buffer_set_direction (buffer, hb_direction_from_string (direction, -1));
    hb_buffer_set_script (buffer, hb_script_from_string (script, -1));
    hb_buffer_set_language (buffer, hb_language_from_string (language, -1));
    hb_buffer_set_flags (buffer, (hb_buffer_flags_t) (HB_BUFFER_FLAG_DEFAULT |
			 (bot ? HB_BUFFER_FLAG_BOT : 0) |
			 (eot ? HB_BUFFER_FLAG_EOT : 0) |
			 (preserve_default_ignorables ? HB_BUFFER_FLAG_PRESERVE_DEFAULT_IGNORABLES : 0)));
    hb_buffer_guess_segment_properties (buffer);
  }

  void populate_buffer (hb_buffer_t *buffer, const char *text, int text_len,
			const char *text_before, const char *text_after)
  {
    hb_buffer_clear_contents (buffer);
    if (text_before) {
      unsigned int len = strlen (text_before);
      hb_buffer_add_utf8 (buffer, text_before, len, len, 0);
    }
    hb_buffer_add_utf8 (buffer, text, text_len, 0, text_len);
    if (text_after) {
      hb_buffer_add_utf8 (buffer, text_after, -1, 0, 0);
    }

    if (!utf8_clusters) {
      /* Reset cluster values to refer to Unicode character index
       * instead of UTF-8 index. */
      unsigned int num_glyphs = hb_buffer_get_length (buffer);
      hb_glyph_info_t *info = hb_buffer_get_glyph_infos (buffer, NULL);
      for (unsigned int i = 0; i < num_glyphs; i++)
      {
	info->cluster = i;
	info++;
      }
    }

    setup_buffer (buffer);
  }

  hb_bool_t shape (hb_font_t *font, hb_buffer_t *buffer)
  {
    hb_bool_t res = hb_shape_full (font, buffer, features, num_features, shapers);
    if (normalize_glyphs)
      hb_buffer_normalize_glyphs (buffer);
    return res;
  }

  void shape_closure (const char *text, int text_len,
		      hb_font_t *font, hb_buffer_t *buffer,
		      hb_set_t *glyphs)
  {
    hb_buffer_reset (buffer);
    hb_buffer_add_utf8 (buffer, text, text_len, 0, text_len);
    setup_buffer (buffer);
    hb_ot_shape_glyphs_closure (font, buffer, features, num_features, glyphs);
  }

  /* Buffer properties */
  const char *direction;
  const char *language;
  const char *script;

  /* Buffer flags */
  hb_bool_t bot;
  hb_bool_t eot;
  hb_bool_t preserve_default_ignorables;

  hb_feature_t *features;
  unsigned int num_features;
  char **shapers;
  hb_bool_t utf8_clusters;
  hb_bool_t normalize_glyphs;
  unsigned int num_iterations;
};


struct font_options_t : option_group_t
{
  font_options_t (option_parser_t *parser) {
    font_file = NULL;
    face_index = 0;
    font_funcs = NULL;

    font = NULL;

    add_options (parser);
  }
  ~font_options_t (void) {
    hb_font_destroy (font);
  }

  void add_options (option_parser_t *parser);

  hb_font_t *get_font (void) const;

  const char *font_file;
  int face_index;
  const char *font_funcs;

  private:
  mutable hb_font_t *font;
};


struct text_options_t : option_group_t
{
  text_options_t (option_parser_t *parser) {
    text_before = NULL;
    text_after = NULL;

    text = NULL;
    text_file = NULL;

    fp = NULL;
    gs = NULL;
    text_len = (unsigned int) -1;

    add_options (parser);
  }
  ~text_options_t (void) {
    if (gs)
      g_string_free (gs, true);
    if (fp)
      fclose (fp);
  }

  void add_options (option_parser_t *parser);

  void post_parse (GError **error G_GNUC_UNUSED) {
    if (text && text_file)
      g_set_error (error,
		   G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE,
		   "Only one of text and text-file can be set");

  };

  const char *get_line (unsigned int *len);

  const char *text_before;
  const char *text_after;

  const char *text;
  const char *text_file;

  private:
  FILE *fp;
  GString *gs;
  unsigned int text_len;
};

struct output_options_t : option_group_t
{
  output_options_t (option_parser_t *parser,
		    const char **supported_formats_ = NULL) {
    output_file = NULL;
    output_format = NULL;
    supported_formats = supported_formats_;
    explicit_output_format = false;

    fp = NULL;

    add_options (parser);
  }
  ~output_options_t (void) {
    if (fp)
      fclose (fp);
  }

  void add_options (option_parser_t *parser);

  void post_parse (GError **error G_GNUC_UNUSED)
  {
    if (output_format)
      explicit_output_format = true;

    if (output_file && !output_format) {
      output_format = strrchr (output_file, '.');
      if (output_format)
	  output_format++; /* skip the dot */
    }

    if (output_file && 0 == strcmp (output_file, "-"))
      output_file = NULL; /* STDOUT */
  }

  FILE *get_file_handle (void);

  const char *output_file;
  const char *output_format;
  const char **supported_formats;
  bool explicit_output_format;

  mutable FILE *fp;
};

struct format_options_t : option_group_t
{
  format_options_t (option_parser_t *parser) {
    show_glyph_names = true;
    show_positions = true;
    show_clusters = true;
    show_text = false;
    show_unicode = false;
    show_line_num = false;

    add_options (parser);
  }

  void add_options (option_parser_t *parser);

  void serialize_unicode (hb_buffer_t  *buffer,
			  GString      *gs);
  void serialize_glyphs (hb_buffer_t  *buffer,
			 hb_font_t    *font,
			 hb_buffer_serialize_format_t format,
			 hb_buffer_serialize_flags_t flags,
			 GString      *gs);
  void serialize_line_no (unsigned int  line_no,
			  GString      *gs);
  void serialize_buffer_of_text (hb_buffer_t  *buffer,
				 unsigned int  line_no,
				 const char   *text,
				 unsigned int  text_len,
				 hb_font_t    *font,
				 GString      *gs);
  void serialize_message (unsigned int  line_no,
			  const char   *msg,
			  GString      *gs);
  void serialize_buffer_of_glyphs (hb_buffer_t  *buffer,
				   unsigned int  line_no,
				   const char   *text,
				   unsigned int  text_len,
				   hb_font_t    *font,
				   hb_buffer_serialize_format_t output_format,
				   hb_buffer_serialize_flags_t format_flags,
				   GString      *gs);


  hb_bool_t show_glyph_names;
  hb_bool_t show_positions;
  hb_bool_t show_clusters;
  hb_bool_t show_text;
  hb_bool_t show_unicode;
  hb_bool_t show_line_num;
};


#endif
