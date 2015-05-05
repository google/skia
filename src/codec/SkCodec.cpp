/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec.h"
#include "SkData.h"
#include "SkCodec_libbmp.h"
#include "SkCodec_libgif.h"
#include "SkCodec_libico.h"
#include "SkCodec_libpng.h"
#include "SkCodec_wbmp.h"
#include "SkCodecPriv.h"
#include "SkJpegCodec.h"
#include "SkStream.h"

struct DecoderProc {
    bool (*IsFormat)(SkStream*);
    SkCodec* (*NewFromStream)(SkStream*);
};

static const DecoderProc gDecoderProcs[] = {
    { SkPngCodec::IsPng, SkPngCodec::NewFromStream },
    { SkJpegCodec::IsJpeg, SkJpegCodec::NewFromStream },
    { SkGifCodec::IsGif, SkGifCodec::NewFromStream },
    { SkIcoCodec::IsIco, SkIcoCodec::NewFromStream },
    { SkBmpCodec::IsBmp, SkBmpCodec::NewFromStream },
    { SkWbmpCodec::IsWbmp, SkWbmpCodec::NewFromStream }
};

SkCodec* SkCodec::NewFromStream(SkStream* stream) {
    if (!stream) {
        return NULL;
    }

    SkAutoTDelete<SkStream> streamDeleter(stream);
    
    SkAutoTDelete<SkCodec> codec(NULL);
    for (uint32_t i = 0; i < SK_ARRAY_COUNT(gDecoderProcs); i++) {
        DecoderProc proc = gDecoderProcs[i];
        const bool correctFormat = proc.IsFormat(stream);
        if (!stream->rewind()) {
            return NULL;
        }
        if (correctFormat) {
            codec.reset(proc.NewFromStream(streamDeleter.detach()));
            break;
        }
    }

    // Set the max size at 128 megapixels (512 MB for kN32).
    // This is about 4x smaller than a test image that takes a few minutes for
    // dm to decode and draw.
    const int32_t maxSize = 1 << 27;
    if (codec && codec->getInfo().width() * codec->getInfo().height() > maxSize) {
        SkCodecPrintf("Error: Image size too large, cannot decode.\n");
        return NULL;
    } else {
        return codec.detach();
    }
}

SkCodec* SkCodec::NewFromData(SkData* data) {
    if (!data) {
        return NULL;
    }
    return NewFromStream(SkNEW_ARGS(SkMemoryStream, (data)));
}

SkCodec::SkCodec(const SkImageInfo& info, SkStream* stream)
    : INHERITED(info)
    , fStream(stream)
    , fNeedsRewind(false)
{}

SkCodec::RewindState SkCodec::rewindIfNeeded() {
    // Store the value of fNeedsRewind so we can update it. Next read will
    // require a rewind.
    const bool needsRewind = fNeedsRewind;
    fNeedsRewind = true;
    if (!needsRewind) {
        return kNoRewindNecessary_RewindState;
    }
    return fStream->rewind() ? kRewound_RewindState
                             : kCouldNotRewind_RewindState;
}

SkScanlineDecoder* SkCodec::getScanlineDecoder(const SkImageInfo& dstInfo, const Options* options,
        SkPMColor ctable[], int* ctableCount) {

    // Set options.
    Options optsStorage;
    if (NULL == options) {
        options = &optsStorage;
    }

    fScanlineDecoder.reset(this->onGetScanlineDecoder(dstInfo, *options, ctable, ctableCount));
    return fScanlineDecoder.get();
}

SkScanlineDecoder* SkCodec::getScanlineDecoder(const SkImageInfo& dstInfo) {
    SkASSERT(kIndex_8_SkColorType != dstInfo.colorType());
    if (kIndex_8_SkColorType == dstInfo.colorType()) {
        return NULL;
    }
    return this->getScanlineDecoder(dstInfo, NULL, NULL, NULL);
}
