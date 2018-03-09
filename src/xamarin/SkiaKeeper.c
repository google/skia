//
//  SkiaKeeper.c
//
//  Created by Bill Holmes on 11/23/15.
//  Copyright © 2015 Xamarin. All rights reserved.
//

#include "sk_xamarin.h"

// Skia
#include "gr_context.h"
#include "sk_bitmap.h"
#include "sk_canvas.h"
#include "sk_codec.h"
#include "sk_colorfilter.h"
#include "sk_colorspace.h"
#include "sk_colortable.h"
#include "sk_data.h"
#include "sk_document.h"
#include "sk_image.h"
#include "sk_imagefilter.h"
#include "sk_mask.h"
#include "sk_maskfilter.h"
#include "sk_matrix.h"
#include "sk_paint.h"
#include "sk_path.h"
#include "sk_patheffect.h"
#include "sk_picture.h"
#include "sk_pixmap.h"
#include "sk_pixelserializer.h"
#include "sk_region.h"
#include "sk_shader.h"
#include "sk_stream.h"
#include "sk_string.h"
#include "sk_surface.h"
#include "sk_svg.h"
#include "sk_typeface.h"
#include "sk_xml.h"
#include "sk_vertices.h"

// Xamarin
#include "sk_managedstream.h"
#include "sk_managedpixelserializer.h"

SK_X_API void** KeepSkiaCSymbols ();

void** KeepSkiaCSymbols ()
{
    static void* ret[] = {
        // Skia
        (void*)gr_context_create,
        (void*)gr_glinterface_create_native_interface,
        (void*)sk_bitmap_new,
        (void*)sk_canvas_destroy,
        (void*)sk_codec_min_buffered_bytes_needed,
        (void*)sk_colorfilter_unref,
        (void*)sk_colorspace_unref,
        (void*)sk_colortable_unref,
        (void*)sk_data_new_empty,
        (void*)sk_document_unref,
        (void*)sk_image_ref,
        (void*)sk_imagefilter_croprect_new,
        (void*)sk_mask_alloc_image,
        (void*)sk_maskfilter_ref,
        (void*)sk_matrix_set_identity,
        (void*)sk_paint_new,
        (void*)sk_path_new,
        (void*)sk_path_effect_unref,
        (void*)sk_picture_recorder_new,
        (void*)sk_pixmap_destructor,
        (void*)sk_pixelserializer_unref,
        (void*)sk_region_new,
        (void*)sk_shader_ref,
        (void*)sk_stream_asset_destroy,
        (void*)sk_string_new_empty,
        (void*)sk_colortype_get_default_8888,
        (void*)sk_svgcanvas_create,
        (void*)sk_typeface_create_from_name,
        (void*)sk_xmlstreamwriter_new,
        (void*)sk_vertices_unref,

        // Xamarin
        (void*)sk_managedstream_new,
        (void*)sk_managedpixelserializer_new,
    };
    return ret;
}
