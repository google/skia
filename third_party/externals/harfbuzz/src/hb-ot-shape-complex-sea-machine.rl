/*
 * Copyright © 2011,2012,2013  Google, Inc.
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

#ifndef HB_OT_SHAPE_COMPLEX_SEA_MACHINE_HH
#define HB_OT_SHAPE_COMPLEX_SEA_MACHINE_HH

#include "hb-private.hh"

%%{
  machine sea_syllable_machine;
  alphtype unsigned char;
  write data;
}%%

%%{

# Same order as enum sea_category_t.  Not sure how to avoid duplication.
C    = 1;
GB   = 12; # Generic Base
H    = 4;  # Halant
IV   = 2;  # Independent Vowel
MR   = 22; # Medial Ra
CM   = 17; # Consonant Medial
VAbv = 26;
VBlw = 27;
VPre = 28;
VPst = 29;
T    = 3;  # Tone Marks
A    = 10; # Anusvara

syllable_tail = (VPre|VAbv|VBlw|VPst|H.C|CM|MR|T|A)*;

consonant_syllable =	(C|IV|GB) syllable_tail;
broken_cluster =	syllable_tail;
other =			any;

main := |*
	consonant_syllable	=> { found_syllable (consonant_syllable); };
	broken_cluster		=> { found_syllable (broken_cluster); };
	other			=> { found_syllable (non_sea_cluster); };
*|;


}%%

#define found_syllable(syllable_type) \
  HB_STMT_START { \
    if (0) fprintf (stderr, "syllable %d..%d %s\n", last, p+1, #syllable_type); \
    for (unsigned int i = last; i < p+1; i++) \
      info[i].syllable() = (syllable_serial << 4) | syllable_type; \
    last = p+1; \
    syllable_serial++; \
    if (unlikely (syllable_serial == 16)) syllable_serial = 1; \
  } HB_STMT_END

static void
find_syllables (hb_buffer_t *buffer)
{
  unsigned int p, pe, eof, ts HB_UNUSED, te HB_UNUSED, act HB_UNUSED;
  int cs;
  hb_glyph_info_t *info = buffer->info;
  %%{
    write init;
    getkey info[p].sea_category();
  }%%

  p = 0;
  pe = eof = buffer->len;

  unsigned int last = 0;
  unsigned int syllable_serial = 1;
  %%{
    write exec;
  }%%
}

#undef found_syllable

#endif /* HB_OT_SHAPE_COMPLEX_SEA_MACHINE_HH */
