
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOrderedWriteBuffer_DEFINED
#define SkOrderedWriteBuffer_DEFINED

#include "SkRefCnt.h"
#include "SkBitmap.h"
#include "SkFlattenable.h"
#include "SkWriter32.h"
#include "SkPath.h"

class SkOrderedWriteBuffer : public SkFlattenableWriteBuffer {
public:
    SkOrderedWriteBuffer(size_t minSize);
    SkOrderedWriteBuffer(size_t minSize, void* initialStorage,
                         size_t storageSize);
    virtual ~SkOrderedWriteBuffer() {}

    // deprecated naming convention that will be removed after callers are updated
    virtual bool writeBool(bool value) { return fWriter.writeBool(value); }
    virtual void writeInt(int32_t value) { fWriter.writeInt(value); }
    virtual void write8(int32_t value) { fWriter.write8(value); }
    virtual void write16(int32_t value) { fWriter.write16(value); }
    virtual void write32(int32_t value) { fWriter.write32(value); }
    virtual void writeScalar(SkScalar value) { fWriter.writeScalar(value); }
    virtual void writeMul4(const void* values, size_t size) { fWriter.writeMul4(values, size); }

    virtual void writePad(const void* src, size_t size) { fWriter.writePad(src, size); }
    virtual void writeString(const char* str, size_t len = (size_t)-1) { fWriter.writeString(str, len); }
    virtual bool writeToStream(SkWStream* stream) { return fWriter.writeToStream(stream); }
    virtual void write(const void* values, size_t size) { fWriter.write(values, size); }
    virtual void writeRect(const SkRect& rect) { fWriter.writeRect(rect); }
    virtual size_t readFromStream(SkStream* s, size_t length) { return fWriter.readFromStream(s, length); }

    virtual void writeMatrix(const SkMatrix& matrix) { fWriter.writeMatrix(matrix); }
    virtual void writePath(const SkPath& path) { path.flatten(fWriter); };
    virtual void writePoint(const SkPoint& point) {
        fWriter.writeScalar(point.fX);
        fWriter.writeScalar(point.fY);
    }

    virtual uint32_t* reserve(size_t size) { return fWriter.reserve(size); }
    virtual void flatten(void* dst) { fWriter.flatten(dst); }
    virtual uint32_t size() { return fWriter.size(); }

    virtual void writeFunctionPtr(void*);
    virtual void writeFlattenable(SkFlattenable* flattenable);

private:
    SkWriter32 fWriter;
    typedef SkFlattenableWriteBuffer INHERITED;
};

#endif // SkOrderedWriteBuffer_DEFINED

