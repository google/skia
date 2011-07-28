
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkThread.h"

int32_t sk_atomic_inc(int32_t* addr)
{
    int32_t value = *addr;
    *addr = value + 1;
    return value;
}

int32_t sk_atomic_dec(int32_t* addr)
{
    int32_t value = *addr;
    *addr = value - 1;
    return value;
}

SkMutex::SkMutex(bool /* isGlobal */)
{
}

SkMutex::~SkMutex()
{
}

void SkMutex::acquire()
{
}

void SkMutex::release()
{
}

