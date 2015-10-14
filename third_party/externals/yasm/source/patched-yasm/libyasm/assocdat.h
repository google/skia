/**
 * \file assocdat.h
 * \brief YASM associated data storage (libyasm internal use)
 *
 * \license
 *  Copyright (C) 2003-2007  Peter Johnson
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
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
 * \endlicense
 */
#ifndef YASM_ASSOCDAT_H
#define YASM_ASSOCDAT_H

#ifndef YASM_LIB_DECL
#define YASM_LIB_DECL
#endif

/** Associated data container. */
typedef struct yasm__assoc_data yasm__assoc_data;

/** Create an associated data container. */
YASM_LIB_DECL
/*@only@*/ yasm__assoc_data *yasm__assoc_data_create(void);

/** Get associated data for a data callback.
 * \param assoc_data    container of associated data
 * \param callback      callback used when adding data
 * \return Associated data (NULL if none).
 */
YASM_LIB_DECL
/*@dependent@*/ /*@null@*/ void *yasm__assoc_data_get
    (/*@null@*/ yasm__assoc_data *assoc_data,
     const yasm_assoc_data_callback *callback);

/** Add associated data to a associated data container.
 * \attention Deletes any existing associated data for that data callback.
 * \param assoc_data    container of associated data
 * \param callback      callback
 * \param data          data to associate
 */
YASM_LIB_DECL
/*@only@*/ yasm__assoc_data *yasm__assoc_data_add
    (/*@null@*/ /*@only@*/ yasm__assoc_data *assoc_data,
     const yasm_assoc_data_callback *callback,
     /*@only@*/ /*@null@*/ void *data);

/** Destroy all associated data in a container. */
YASM_LIB_DECL
void yasm__assoc_data_destroy
    (/*@null@*/ /*@only@*/ yasm__assoc_data *assoc_data);

/** Print all associated data in a container. */
YASM_LIB_DECL
void yasm__assoc_data_print(const yasm__assoc_data *assoc_data, FILE *f,
                            int indent_level);

#endif
