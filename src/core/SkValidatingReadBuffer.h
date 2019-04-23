/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkValidatingReadBuffer_DEFINED
#define SkValidatingReadBuffer_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRefCnt.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkReader32.h"
#include "src/core/SkWriteBuffer.h"

class SkBitmap;

// DEPRECATED -- just use SkReadBuffer (so we can delete this header)
typedef SkReadBuffer SkValidatingReadBuffer;

#endif
