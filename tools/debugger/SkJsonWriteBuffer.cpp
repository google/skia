/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJsonWriteBuffer.h"

#include "SkDrawCommand.h"

void SkJsonWriteBuffer::append(const char* type) {
    SkString fullName = SkStringPrintf("%02d_%s", fCount++, type);
    fWriter->appendName(fullName.c_str());
}

void SkJsonWriteBuffer::writePad32(const void* data, size_t size) {
    this->append("rawBytes");
    fWriter->beginArray();
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data);
    for (size_t i = 0; i < size; ++i) {
        SkString hexByte = SkStringPrintf("%02x", bytes[i]);
        fWriter->appendString(hexByte.c_str());
    }
    fWriter->endArray();
}

void SkJsonWriteBuffer::writeByteArray(const void* data, size_t size) {
    this->append("byteArray");
    fWriter->beginArray();
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data);
    for (size_t i = 0; i < size; ++i) {
        SkString hexByte = SkStringPrintf("%02x", bytes[i]);
        fWriter->appendString(hexByte.c_str());
    }
    fWriter->endArray();
}

void SkJsonWriteBuffer::writeBool(bool value) {
    this->append("bool"); fWriter->appendBool(value);
}

void SkJsonWriteBuffer::writeScalar(SkScalar value) {
    this->append("scalar"); fWriter->appendFloat(value);
}

void SkJsonWriteBuffer::writeScalarArray(const SkScalar* value, uint32_t count) {
    this->append("scalarArray");
    fWriter->beginArray();
    for (uint32_t i = 0; i < count; ++i) {
        fWriter->appendFloat(value[i]);
    }
    fWriter->endArray();
}

void SkJsonWriteBuffer::writeInt(int32_t value) {
    this->append("int"); fWriter->appendS32(value);
}

void SkJsonWriteBuffer::writeIntArray(const int32_t* value, uint32_t count) {
    this->append("intArray");
    fWriter->beginArray();
    for (uint32_t i = 0; i < count; ++i) {
        fWriter->appendS32(value[i]);
    }
    fWriter->endArray();
}

void SkJsonWriteBuffer::writeUInt(uint32_t value) {
    this->append("uint"); fWriter->appendU32(value);
}

void SkJsonWriteBuffer::writeString(const char* value) {
    this->append("string"); fWriter->appendString(value);
}

void SkJsonWriteBuffer::writeFlattenable(const SkFlattenable* flattenable) {
    if (flattenable) {
        this->append(flattenable->getTypeName());
        fWriter->beginObject();
        SkJsonWriteBuffer flattenableBuffer(fWriter, fUrlDataManager);
        flattenable->flatten(flattenableBuffer);
        fWriter->endObject();
    } else {
        this->append("flattenable"); fWriter->appendPointer(nullptr);
    }
}

void SkJsonWriteBuffer::writeColor(SkColor color) {
    this->append("color"); SkDrawCommand::MakeJsonColor(*fWriter, color);
}

void SkJsonWriteBuffer::writeColorArray(const SkColor* color, uint32_t count) {
    this->append("colorArray");
    fWriter->beginArray();
    for (uint32_t i = 0; i < count; ++i) {
        SkDrawCommand::MakeJsonColor(*fWriter, color[i]);
    }
    fWriter->endArray();
}

void SkJsonWriteBuffer::writeColor4f(const SkColor4f& color) {
    this->append("color"); SkDrawCommand::MakeJsonColor4f(*fWriter, color);
}

void SkJsonWriteBuffer::writeColor4fArray(const SkColor4f* color, uint32_t count) {
    this->append("colorArray");
    fWriter->beginArray();
    for (uint32_t i = 0; i < count; ++i) {
        SkDrawCommand::MakeJsonColor4f(*fWriter, color[i]);
    }
    fWriter->endArray();
}

void SkJsonWriteBuffer::writePoint(const SkPoint& point) {
    this->append("point"); SkDrawCommand::MakeJsonPoint(*fWriter, point);
}

void SkJsonWriteBuffer::writePoint3(const SkPoint3& point) {
    this->append("point3"); SkDrawCommand::MakeJsonPoint3(*fWriter, point);
}

void SkJsonWriteBuffer::writePointArray(const SkPoint* point, uint32_t count) {
    this->append("pointArray");
    fWriter->beginArray();
    for (uint32_t i = 0; i < count; ++i) {
        SkDrawCommand::MakeJsonPoint(*fWriter, point[i]);
    }
    fWriter->endArray();
}

void SkJsonWriteBuffer::writeMatrix(const SkMatrix& matrix) {
    this->append("matrix"); SkDrawCommand::MakeJsonMatrix(*fWriter, matrix);
}

void SkJsonWriteBuffer::writeIRect(const SkIRect& rect) {
    this->append("irect"); SkDrawCommand::MakeJsonIRect(*fWriter, rect);
}

void SkJsonWriteBuffer::writeRect(const SkRect& rect) {
    this->append("rect"); SkDrawCommand::MakeJsonRect(*fWriter, rect);
}

void SkJsonWriteBuffer::writeRegion(const SkRegion& region) {
    this->append("region"); SkDrawCommand::MakeJsonRegion(*fWriter, region);
}

void SkJsonWriteBuffer::writePath(const SkPath& path) {
    this->append("path"); SkDrawCommand::MakeJsonPath(*fWriter, path);
}

size_t SkJsonWriteBuffer::writeStream(SkStream* stream, size_t length) {
    // Contents not supported
    this->append("stream"); fWriter->appendU64(static_cast<uint64_t>(length));
    return 0;
}

void SkJsonWriteBuffer::writeImage(const SkImage* image) {
    this->append("image");
    fWriter->beginObject();
    SkDrawCommand::flatten(*image, *fWriter, *fUrlDataManager);
    fWriter->endObject();
}

void SkJsonWriteBuffer::writeTypeface(SkTypeface* typeface) {
    // Unsupported
    this->append("typeface"); fWriter->appendPointer(typeface);
}

void SkJsonWriteBuffer::writePaint(const SkPaint& paint) {
    this->append("paint"); SkDrawCommand::MakeJsonPaint(*fWriter, paint, *fUrlDataManager);
}
