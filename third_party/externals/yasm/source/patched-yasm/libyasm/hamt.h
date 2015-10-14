/**
 * \file libyasm/hamt.h
 * \brief Hash Array Mapped Trie (HAMT) functions.
 *
 * \license
 *  Copyright (C) 2001-2007  Peter Johnson
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
 * \endlicense
 */
#ifndef YASM_HAMT_H
#define YASM_HAMT_H

#ifndef YASM_LIB_DECL
#define YASM_LIB_DECL
#endif

/** Hash array mapped trie data structure (opaque type). */
typedef struct HAMT HAMT;
/** Hash array mapped trie entry (opaque type). */
typedef struct HAMTEntry HAMTEntry;

/** Create new, empty, HAMT.  error_func() is called when an internal error is
 * encountered--it should NOT return to the calling function.
 * \param   nocase          nonzero if HAMT should be case-insensitive
 * \param   error_func      function called on internal error
 * \return New, empty, hash array mapped trie.
 */
YASM_LIB_DECL
HAMT *HAMT_create(int nocase, /*@exits@*/ void (*error_func)
    (const char *file, unsigned int line, const char *message));

/** Delete HAMT and all data associated with it.  Uses deletefunc() to delete
 * each data item.
 * \param hamt          Hash array mapped trie
 * \param deletefunc    Data deletion function
 */
YASM_LIB_DECL
void HAMT_destroy(/*@only@*/ HAMT *hamt,
                  void (*deletefunc) (/*@only@*/ void *data));

/** Insert key into HAMT, associating it with data. 
 * If the key is not present in the HAMT, inserts it, sets *replace to 1, and
 *  returns the data passed in.
 * If the key is already present and *replace is 0, deletes the data passed
 *  in using deletefunc() and returns the data currently associated with the
 *  key.
 * If the key is already present and *replace is 1, deletes the data currently
 *  associated with the key using deletefunc() and replaces it with the data
 *  passed in.
 * \param hamt          Hash array mapped trie
 * \param str           Key
 * \param data          Data to associate with key
 * \param replace       See above description
 * \param deletefunc    Data deletion function if data is replaced
 * \return Data now associated with key.
 */
YASM_LIB_DECL
/*@dependent@*/ void *HAMT_insert(HAMT *hamt, /*@dependent@*/ const char *str,
                                  /*@only@*/ void *data, int *replace,
                                  void (*deletefunc) (/*@only@*/ void *data));

/** Search for the data associated with a key in the HAMT.
 * \param hamt          Hash array mapped trie
 * \param str           Key
 * \return NULL if key/data not present in HAMT, otherwise associated data.
 */
YASM_LIB_DECL
/*@dependent@*/ /*@null@*/ void *HAMT_search(HAMT *hamt, const char *str);

/** Traverse over all keys in HAMT, calling function on each data item. 
 * \param hamt          Hash array mapped trie
 * \param d             Data to pass to each call to func.
 * \param func          Function to call
 * \return Stops early (and returns func's return value) if func returns a
 *         nonzero value; otherwise 0.
 */
YASM_LIB_DECL
int HAMT_traverse(HAMT *hamt, /*@null@*/ void *d,
                  int (*func) (/*@dependent@*/ /*@null@*/ void *node,
                               /*@null@*/ void *d));

/** Get the first entry in a HAMT.
 * \param hamt          Hash array mapped trie
 * \return First entry in HAMT, or NULL if HAMT is empty.
 */
YASM_LIB_DECL
const HAMTEntry *HAMT_first(const HAMT *hamt);

/** Get the next entry in a HAMT.
 * \param prev          Previous entry in HAMT
 * \return Next entry in HAMT, or NULL if no more entries.
 */
YASM_LIB_DECL
/*@null@*/ const HAMTEntry *HAMT_next(const HAMTEntry *prev);

/** Get the corresponding data for a HAMT entry.
 * \param entry         HAMT entry (as returned by HAMT_first() and HAMT_next())
 * \return Corresponding data item.
 */
YASM_LIB_DECL
void *HAMTEntry_get_data(const HAMTEntry *entry);

#endif
