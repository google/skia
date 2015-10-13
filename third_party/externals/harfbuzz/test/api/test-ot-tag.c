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

#include "hb-test.h"

#include <hb-ot.h>

/* Unit tests for hb-ot-tag.h */


/* https://www.microsoft.com/typography/otspec/scripttags.htm */

static void
test_simple_tags (const char *s, hb_script_t script)
{
  hb_script_t tag;
  hb_script_t t1, t2;

  g_test_message ("Testing script %c%c%c%c: tag %s", HB_UNTAG (hb_script_to_iso15924_tag (script)), s);
  tag = hb_tag_from_string (s, -1);

  hb_ot_tags_from_script (script, &t1, &t2);

  g_assert_cmphex (t1, ==, tag);
  g_assert_cmphex (t2, ==, HB_OT_TAG_DEFAULT_SCRIPT);

  g_assert_cmphex (hb_ot_tag_to_script (tag), ==, script);
}

static void
test_indic_tags (const char *s1, const char *s2, hb_script_t script)
{
  hb_script_t tag1, tag2;
  hb_script_t t1, t2;

  g_test_message ("Testing script %c%c%c%c: new tag %s, old tag %s", HB_UNTAG (hb_script_to_iso15924_tag (script)), s1, s2);
  tag1 = hb_tag_from_string (s1, -1);
  tag2 = hb_tag_from_string (s2, -1);

  hb_ot_tags_from_script (script, &t1, &t2);

  g_assert_cmphex (t1, ==, tag1);
  g_assert_cmphex (t2, ==, tag2);

  g_assert_cmphex (hb_ot_tag_to_script (tag1), ==, script);
  g_assert_cmphex (hb_ot_tag_to_script (tag2), ==, script);
}

static void
test_ot_tag_script_degenerate (void)
{
  hb_script_t t1, t2;

  g_assert_cmphex (HB_TAG_CHAR4 ("DFLT"), ==, HB_OT_TAG_DEFAULT_SCRIPT);

  /* HIRAGANA and KATAKANA both map to 'kana' */
  test_simple_tags ("kana", HB_SCRIPT_KATAKANA);
  hb_ot_tags_from_script (HB_SCRIPT_HIRAGANA, &t1, &t2);
  g_assert_cmphex (t1, ==, HB_TAG_CHAR4 ("kana"));
  g_assert_cmphex (t2, ==, HB_OT_TAG_DEFAULT_SCRIPT);

  test_simple_tags ("DFLT", HB_SCRIPT_INVALID);

  /* Spaces are replaced */
  g_assert_cmphex (hb_ot_tag_to_script (HB_TAG_CHAR4 ("be  ")), ==, hb_script_from_string ("Beee", -1));
}

static void
test_ot_tag_script_simple (void)
{
  /* Arbitrary non-existent script */
  test_simple_tags ("wwyz", hb_script_from_string ("wWyZ", -1));

  /* These we don't really care about */
  test_simple_tags ("zyyy", HB_SCRIPT_COMMON);
  test_simple_tags ("zinh", HB_SCRIPT_INHERITED);
  test_simple_tags ("zzzz", HB_SCRIPT_UNKNOWN);

  test_simple_tags ("arab", HB_SCRIPT_ARABIC);
  test_simple_tags ("copt", HB_SCRIPT_COPTIC);
  test_simple_tags ("kana", HB_SCRIPT_KATAKANA);
  test_simple_tags ("latn", HB_SCRIPT_LATIN);

  /* These are trickier since their OT script tags have space. */
  test_simple_tags ("lao ", HB_SCRIPT_LAO);
  test_simple_tags ("yi  ", HB_SCRIPT_YI);
  /* Unicode-5.0 additions */
  test_simple_tags ("nko ", HB_SCRIPT_NKO);
  /* Unicode-5.1 additions */
  test_simple_tags ("vai ", HB_SCRIPT_VAI);

  /* https://www.microsoft.com/typography/otspec160/scripttagsProposed.htm */

  /* Unicode-5.2 additions */
  test_simple_tags ("mtei", HB_SCRIPT_MEETEI_MAYEK);
  /* Unicode-6.0 additions */
  test_simple_tags ("mand", HB_SCRIPT_MANDAIC);
}

static void
test_ot_tag_script_indic (void)
{
  test_indic_tags ("bng2", "beng", HB_SCRIPT_BENGALI);
  test_indic_tags ("dev2", "deva", HB_SCRIPT_DEVANAGARI);
  test_indic_tags ("gjr2", "gujr", HB_SCRIPT_GUJARATI);
  test_indic_tags ("gur2", "guru", HB_SCRIPT_GURMUKHI);
  test_indic_tags ("knd2", "knda", HB_SCRIPT_KANNADA);
  test_indic_tags ("mlm2", "mlym", HB_SCRIPT_MALAYALAM);
  test_indic_tags ("ory2", "orya", HB_SCRIPT_ORIYA);
  test_indic_tags ("tml2", "taml", HB_SCRIPT_TAMIL);
  test_indic_tags ("tel2", "telu", HB_SCRIPT_TELUGU);
  test_indic_tags ("mym2", "mymr", HB_SCRIPT_MYANMAR);
}



/* https://www.microsoft.com/typography/otspec/languagetags.htm */

static void
test_language_two_way (const char *tag_s, const char *lang_s)
{
  hb_language_t lang = hb_language_from_string (lang_s, -1);
  hb_tag_t tag = hb_tag_from_string (tag_s, -1);

  g_test_message ("Testing language %s <-> tag %s", lang_s, tag_s);

  g_assert_cmphex (tag, ==, hb_ot_tag_from_language (lang));
  g_assert (lang == hb_ot_tag_to_language (tag));
}

static void
test_tag_from_language (const char *tag_s, const char *lang_s)
{
  hb_language_t lang = hb_language_from_string (lang_s, -1);
  hb_tag_t tag = hb_tag_from_string (tag_s, -1);

  g_test_message ("Testing language %s -> tag %s", lang_s, tag_s);

  g_assert_cmphex (tag, ==, hb_ot_tag_from_language (lang));
}

static void
test_tag_to_language (const char *tag_s, const char *lang_s)
{
  hb_language_t lang = hb_language_from_string (lang_s, -1);
  hb_tag_t tag = hb_tag_from_string (tag_s, -1);

  g_test_message ("Testing tag %s -> language %s", tag_s, lang_s);

  g_assert (lang == hb_ot_tag_to_language (tag));
}

static void
test_ot_tag_language (void)
{
  g_assert_cmphex (HB_TAG_CHAR4 ("dflt"), ==, HB_OT_TAG_DEFAULT_LANGUAGE);
  test_language_two_way ("dflt", NULL);

  test_language_two_way ("ARA", "ar");

  test_language_two_way ("AZE", "az");
  test_tag_from_language ("AZE", "az-ir");
  test_tag_from_language ("AZE", "az-az");

  test_language_two_way ("ENG", "en");
  test_tag_from_language ("ENG", "en_US");

  test_language_two_way ("EVN", "eve");

  test_language_two_way ("FAR", "fa");
  test_tag_from_language ("FAR", "fa_IR");

  test_language_two_way ("ZHH", "zh-hk"); /* Chinese (Hong Kong) */

  test_tag_from_language ("ZHS", "zh"); /* Chinese */
  test_tag_from_language ("ZHS", "zh-cn"); /* Chinese (China) */
  test_tag_from_language ("ZHS", "zh-sg"); /* Chinese (Singapore) */
  test_tag_from_language ("ZHT", "zh-mo"); /* Chinese (Macao) */
  test_tag_from_language ("ZHT", "zh-tw"); /* Chinese (Taiwan) */
  test_tag_from_language ("ZHS", "zh-Hans"); /* Chinese (Simplified) */
  test_tag_from_language ("ZHT", "zh-Hant"); /* Chinese (Traditional) */
  test_tag_from_language ("ZHS", "zh-xx"); /* Chinese (Other) */

  test_tag_from_language ("ZHS", "zh"); /* Chinese */
  test_tag_from_language ("ZHS", "zh-xx");

  test_tag_to_language ("ZHS", "zh-Hans");
  test_tag_to_language ("ZHT", "zh-Hant");
  test_tag_to_language ("ZHP", "x-hbotzhp");

  test_language_two_way ("ABC", "x-hbotabc");
  test_tag_from_language ("ABC", "asdf-asdf-wer-x-hbotabc-zxc");
  test_tag_from_language ("ABC", "asdf-asdf-wer-x-hbotabc");
  test_tag_from_language ("ABCD", "asdf-asdf-wer-x-hbotabcd");

  test_tag_from_language ("dflt", "asdf-asdf-wer-x-hbot-zxc");

  test_tag_from_language ("dflt", "xy");
  test_tag_from_language ("XYZ", "xyz"); /* Unknown ISO 639-3 */
  test_tag_from_language ("XYZ", "xyz-qw"); /* Unknown ISO 639-3 */

  /* Test that x-hbot overrides the base language */
  test_tag_from_language ("ABC", "fa-x-hbotabc-zxc");
  test_tag_from_language ("ABC", "fa-ir-x-hbotabc-zxc");
  test_tag_from_language ("ABC", "zh-x-hbotabc-zxc");
  test_tag_from_language ("ABC", "zh-cn-x-hbotabc-zxc");
  test_tag_from_language ("ABC", "zh-xy-x-hbotabc-zxc");
  test_tag_from_language ("ABC", "xyz-xy-x-hbotabc-zxc");
}

int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);

  hb_test_add (test_ot_tag_script_degenerate);
  hb_test_add (test_ot_tag_script_simple);
  hb_test_add (test_ot_tag_script_indic);

  hb_test_add (test_ot_tag_language);

  return hb_test_run();
}
