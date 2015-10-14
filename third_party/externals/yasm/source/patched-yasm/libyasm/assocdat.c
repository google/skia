/*
 * YASM associated data storage (libyasm internal use)
 *
 *  Copyright (C) 2003-2007  Peter Johnson
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND OTHER CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR OTHER CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "util.h"

#include "coretype.h"
#include "assocdat.h"


typedef struct assoc_data_item {
    const yasm_assoc_data_callback *callback;
    void *data;
} assoc_data_item;

struct yasm__assoc_data {
    assoc_data_item *vector;
    size_t size;
    size_t alloc;
};


yasm__assoc_data *
yasm__assoc_data_create(void)
{
    yasm__assoc_data *assoc_data = yasm_xmalloc(sizeof(yasm__assoc_data));

    assoc_data->size = 0;
    assoc_data->alloc = 2;
    assoc_data->vector = yasm_xmalloc(assoc_data->alloc *
                                      sizeof(assoc_data_item));

    return assoc_data;
}

void *
yasm__assoc_data_get(yasm__assoc_data *assoc_data,
                     const yasm_assoc_data_callback *callback)
{
    size_t i;

    if (!assoc_data)
        return NULL;

    for (i=0; i<assoc_data->size; i++) {
        if (assoc_data->vector[i].callback == callback)
            return assoc_data->vector[i].data;
    }
    return NULL;
}

yasm__assoc_data *
yasm__assoc_data_add(yasm__assoc_data *assoc_data_arg,
                     const yasm_assoc_data_callback *callback, void *data)
{
    yasm__assoc_data *assoc_data;
    assoc_data_item *item = NULL;
    size_t i;

    /* Create a new assoc_data if necessary */
    if (assoc_data_arg)
        assoc_data = assoc_data_arg;
    else
        assoc_data = yasm__assoc_data_create();

    /* See if there's already assocated data for this callback */
    for (i=0; i<assoc_data->size; i++) {
        if (assoc_data->vector[i].callback == callback)
            item = &assoc_data->vector[i];
    }

    /* No?  Then append a new one */
    if (!item) {
        assoc_data->size++;
        if (assoc_data->size > assoc_data->alloc) {
            assoc_data->alloc *= 2;
            assoc_data->vector =
                yasm_xrealloc(assoc_data->vector,
                              assoc_data->alloc * sizeof(assoc_data_item));
        }
        item = &assoc_data->vector[assoc_data->size-1];
        item->callback = callback;
        item->data = NULL;
    }

    /* Delete existing data (if any) */
    if (item->data && item->data != data)
        item->callback->destroy(item->data);

    item->data = data;

    return assoc_data;
}

void
yasm__assoc_data_destroy(yasm__assoc_data *assoc_data)
{
    size_t i;

    if (!assoc_data)
        return;

    for (i=0; i<assoc_data->size; i++)
        assoc_data->vector[i].callback->destroy(assoc_data->vector[i].data);
    yasm_xfree(assoc_data->vector);
    yasm_xfree(assoc_data);
}

void
yasm__assoc_data_print(const yasm__assoc_data *assoc_data, FILE *f,
                       int indent_level)
{
    /*TODO*/
}
