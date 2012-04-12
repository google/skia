
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOrderedReadBuffer_DEFINED
#define SkOrderedReadBuffer_DEFINED

#include "SkRefCnt.h"
#include "SkBitmap.h"
#include "SkFlattenable.h"
#include "SkWriter32.h"
#include "SkPath.h"

class SkOrderedReadBuffer : public SkFlattenableReadBuffer {
public:
    SkOrderedReadBuffer() : INHERITED() {}
    SkOrderedReadBuffer(const void* data, size_t size);

    void setMemory(const void* data, size_t size) { fReader.setMemory(data, size); }
    uint32_t size() { return fReader.size(); }
    const void* base() { return fReader.base(); }
    uint32_t offset() { return fReader.offset(); }
    bool eof() { return fReader.eof(); }
    void rewind() { fReader.rewind(); }
    void setOffset(size_t offset) { fReader.setOffset(offset); }

    SkReader32* getReader32() { return &fReader; }

    virtual uint8_t readU8() { return fReader.readU8(); }
    virtual uint16_t readU16() { return fReader.readU16(); }
    virtual uint32_t readU32() { return fReader.readU32(); }
    virtual void read(void* dst, size_t size) { return fReader.read(dst, size); }
    virtual bool readBool() { return fReader.readBool(); }
    virtual int32_t readInt() { return fReader.readInt(); }
    virtual SkScalar readScalar() { return fReader.readScalar(); }
    virtual const void* skip(size_t size) { return fReader.skip(size); }

    virtual void readMatrix(SkMatrix* m) { fReader.readMatrix(m); }
    virtual void readPath(SkPath* p) { p->unflatten(fReader); }
    virtual void readPoint(SkPoint* p) {
        p->fX = fReader.readScalar();
        p->fY = fReader.readScalar();
    }

    virtual SkTypeface* readTypeface();
    virtual SkRefCnt* readRefCnt();
    virtual void* readFunctionPtr();
    virtual SkFlattenable* readFlattenable();

private:
    SkReader32 fReader;

    typedef SkFlattenableReadBuffer INHERITED;
};

#endif // SkOrderedReadBuffer_DEFINED

