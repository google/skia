/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "JsonWriteBuffer.h"

#include "DrawCommand.h"

void JsonWriteBuffer::append(const char* type) {
    SkString fullName = SkStringPrintf("%02d_%s", fCount++, type);
    fWriter->appendName(fullName.c_str());
}

void JsonWriteBuffer::writePad32(const void* data, size_t size) {
    this->append("rawBytes");
    fWriter->beginArray();
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data);
    for (size_t i = 0; i < size; ++i) {
        SkString hexByte = SkStringPrintf("%02x", bytes[i]);
        fWriter->appendString(hexByte.c_str());
    }
    fWriter->endArray();
}

void JsonWriteBuffer::writeByteArray(const void* data, size_t size) {
    this->append("byteArray");
    fWriter->beginArray();
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data);
    for (size_t i = 0; i < size; ++i) {
        SkString hexByte = SkStringPrintf("%02x", bytes[i]);
        fWriter->appendString(hexByte.c_str());
    }
    fWriter->endArray();
}

void JsonWriteBuffer::writeBool(bool value) {
    this->append("bool");
    fWriter->appendBool(value);
}

void JsonWriteBuffer::writeScalar(SkScalar value) {
    this->append("scalar");
    fWriter->appendFloat(value);
}

void JsonWriteBuffer::writeScalarArray(const SkScalar* value, uint32_t count) {
    this->append("scalarArray");
    fWriter->beginArray();
    for (uint32_t i = 0; i < count; ++i) {
        fWriter->appendFloat(value[i]);
    }
    fWriter->endArray();
}

void JsonWriteBuffer::writeInt(int32_t value) {
    this->append("int");
    fWriter->appendS32(value);
}

void JsonWriteBuffer::writeIntArray(const int32_t* value, uint32_t count) {
    this->append("intArray");
    fWriter->beginArray();
    for (uint32_t i = 0; i < count; ++i) {
        fWriter->appendS32(value[i]);
    }
    fWriter->endArray();
}

void JsonWriteBuffer::writeUInt(uint32_t value) {
    this->append("uint");
    fWriter->appendU32(value);
}

void JsonWriteBuffer::writeString(const char* value) {
    this->append("string");
    fWriter->appendString(value);
}

void JsonWriteBuffer::writeFlattenable(const SkFlattenable* flattenable) {
    if (flattenable) {
        this->append(flattenable->getTypeName());
        fWriter->beginObject();
        JsonWriteBuffer flattenableBuffer(fWriter, fUrlDataManager);
        flattenable->flatten(flattenableBuffer);
        fWriter->endObject();
    } else {
        this->append("flattenable");
        fWriter->appendPointer(nullptr);
    }
}

void JsonWriteBuffer::writeColor(SkColor color) {
    this->append("color");
    DrawCommand::MakeJsonColor(*fWriter, color);
}

void JsonWriteBuffer::writeColorArray(const SkColor* color, uint32_t count) {
    this->append("colorArray");
    fWriter->beginArray();
    for (uint32_t i = 0; i < count; ++i) {
        DrawCommand::MakeJsonColor(*fWriter, color[i]);
    }
    fWriter->endArray();
}

void JsonWriteBuffer::writeColor4f(const SkColor4f& color) {
    this->append("color");
    DrawCommand::MakeJsonColor4f(*fWriter, color);
}

void JsonWriteBuffer::writeColor4fArray(const SkColor4f* color, uint32_t count) {
    this->append("colorArray");
    fWriter->beginArray();
    for (uint32_t i = 0; i < count; ++i) {
        DrawCommand::MakeJsonColor4f(*fWriter, color[i]);
    }
    fWriter->endArray();
}

void JsonWriteBuffer::writePoint(const SkPoint& point) {
    this->append("point");
    DrawCommand::MakeJsonPoint(*fWriter, point);
}

void JsonWriteBuffer::writePoint3(const SkPoint3& point) {
    this->append("point3");
    DrawCommand::MakeJsonPoint3(*fWriter, point);
}

void JsonWriteBuffer::writePointArray(const SkPoint* point, uint32_t count) {
    this->append("pointArray");
    fWriter->beginArray();
    for (uint32_t i = 0; i < count; ++i) {
        DrawCommand::MakeJsonPoint(*fWriter, point[i]);
    }
    fWriter->endArray();
}

void JsonWriteBuffer::writeMatrix(const SkMatrix& matrix) {
    this->append("matrix");
    DrawCommand::MakeJsonMatrix(*fWriter, matrix);
}

void JsonWriteBuffer::writeIRect(const SkIRect& rect) {
    this->append("irect");
    DrawCommand::MakeJsonIRect(*fWriter, rect);
}

void JsonWriteBuffer::writeRect(const SkRect& rect) {
    this->append("rect");
    DrawCommand::MakeJsonRect(*fWriter, rect);
}

void JsonWriteBuffer::writeRegion(const SkRegion& region) {
    this->append("region");
    DrawCommand::MakeJsonRegion(*fWriter, region);
}

void JsonWriteBuffer::writePath(const SkPath& path) {
    this->append("path");
    DrawCommand::MakeJsonPath(*fWriter, path);
}

size_t JsonWriteBuffer::writeStream(SkStream* stream, size_t length) {
    // Contents not supported
    this->append("stream");
    fWriter->appendU64(static_cast<uint64_t>(length));
    return 0;
}

void JsonWriteBuffer::writeImage(const SkImage* image) {
    this->append("image");
    fWriter->beginObject();
    DrawCommand::flatten(*image, *fWriter, *fUrlDataManager);
    fWriter->endObject();
}

void JsonWriteBuffer::writeTypeface(SkTypeface* typeface) {
    // Unsupported
    this->append("typeface");
    fWriter->appendPointer(typeface);
}

void JsonWriteBuffer::writePaint(const SkPaint& paint) {
    this->append("paint");
    DrawCommand::MakeJsonPaint(*fWriter, paint, *fUrlDataManager);
}
