/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPixmap.h"

#include "sk_pixmap.h"

#include "sk_types_priv.h"


void sk_pixmap_destructor(sk_pixmap_t* cpixmap)
{
    delete AsPixmap(cpixmap);
}

sk_pixmap_t* sk_pixmap_new()
{
    return ToPixmap(new SkPixmap());
}

sk_pixmap_t* sk_pixmap_new_with_params(const sk_imageinfo_t* cinfo, const void* addr, size_t rowBytes, sk_colortable_t* ctable)
{
    SkImageInfo info;
    from_c(*cinfo, &info);

    return ToPixmap(new SkPixmap(info, addr, rowBytes, AsColorTable(ctable)));
}

void sk_pixmap_reset(sk_pixmap_t* cpixmap)
{
    AsPixmap(cpixmap)->reset();
}

void sk_pixmap_reset_with_params(sk_pixmap_t* cpixmap, const sk_imageinfo_t* cinfo, const void* addr, size_t rowBytes, sk_colortable_t* ctable)
{
    SkImageInfo info;
    from_c(*cinfo, &info);

    AsPixmap(cpixmap)->reset(info, addr, rowBytes, AsColorTable(ctable));
}

void sk_pixmap_get_info(sk_pixmap_t* cpixmap, sk_imageinfo_t* cinfo)
{
    from_sk(AsPixmap(cpixmap)->info(), cinfo);
}

size_t sk_pixmap_get_row_bytes(sk_pixmap_t* cpixmap)
{
    return AsPixmap(cpixmap)->rowBytes();
}

const void* sk_pixmap_get_pixels(sk_pixmap_t* cpixmap)
{
    return AsPixmap(cpixmap)->addr();
}

sk_colortable_t* sk_pixmap_get_colortable(sk_pixmap_t* cpixmap)
{
    return ToColorTable(AsPixmap(cpixmap)->ctable());
}
