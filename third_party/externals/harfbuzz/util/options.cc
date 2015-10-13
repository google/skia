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

#ifdef HAVE_FREETYPE
#include <hb-ft.h>
#endif
#ifdef HAVE_OT
#include <hb-ot-font.h>
#endif

struct supported_font_funcs_t {
	char name[4];
	void (*func) (hb_font_t *);
} supported_font_funcs[] =
{
#ifdef HAVE_FREETYPE
  {"ft",	hb_ft_font_set_funcs},
#endif
#ifdef HAVE_OT
  {"ot",	hb_ot_font_set_funcs},
#endif
};


void
fail (hb_bool_t suggest_help, const char *format, ...)
{
  const char *msg;

  va_list vap;
  va_start (vap, format);
  msg = g_strdup_vprintf (format, vap);
  const char *prgname = g_get_prgname ();
  g_printerr ("%s: %s\n", prgname, msg);
  if (suggest_help)
    g_printerr ("Try `%s --help' for more information.\n", prgname);

  exit (1);
}


hb_bool_t debug = false;

static gchar *
shapers_to_string (void)
{
  GString *shapers = g_string_new (NULL);
  const char **shaper_list = hb_shape_list_shapers ();

  for (; *shaper_list; shaper_list++) {
    g_string_append (shapers, *shaper_list);
    g_string_append_c (shapers, ',');
  }
  g_string_truncate (shapers, MAX (0, (gint)shapers->len - 1));

  return g_string_free (shapers, false);
}

static G_GNUC_NORETURN gboolean
show_version (const char *name G_GNUC_UNUSED,
	      const char *arg G_GNUC_UNUSED,
	      gpointer    data G_GNUC_UNUSED,
	      GError    **error G_GNUC_UNUSED)
{
  g_printf ("%s (%s) %s\n", g_get_prgname (), PACKAGE_NAME, PACKAGE_VERSION);

  char *shapers = shapers_to_string ();
  g_printf ("Available shapers: %s\n", shapers);
  g_free (shapers);
  if (strcmp (HB_VERSION_STRING, hb_version_string ()))
    g_printf ("Linked HarfBuzz library has a different version: %s\n", hb_version_string ());

  exit(0);
}


void
option_parser_t::add_main_options (void)
{
  GOptionEntry entries[] =
  {
    {"version",		0, G_OPTION_FLAG_NO_ARG,
			      G_OPTION_ARG_CALLBACK,	(gpointer) &show_version,	"Show version numbers",			NULL},
    {"debug",		0, 0, G_OPTION_ARG_NONE,	&debug,				"Free all resources before exit",	NULL},
    {NULL}
  };
  g_option_context_add_main_entries (context, entries, NULL);
}

static gboolean
pre_parse (GOptionContext *context G_GNUC_UNUSED,
	   GOptionGroup *group G_GNUC_UNUSED,
	   gpointer data,
	   GError **error)
{
  option_group_t *option_group = (option_group_t *) data;
  option_group->pre_parse (error);
  return *error == NULL;
}

static gboolean
post_parse (GOptionContext *context G_GNUC_UNUSED,
	    GOptionGroup *group G_GNUC_UNUSED,
	    gpointer data,
	    GError **error)
{
  option_group_t *option_group = static_cast<option_group_t *>(data);
  option_group->post_parse (error);
  return *error == NULL;
}

void
option_parser_t::add_group (GOptionEntry   *entries,
			    const gchar    *name,
			    const gchar    *description,
			    const gchar    *help_description,
			    option_group_t *option_group)
{
  GOptionGroup *group = g_option_group_new (name, description, help_description,
					    static_cast<gpointer>(option_group), NULL);
  g_option_group_add_entries (group, entries);
  g_option_group_set_parse_hooks (group, pre_parse, post_parse);
  g_option_context_add_group (context, group);
}

void
option_parser_t::parse (int *argc, char ***argv)
{
  setlocale (LC_ALL, "");

  GError *parse_error = NULL;
  if (!g_option_context_parse (context, argc, argv, &parse_error))
  {
    if (parse_error != NULL) {
      fail (true, "%s", parse_error->message);
      //g_error_free (parse_error);
    } else
      fail (true, "Option parse error");
  }
}


static gboolean
parse_margin (const char *name G_GNUC_UNUSED,
	      const char *arg,
	      gpointer    data,
	      GError    **error G_GNUC_UNUSED)
{
  view_options_t *view_opts = (view_options_t *) data;
  view_options_t::margin_t &m = view_opts->margin;
  switch (sscanf (arg, "%lf %lf %lf %lf", &m.t, &m.r, &m.b, &m.l)) {
    case 1: m.r = m.t;
    case 2: m.b = m.t;
    case 3: m.l = m.r;
    case 4: return true;
    default:
      g_set_error (error, G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE,
		   "%s argument should be one to four space-separated numbers",
		   name);
      return false;
  }
}


static gboolean
parse_shapers (const char *name G_GNUC_UNUSED,
	       const char *arg,
	       gpointer    data,
	       GError    **error G_GNUC_UNUSED)
{
  shape_options_t *shape_opts = (shape_options_t *) data;
  g_strfreev (shape_opts->shapers);
  shape_opts->shapers = g_strsplit (arg, ",", 0);
  return true;
}

static G_GNUC_NORETURN gboolean
list_shapers (const char *name G_GNUC_UNUSED,
	      const char *arg G_GNUC_UNUSED,
	      gpointer    data G_GNUC_UNUSED,
	      GError    **error G_GNUC_UNUSED)
{
  for (const char **shaper = hb_shape_list_shapers (); *shaper; shaper++)
    g_printf ("%s\n", *shaper);

  exit(0);
}


static gboolean
parse_features (const char *name G_GNUC_UNUSED,
	        const char *arg,
	        gpointer    data,
	        GError    **error G_GNUC_UNUSED)
{
  shape_options_t *shape_opts = (shape_options_t *) data;
  char *s = (char *) arg;
  char *p;

  shape_opts->num_features = 0;
  g_free (shape_opts->features);
  shape_opts->features = NULL;

  if (!*s)
    return true;

  /* count the features first, so we can allocate memory */
  p = s;
  do {
    shape_opts->num_features++;
    p = strchr (p, ',');
    if (p)
      p++;
  } while (p);

  shape_opts->features = (hb_feature_t *) calloc (shape_opts->num_features, sizeof (*shape_opts->features));

  /* now do the actual parsing */
  p = s;
  shape_opts->num_features = 0;
  while (p && *p) {
    char *end = strchr (p, ',');
    if (hb_feature_from_string (p, end ? end - p : -1, &shape_opts->features[shape_opts->num_features]))
      shape_opts->num_features++;
    p = end ? end + 1 : NULL;
  }

  return true;
}


void
view_options_t::add_options (option_parser_t *parser)
{
  GOptionEntry entries[] =
  {
    {"annotate",	0, 0, G_OPTION_ARG_NONE,	&this->annotate,		"Annotate output rendering",				NULL},
    {"background",	0, 0, G_OPTION_ARG_STRING,	&this->back,			"Set background color (default: " DEFAULT_BACK ")",	"rrggbb/rrggbbaa"},
    {"foreground",	0, 0, G_OPTION_ARG_STRING,	&this->fore,			"Set foreground color (default: " DEFAULT_FORE ")",	"rrggbb/rrggbbaa"},
    {"line-space",	0, 0, G_OPTION_ARG_DOUBLE,	&this->line_space,		"Set space between lines (default: 0)",			"units"},
    {"margin",		0, 0, G_OPTION_ARG_CALLBACK,	(gpointer) &parse_margin,	"Margin around output (default: " G_STRINGIFY(DEFAULT_MARGIN) ")","one to four numbers"},
    {"font-size",	0, 0, G_OPTION_ARG_DOUBLE,	&this->font_size,		"Font size (default: " G_STRINGIFY(DEFAULT_FONT_SIZE) ")","size"},
    {NULL}
  };
  parser->add_group (entries,
		     "view",
		     "View options:",
		     "Options controlling output rendering",
		     this);
}

void
shape_options_t::add_options (option_parser_t *parser)
{
  GOptionEntry entries[] =
  {
    {"list-shapers",	0, G_OPTION_FLAG_NO_ARG,
			      G_OPTION_ARG_CALLBACK,	(gpointer) &list_shapers,	"List available shapers and quit",	NULL},
    {"shaper",		0, G_OPTION_FLAG_HIDDEN,
			      G_OPTION_ARG_CALLBACK,	(gpointer) &parse_shapers,	"Hidden duplicate of --shapers",	NULL},
    {"shapers",		0, 0, G_OPTION_ARG_CALLBACK,	(gpointer) &parse_shapers,	"Set comma-separated list of shapers to try","list"},
    {"direction",	0, 0, G_OPTION_ARG_STRING,	&this->direction,		"Set text direction (default: auto)",	"ltr/rtl/ttb/btt"},
    {"language",	0, 0, G_OPTION_ARG_STRING,	&this->language,		"Set text language (default: $LANG)",	"langstr"},
    {"script",		0, 0, G_OPTION_ARG_STRING,	&this->script,			"Set text script (default: auto)",	"ISO-15924 tag"},
    {"bot",		0, 0, G_OPTION_ARG_NONE,	&this->bot,			"Treat text as beginning-of-paragraph",	NULL},
    {"eot",		0, 0, G_OPTION_ARG_NONE,	&this->eot,			"Treat text as end-of-paragraph",	NULL},
    {"preserve-default-ignorables",0, 0, G_OPTION_ARG_NONE,	&this->preserve_default_ignorables,	"Preserve Default-Ignorable characters",	NULL},
    {"utf8-clusters",	0, 0, G_OPTION_ARG_NONE,	&this->utf8_clusters,		"Use UTF8 byte indices, not char indices",	NULL},
    {"normalize-glyphs",0, 0, G_OPTION_ARG_NONE,	&this->normalize_glyphs,	"Rearrange glyph clusters in nominal order",	NULL},
    {"num-iterations",	0, 0, G_OPTION_ARG_INT,		&this->num_iterations,		"Run shaper N times (default: 1)",	"N"},
    {NULL}
  };
  parser->add_group (entries,
		     "shape",
		     "Shape options:",
		     "Options controlling the shaping process",
		     this);

  const gchar *features_help = "Comma-separated list of font features\n"
    "\n"
    "    Features can be enabled or disabled, either globally or limited to\n"
    "    specific character ranges.  The format for specifying feature settings\n"
    "    follows.  All valid CSS font-feature-settings values other than 'normal'\n"
    "    and 'inherited' are also accepted, though, not documented below.\n"
    "\n"
    "    The range indices refer to the positions between Unicode characters,\n"
    "    unless the --utf8-clusters is provided, in which case range indices\n"
    "    refer to UTF-8 byte indices. The position before the first character\n"
    "    is always 0.\n"
    "\n"
    "    The format is Python-esque.  Here is how it all works:\n"
    "\n"
    "      Syntax:       Value:    Start:    End:\n"
    "\n"
    "    Setting value:\n"
    "      \"kern\"        1         0         ∞         # Turn feature on\n"
    "      \"+kern\"       1         0         ∞         # Turn feature on\n"
    "      \"-kern\"       0         0         ∞         # Turn feature off\n"
    "      \"kern=0\"      0         0         ∞         # Turn feature off\n"
    "      \"kern=1\"      1         0         ∞         # Turn feature on\n"
    "      \"aalt=2\"      2         0         ∞         # Choose 2nd alternate\n"
    "\n"
    "    Setting index:\n"
    "      \"kern[]\"      1         0         ∞         # Turn feature on\n"
    "      \"kern[:]\"     1         0         ∞         # Turn feature on\n"
    "      \"kern[5:]\"    1         5         ∞         # Turn feature on, partial\n"
    "      \"kern[:5]\"    1         0         5         # Turn feature on, partial\n"
    "      \"kern[3:5]\"   1         3         5         # Turn feature on, range\n"
    "      \"kern[3]\"     1         3         3+1       # Turn feature on, single char\n"
    "\n"
    "    Mixing it all:\n"
    "\n"
    "      \"aalt[3:5]=2\" 2         3         5         # Turn 2nd alternate on for range";

  GOptionEntry entries2[] =
  {
    {"features",	0, 0, G_OPTION_ARG_CALLBACK,	(gpointer) &parse_features,	features_help,	"list"},
    {NULL}
  };
  parser->add_group (entries2,
		     "features",
		     "Features options:",
		     "Options controlling font features used",
		     this);
}

void
font_options_t::add_options (option_parser_t *parser)
{
  char *text = NULL;

  {
    ASSERT_STATIC (ARRAY_LENGTH_CONST (supported_font_funcs) > 0);
    GString *s = g_string_new (NULL);
    g_string_printf (s, "Set font functions implementation to use (default: %s)\n\n    Supported font function implementations are: %s",
		     supported_font_funcs[0].name,
		     supported_font_funcs[0].name);
    for (unsigned int i = 1; i < ARRAY_LENGTH (supported_font_funcs); i++)
    {
      g_string_append_c (s, '/');
      g_string_append (s, supported_font_funcs[i].name);
    }
    text = g_string_free (s, FALSE);
    parser->free_later (text);
  }

  GOptionEntry entries[] =
  {
    {"font-file",	0, 0, G_OPTION_ARG_STRING,	&this->font_file,		"Set font file-name",			"filename"},
    {"face-index",	0, 0, G_OPTION_ARG_INT,		&this->face_index,		"Set face index (default: 0)",		"index"},
    {"font-funcs",	0, 0, G_OPTION_ARG_STRING,	&this->font_funcs,		text,					"format"},
    {NULL}
  };
  parser->add_group (entries,
		     "font",
		     "Font options:",
		     "Options controlling the font",
		     this);
}

void
text_options_t::add_options (option_parser_t *parser)
{
  GOptionEntry entries[] =
  {
    {"text",		0, 0, G_OPTION_ARG_STRING,	&this->text,			"Set input text",			"string"},
    {"text-file",	0, 0, G_OPTION_ARG_STRING,	&this->text_file,		"Set input text file-name\n\n    If no text is provided, standard input is used for input.\n",		"filename"},
    {"text-before",	0, 0, G_OPTION_ARG_STRING,	&this->text_before,		"Set text context before each line",	"string"},
    {"text-after",	0, 0, G_OPTION_ARG_STRING,	&this->text_after,		"Set text context after each line",	"string"},
    {NULL}
  };
  parser->add_group (entries,
		     "text",
		     "Text options:",
		     "Options controlling the input text",
		     this);
}

void
output_options_t::add_options (option_parser_t *parser)
{
  const char *text;

  if (NULL == supported_formats)
    text = "Set output format";
  else
  {
    char *items = g_strjoinv ("/", const_cast<char **> (supported_formats));
    text = g_strdup_printf ("Set output format\n\n    Supported output formats are: %s", items);
    g_free (items);
    parser->free_later ((char *) text);
  }

  GOptionEntry entries[] =
  {
    {"output-file",	0, 0, G_OPTION_ARG_STRING,	&this->output_file,		"Set output file-name (default: stdout)","filename"},
    {"output-format",	0, 0, G_OPTION_ARG_STRING,	&this->output_format,		text,					"format"},
    {NULL}
  };
  parser->add_group (entries,
		     "output",
		     "Output options:",
		     "Options controlling the output",
		     this);
}



hb_font_t *
font_options_t::get_font (void) const
{
  if (font)
    return font;

  hb_blob_t *blob = NULL;

  /* Create the blob */
  {
    char *font_data;
    unsigned int len = 0;
    hb_destroy_func_t destroy;
    void *user_data;
    hb_memory_mode_t mm;

    /* This is a hell of a lot of code for just reading a file! */
    if (!font_file)
      fail (true, "No font file set");

    if (0 == strcmp (font_file, "-")) {
      /* read it */
      GString *gs = g_string_new (NULL);
      char buf[BUFSIZ];
#if defined(_WIN32) || defined(__CYGWIN__)
      setmode (fileno (stdin), _O_BINARY);
#endif
      while (!feof (stdin)) {
	size_t ret = fread (buf, 1, sizeof (buf), stdin);
	if (ferror (stdin))
	  fail (false, "Failed reading font from standard input: %s",
		strerror (errno));
	g_string_append_len (gs, buf, ret);
      }
      len = gs->len;
      font_data = g_string_free (gs, false);
      user_data = font_data;
      destroy = (hb_destroy_func_t) g_free;
      mm = HB_MEMORY_MODE_WRITABLE;
    } else {
      GError *error = NULL;
      GMappedFile *mf = g_mapped_file_new (font_file, false, &error);
      if (mf) {
	font_data = g_mapped_file_get_contents (mf);
	len = g_mapped_file_get_length (mf);
	if (len) {
	  destroy = (hb_destroy_func_t) g_mapped_file_unref;
	  user_data = (void *) mf;
	  mm = HB_MEMORY_MODE_READONLY_MAY_MAKE_WRITABLE;
	} else
	  g_mapped_file_unref (mf);
      } else {
	fail (false, "%s", error->message);
	//g_error_free (error);
      }
      if (!len) {
	/* GMappedFile is buggy, it doesn't fail if file isn't regular.
	 * Try reading.
	 * https://bugzilla.gnome.org/show_bug.cgi?id=659212 */
        GError *error = NULL;
	gsize l;
	if (g_file_get_contents (font_file, &font_data, &l, &error)) {
	  len = l;
	  destroy = (hb_destroy_func_t) g_free;
	  user_data = (void *) font_data;
	  mm = HB_MEMORY_MODE_WRITABLE;
	} else {
	  fail (false, "%s", error->message);
	  //g_error_free (error);
	}
      }
    }

    blob = hb_blob_create (font_data, len, mm, user_data, destroy);
  }

  /* Create the face */
  hb_face_t *face = hb_face_create (blob, face_index);
  hb_blob_destroy (blob);


  font = hb_font_create (face);

  unsigned int upem = hb_face_get_upem (face);
  hb_font_set_scale (font, upem, upem);
  hb_face_destroy (face);

  void (*set_font_funcs) (hb_font_t *) = NULL;
  if (!font_funcs)
  {
    set_font_funcs = supported_font_funcs[0].func;
  }
  else
  {
    for (unsigned int i = 0; i < ARRAY_LENGTH (supported_font_funcs); i++)
      if (0 == strcasecmp (font_funcs, supported_font_funcs[i].name))
      {
	set_font_funcs = supported_font_funcs[i].func;
	break;
      }
    if (!set_font_funcs)
    {
      GString *s = g_string_new (NULL);
      for (unsigned int i = 0; i < ARRAY_LENGTH (supported_font_funcs); i++)
      {
        if (i)
	  g_string_append_c (s, '/');
	g_string_append (s, supported_font_funcs[i].name);
      }
      char *p = g_string_free (s, FALSE);
      fail (false, "Unknown font function implementation `%s'; supported values are: %s; default is %s",
	    font_funcs,
	    p,
	    supported_font_funcs[0].name);
      //free (p);
    }
  }
  set_font_funcs (font);

  return font;
}


const char *
text_options_t::get_line (unsigned int *len)
{
  if (text) {
    if (text_len == (unsigned int) -1)
      text_len = strlen (text);

    if (!text_len) {
      *len = 0;
      return NULL;
    }

    const char *ret = text;
    const char *p = (const char *) memchr (text, '\n', text_len);
    unsigned int ret_len;
    if (!p) {
      ret_len = text_len;
      text += ret_len;
      text_len = 0;
    } else {
      ret_len = p - ret;
      text += ret_len + 1;
      text_len -= ret_len + 1;
    }

    *len = ret_len;
    return ret;
  }

  if (!fp) {
    if (!text_file)
      fail (true, "At least one of text or text-file must be set");

    if (0 != strcmp (text_file, "-"))
      fp = fopen (text_file, "r");
    else
      fp = stdin;

    if (!fp)
      fail (false, "Failed opening text file `%s': %s",
	    text_file, strerror (errno));

    gs = g_string_new (NULL);
  }

  g_string_set_size (gs, 0);
  char buf[BUFSIZ];
  while (fgets (buf, sizeof (buf), fp)) {
    unsigned int bytes = strlen (buf);
    if (bytes && buf[bytes - 1] == '\n') {
      bytes--;
      g_string_append_len (gs, buf, bytes);
      break;
    }
      g_string_append_len (gs, buf, bytes);
  }
  if (ferror (fp))
    fail (false, "Failed reading text: %s",
	  strerror (errno));
  *len = gs->len;
  return !*len && feof (fp) ? NULL : gs->str;
}


FILE *
output_options_t::get_file_handle (void)
{
  if (fp)
    return fp;

  if (output_file)
    fp = fopen (output_file, "wb");
  else {
#if defined(_WIN32) || defined(__CYGWIN__)
    setmode (fileno (stdout), _O_BINARY);
#endif
    fp = stdout;
  }
  if (!fp)
    fail (false, "Cannot open output file `%s': %s",
	  g_filename_display_name (output_file), strerror (errno));

  return fp;
}

static gboolean
parse_verbose (const char *name G_GNUC_UNUSED,
	       const char *arg G_GNUC_UNUSED,
	       gpointer    data G_GNUC_UNUSED,
	       GError    **error G_GNUC_UNUSED)
{
  format_options_t *format_opts = (format_options_t *) data;
  format_opts->show_text = format_opts->show_unicode = format_opts->show_line_num = true;
  return true;
}

void
format_options_t::add_options (option_parser_t *parser)
{
  GOptionEntry entries[] =
  {
    {"no-glyph-names",	0, G_OPTION_FLAG_REVERSE, G_OPTION_ARG_NONE,	&this->show_glyph_names,	"Use glyph indices instead of names",	NULL},
    {"no-positions",	0, G_OPTION_FLAG_REVERSE, G_OPTION_ARG_NONE,	&this->show_positions,		"Do not show glyph positions",		NULL},
    {"no-clusters",	0, G_OPTION_FLAG_REVERSE, G_OPTION_ARG_NONE,	&this->show_clusters,		"Do not show cluster mapping",		NULL},
    {"show-text",	0, 0,			  G_OPTION_ARG_NONE,	&this->show_text,		"Show input text",			NULL},
    {"show-unicode",	0, 0,			  G_OPTION_ARG_NONE,	&this->show_unicode,		"Show input Unicode codepoints",	NULL},
    {"show-line-num",	0, 0,			  G_OPTION_ARG_NONE,	&this->show_line_num,		"Show line numbers",			NULL},
    {"verbose",		0, G_OPTION_FLAG_NO_ARG,  G_OPTION_ARG_CALLBACK,(gpointer) &parse_verbose,	"Show everything",			NULL},
    {NULL}
  };
  parser->add_group (entries,
		     "format",
		     "Format options:",
		     "Options controlling the formatting of buffer contents",
		     this);
}

void
format_options_t::serialize_unicode (hb_buffer_t *buffer,
				     GString     *gs)
{
  unsigned int num_glyphs = hb_buffer_get_length (buffer);
  hb_glyph_info_t *info = hb_buffer_get_glyph_infos (buffer, NULL);

  g_string_append_c (gs, '<');
  for (unsigned int i = 0; i < num_glyphs; i++)
  {
    if (i)
      g_string_append_c (gs, ',');
    g_string_append_printf (gs, "U+%04X", info->codepoint);
    info++;
  }
  g_string_append_c (gs, '>');
}

void
format_options_t::serialize_glyphs (hb_buffer_t *buffer,
				    hb_font_t   *font,
				    hb_buffer_serialize_format_t output_format,
				    hb_buffer_serialize_flags_t flags,
				    GString     *gs)
{
  g_string_append_c (gs, '[');
  unsigned int num_glyphs = hb_buffer_get_length (buffer);
  unsigned int start = 0;

  while (start < num_glyphs) {
    char buf[1024];
    unsigned int consumed;
    start += hb_buffer_serialize_glyphs (buffer, start, num_glyphs,
					 buf, sizeof (buf), &consumed,
					 font, output_format, flags);
    if (!consumed)
      break;
    g_string_append (gs, buf);
  }
  g_string_append_c (gs, ']');
}
void
format_options_t::serialize_line_no (unsigned int  line_no,
				     GString      *gs)
{
  if (show_line_num)
    g_string_append_printf (gs, "%d: ", line_no);
}
void
format_options_t::serialize_buffer_of_text (hb_buffer_t  *buffer,
					    unsigned int  line_no,
					    const char   *text,
					    unsigned int  text_len,
					    hb_font_t    *font,
					    GString      *gs)
{
  if (show_text) {
    serialize_line_no (line_no, gs);
    g_string_append_c (gs, '(');
    g_string_append_len (gs, text, text_len);
    g_string_append_c (gs, ')');
    g_string_append_c (gs, '\n');
  }

  if (show_unicode) {
    serialize_line_no (line_no, gs);
    serialize_unicode (buffer, gs);
    g_string_append_c (gs, '\n');
  }
}
void
format_options_t::serialize_message (unsigned int  line_no,
				     const char   *msg,
				     GString      *gs)
{
  serialize_line_no (line_no, gs);
  g_string_append_printf (gs, "%s", msg);
  g_string_append_c (gs, '\n');
}
void
format_options_t::serialize_buffer_of_glyphs (hb_buffer_t  *buffer,
					      unsigned int  line_no,
					      const char   *text,
					      unsigned int  text_len,
					      hb_font_t    *font,
					      hb_buffer_serialize_format_t output_format,
					      hb_buffer_serialize_flags_t format_flags,
					      GString      *gs)
{
  serialize_line_no (line_no, gs);
  serialize_glyphs (buffer, font, output_format, format_flags, gs);
  g_string_append_c (gs, '\n');
}
