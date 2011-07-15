/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#ifndef GrRefCnt_DEFINED
#define GrRefCnt_DEFINED

#include "GrTypes.h"
#include "SkRefCnt.h"

typedef SkRefCnt GrRefCnt;
typedef SkAutoRef GrAutoRef;
typedef SkAutoUnref GrAutoUnref;

static void GrSafeRef(const SkRefCnt* obj) { SkSafeRef(obj); }
static void GrSafeUnref(const SkRefCnt* obj) { SkSafeUnref(obj); }
#define GrSafeAssign(a, b)  SkRefCnt_SafeAssign(a, b)

template<typename T>
static inline void GrSafeSetNull(T*& obj) {
    if (NULL != obj) {
        obj->unref();
        obj = NULL;
    }
}

#endif

