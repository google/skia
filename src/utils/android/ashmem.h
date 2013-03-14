/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _CUTILS_ASHMEM_H
#define _CUTILS_ASHMEM_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int ashmem_create_region(const char *name, size_t size);
int ashmem_set_prot_region(int fd, int prot);
int ashmem_pin_region(int fd, size_t offset, size_t len);
int ashmem_unpin_region(int fd, size_t offset, size_t len);
int ashmem_get_size_region(int fd);
int ashmem_purge_all_caches(int fd);

#ifdef __cplusplus
}
#endif

#ifndef __ASHMEMIOC /* in case someone included <linux/ashmem.h> too */

#define ASHMEM_NAME_LEN     256

#define ASHMEM_NAME_DEF     "dev/ashmem"

/* Return values from ASHMEM_PIN: Was the mapping purged while unpinned? */
#define ASHMEM_NOT_PURGED   0
#define ASHMEM_WAS_PURGED   1

/* Return values from ASHMEM_UNPIN: Is the mapping now pinned or unpinned? */
#define ASHMEM_IS_UNPINNED  0
#define ASHMEM_IS_PINNED    1

#endif  /* ! __ASHMEMIOC */

#endif  /* _CUTILS_ASHMEM_H */
