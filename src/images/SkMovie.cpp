/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkMovie.h"
#include "SkCanvas.h"
#include "SkPaint.h"

// We should never see this in normal operation since our time values are
// 0-based. So we use it as a sentinal.
#define UNINITIALIZED_MSEC ((SkMSec)-1)

SkMovie::SkMovie()
{
    fInfo.fDuration = UNINITIALIZED_MSEC;  // uninitialized
    fCurrTime = UNINITIALIZED_MSEC; // uninitialized
    fNeedBitmap = true;
}

void SkMovie::ensureInfo()
{
    if (fInfo.fDuration == UNINITIALIZED_MSEC && !this->onGetInfo(&fInfo))
        memset(&fInfo, 0, sizeof(fInfo));   // failure
}

SkMSec SkMovie::duration()
{
    this->ensureInfo();
    return fInfo.fDuration;
}

int SkMovie::width()
{
    this->ensureInfo();
    return fInfo.fWidth;
}

int SkMovie::height()
{
    this->ensureInfo();
    return fInfo.fHeight;
}

int SkMovie::isOpaque()
{
    this->ensureInfo();
    return fInfo.fIsOpaque;
}

bool SkMovie::setTime(SkMSec time)
{
    SkMSec dur = this->duration();
    if (time > dur)
        time = dur;

    bool changed = false;
    if (time != fCurrTime)
    {
        fCurrTime = time;
        changed = this->onSetTime(time);
        fNeedBitmap |= changed;
    }
    return changed;
}

const SkBitmap& SkMovie::bitmap()
{
    if (fCurrTime == UNINITIALIZED_MSEC)    // uninitialized
        this->setTime(0);

    if (fNeedBitmap)
    {
        if (!this->onGetBitmap(&fBitmap))   // failure
            fBitmap.reset();
        fNeedBitmap = false;
    }
    return fBitmap;
}

////////////////////////////////////////////////////////////////////

#include "SkStream.h"

SkMovie* SkMovie::DecodeMemory(const void* data, size_t length) {
    SkMemoryStream stream(data, length, false);
    return SkMovie::DecodeStream(&stream);
}

SkMovie* SkMovie::DecodeFile(const char path[]) {
    SkAutoTDelete<SkStreamRewindable> stream(SkStream::NewFromFile(path));
    return stream.get() ? SkMovie::DecodeStream(stream) : nullptr;
}
