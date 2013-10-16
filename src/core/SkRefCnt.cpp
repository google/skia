/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkRefCnt.h"
#include "SkWeakRefCnt.h"

SK_DEFINE_INST_COUNT(SkRefCnt)
SK_DEFINE_INST_COUNT(SkWeakRefCnt)

#ifdef SK_BUILD_FOR_WIN
SkRefCnt::SkRefCnt(const SkRefCnt&) { }
SkRefCnt& SkRefCnt::operator=(const SkRefCnt&) { return *this; }
#endif

