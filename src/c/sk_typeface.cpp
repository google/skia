/*
 * Copyright 2015 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkMaskFilter.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPictureRecorder.h"
#include "SkSurface.h"
#include "SkTypeface.h"

#include "sk_canvas.h"
#include "sk_data.h"
#include "sk_image.h"
#include "sk_paint.h"
#include "sk_path.h"
#include "sk_surface.h"
#include "sk_typeface.h"
#include "sk_types_priv.h"

static inline SkTypeface::Style MapStyle(sk_typeface_style_t ostyle)
{
    SkTypeface::Style style;
    switch (ostyle) {
        #define MAP(X, Y) case (X): style = (Y); break
	    MAP( NORMAL_TYPEFACE_STYLE,      SkTypeface::kNormal );
	    MAP( BOLD_TYPEFACE_STYLE,        SkTypeface::kBold );
	    MAP( ITALIC_TYPEFACE_STYLE,      SkTypeface::kItalic );
	    MAP( BOLD_ITALIC_TYPEFACE_STYLE, SkTypeface::kBoldItalic );
        #undef MAP
        default:
		return SkTypeface::kNormal;
    }
    return style;
}

void sk_typeface_unref(sk_typeface_t* tf)
{
	((SkTypeface *)tf)->unref();
}

sk_typeface_t* sk_typeface_create_from_name(const char *familyName, sk_typeface_style_t sstyle)
{
	SkTypeface::Style style = MapStyle (sstyle);
	return (sk_typeface_t *) SkTypeface::CreateFromName (familyName, style);
}

sk_typeface_t* sk_typeface_create_from_typeface(sk_typeface_t* typeface, sk_typeface_style_t sstyle)
{
	SkTypeface::Style style = MapStyle (sstyle);
	return (sk_typeface_t *) SkTypeface::CreateFromTypeface ((SkTypeface *) typeface, style);
}

sk_typeface_t* sk_typeface_create_from_file(const char* path, int index)
{
	return (sk_typeface_t *) SkTypeface::CreateFromFile (path, index);
}

sk_typeface_t* sk_typeface_create_from_stream(sk_stream_asset_t* stream, int index)
{
	return (sk_typeface_t *) SkTypeface::CreateFromStream ((SkStreamAsset*) stream, index);
}

int sk_typeface_chars_to_glyphs (sk_typeface_t* typeface, const char *chars, sk_encoding_t encoding, uint16_t glyphs [], int glyphCount)
{
	SkTypeface::Encoding e;
	if (encoding == UTF8_ENCODING)
		e = SkTypeface::kUTF8_Encoding;
	else if (encoding == UTF16_ENCODING)
		e = SkTypeface::kUTF16_Encoding;
	else if (encoding == UTF32_ENCODING)
		e = SkTypeface::kUTF32_Encoding;
	else
		e = SkTypeface::kUTF8_Encoding;
	
	return ((SkTypeface *)typeface)->charsToGlyphs(chars, e, glyphs, glyphCount);
}

int sk_typeface_glyph_count (sk_typeface_t* typeface)
{
	return ((SkTypeface*) typeface)->countGlyphs();
}
