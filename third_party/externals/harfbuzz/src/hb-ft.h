/*
 * Copyright © 2009  Red Hat, Inc.
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
 * Red Hat Author(s): Behdad Esfahbod
 */

#ifndef HB_FT_H
#define HB_FT_H

#include "hb.h"

#include <ft2build.h>
#include FT_FREETYPE_H

HB_BEGIN_DECLS

/* Note: FreeType is not thread-safe.  Hence, these functions are not either. */

hb_face_t *
hb_ft_face_create (FT_Face           ft_face,
		   hb_destroy_func_t destroy);

hb_face_t *
hb_ft_face_create_cached (FT_Face ft_face);

hb_font_t *
hb_ft_font_create (FT_Face           ft_face,
		   hb_destroy_func_t destroy);



/* Makes an hb_font_t use FreeType internally to implement font functions. */
void
hb_ft_font_set_funcs (hb_font_t *font);

FT_Face
hb_ft_font_get_face (hb_font_t *font);


HB_END_DECLS

#endif /* HB_FT_H */
