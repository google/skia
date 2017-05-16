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

#ifndef HB_MAIN_FONT_TEXT_HH
#define HB_MAIN_FONT_TEXT_HH

/* main() body for utilities taking font and processing text.*/

template <typename consumer_t>
struct main_font_text_t
{
  main_font_text_t (void)
		  : options ("[FONT-FILE] [TEXT]"),
		    font_opts (&options),
		    input (&options),
		    consumer (&options) {}

  int
  main (int argc, char **argv)
  {
    options.parse (&argc, &argv);

    argc--, argv++;
    if (argc && !font_opts.font_file) font_opts.font_file = argv[0], argc--, argv++;
    if (argc && !input.text && !input.text_file) input.text = argv[0], argc--, argv++;
    if (argc)
      fail (true, "Too many arguments on the command line");
    if (!font_opts.font_file)
      options.usage ();
    if (!input.text && !input.text_file)
      input.text_file = "-";

    consumer.init (&font_opts);

    hb_buffer_t *buffer = hb_buffer_create ();
    unsigned int text_len;
    const char *text;
    while ((text = input.get_line (&text_len)))
      consumer.consume_line (buffer, text, text_len, input.text_before, input.text_after);
    hb_buffer_destroy (buffer);

    consumer.finish (&font_opts);

    return consumer.failed ? 1 : 0;
  }

  protected:
  option_parser_t options;
  font_options_t font_opts;
  text_options_t input;
  consumer_t consumer;
};

#endif

