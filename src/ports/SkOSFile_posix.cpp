/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOSFile.h"

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct {
    dev_t dev;
    ino_t ino;
} SkFILEID;

static bool sk_ino(SkFILE* a, SkFILEID* id) {
    int fd = fileno((FILE*)a);
    if (fd < 0) {
        return 0;
    }
    struct stat status;
    if (0 != fstat(fd, &status)) {
        return 0;
    }
    id->dev = status.st_dev;
    id->ino = status.st_ino;
    return true;
}

bool sk_fidentical(SkFILE* a, SkFILE* b) {
    SkFILEID aID, bID;
    return sk_ino(a, &aID) && sk_ino(b, &bID)
           && aID.ino == bID.ino
           && aID.dev == bID.dev;
}

void sk_fmunmap(const void* addr, size_t length) {
    munmap(const_cast<void*>(addr), length);
}

void* sk_fmmap(SkFILE* f, size_t* size) {
    size_t fileSize = sk_fgetsize(f);
    if (0 == fileSize) {
        return NULL;
    }

    int fd = fileno((FILE*)f);
    if (fd < 0) {
        return NULL;
    }

    void* addr = mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (MAP_FAILED == addr) {
        return NULL;
    }

    *size = fileSize;
    return addr;
}
