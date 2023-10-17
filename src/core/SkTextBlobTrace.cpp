// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "src/core/SkTextBlobTrace.h"

#include "include/core/SkFontMgr.h"
#include "include/core/SkTextBlob.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkPtrRecorder.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkTextBlobPriv.h"
#include "src/core/SkWriteBuffer.h"

std::vector<SkTextBlobTrace::Record> SkTextBlobTrace::CreateBlobTrace(SkStream* stream) {
    std::vector<SkTextBlobTrace::Record> trace;

    uint32_t typefaceCount;
    if (!stream->readU32(&typefaceCount)) {
        return trace;
    }

    std::vector<sk_sp<SkTypeface>> typefaceArray;
    for (uint32_t i = 0; i < typefaceCount; i++) {
        typefaceArray.push_back(SkTypeface::MakeDeserialize(stream, SkFontMgr::RefDefault()));
    }

    uint32_t restOfFile;
    if (!stream->readU32(&restOfFile)) {
        return trace;
    }
    sk_sp<SkData> data = SkData::MakeFromStream(stream, restOfFile);
    SkReadBuffer readBuffer{data->data(), data->size()};
    readBuffer.setTypefaceArray(typefaceArray.data(), typefaceArray.size());

    while (!readBuffer.eof()) {
        SkTextBlobTrace::Record record;
        record.origUniqueID = readBuffer.readUInt();
        record.paint = readBuffer.readPaint();
        readBuffer.readPoint(&record.offset);
        record.blob = SkTextBlobPriv::MakeFromBuffer(readBuffer);
        trace.push_back(std::move(record));
    }
    return trace;
}

void SkTextBlobTrace::DumpTrace(const std::vector<SkTextBlobTrace::Record>& trace) {
    for (const SkTextBlobTrace::Record& record : trace) {
        const SkTextBlob* blob = record.blob.get();
        const SkPaint& p = record.paint;
        bool weirdPaint = p.getStyle() != SkPaint::kFill_Style
        || p.getMaskFilter() != nullptr
        || p.getPathEffect() != nullptr;

        SkDebugf("Blob %d ( %g %g ) %d\n  ",
                blob->uniqueID(), record.offset.x(), record.offset.y(), weirdPaint);
        SkTextBlobRunIterator iter(blob);
        int runNumber = 0;
        while (!iter.done()) {
            SkDebugf("Run %d\n    ", runNumber);
            SkFont font = iter.font();
            SkDebugf("Font %d %g %g %g %d %d %d\n    ",
                    font.getTypefaceOrDefault()->uniqueID(),
                    font.getSize(),
                    font.getScaleX(),
                    font.getSkewX(),
                    SkFontPriv::Flags(font),
                    (int)font.getEdging(),
                    (int)font.getHinting());
            uint32_t glyphCount = iter.glyphCount();
            const uint16_t* glyphs = iter.glyphs();
            for (uint32_t i = 0; i < glyphCount; i++) {
                SkDebugf("%02X ", glyphs[i]);
            }
            SkDebugf("\n");
            runNumber += 1;
            iter.next();
        }
    }
}

SkTextBlobTrace::Capture::Capture() : fTypefaceSet(new SkRefCntSet), fWriteBuffer({}) {
    fWriteBuffer.setTypefaceRecorder(fTypefaceSet);
}

SkTextBlobTrace::Capture::~Capture() = default;

void SkTextBlobTrace::Capture::capture(
        const sktext::GlyphRunList& glyphRunList, const SkPaint& paint) {
    const SkTextBlob* blob = glyphRunList.blob();
    if (blob != nullptr) {
        fWriteBuffer.writeUInt(blob->uniqueID());
        fWriteBuffer.writePaint(paint);
        fWriteBuffer.writePoint(glyphRunList.origin());
        SkTextBlobPriv::Flatten(*blob, fWriteBuffer);
        fBlobCount++;
    }
}

void SkTextBlobTrace::Capture::dump(SkWStream* dst) const {
    SkTLazy<SkFILEWStream> fileStream;
    if (!dst) {
        uint32_t id = SkChecksum::Mix(reinterpret_cast<uintptr_t>(this));
        SkString f = SkStringPrintf("diff-canvas-%08x-%04zu.trace", id, fBlobCount);
        dst = fileStream.init(f.c_str());
        if (!fileStream->isValid()) {
            SkDebugf("Error opening '%s'.\n", f.c_str());
            return;
        }
        SkDebugf("Saving trace to '%s'.\n", f.c_str());
    }
    SkASSERT(dst);
    int count = fTypefaceSet->count();
    dst->write32(count);
    SkPtrSet::Iter iter(*fTypefaceSet);
    while (void* ptr = iter.next()) {
        ((const SkTypeface*)ptr)->serialize(dst, SkTypeface::SerializeBehavior::kDoIncludeData);
    }
    dst->write32(fWriteBuffer.bytesWritten());
    fWriteBuffer.writeToStream(dst);
}
