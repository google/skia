
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include <stdlib.h>
#include "GrTypes.h"

void* GrMalloc(size_t bytes) {
    void* ptr = ::malloc(bytes);
    if (NULL == ptr) {
        ::exit(-1);
    }
    return ptr;
}

void GrFree(void* ptr) {
    if (ptr) {
        ::free(ptr);
    }
}
