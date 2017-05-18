/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJsonWriteBuffer.h"

#include "SkDrawCommand.h"
#include "SkObjectParser.h"

void SkJsonWriteBuffer::append(const char* type, const Json::Value& value) {
    SkString fullName = SkStringPrintf("%02d_%s", fJson.size(), type);
    fJson[fullName.c_str()] = value;
}

void SkJsonWriteBuffer::writeByteArray(const void* data, size_t size) {
    Json::Value jsonArray(Json::arrayValue);
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data);
    for (size_t i = 0; i < size; ++i) {
        SkString hexByte = SkStringPrintf("%02x", bytes[i]);
        jsonArray.append(hexByte.c_str());
    }
    this->append("byteArray", jsonArray);
}

void SkJsonWriteBuffer::writeBool(bool value) {
    this->append("bool", value);
}

void SkJsonWriteBuffer::writeScalar(SkScalar value) {
    this->append("scalar", value);
}

void SkJsonWriteBuffer::writeScalarArray(const SkScalar* value, uint32_t count) {
    Json::Value jsonArray(Json::arrayValue);
    for (uint32_t i = 0; i < count; ++i) {
        jsonArray.append(value[i]);
    }
    this->append("scalarArray", jsonArray);
}

void SkJsonWriteBuffer::writeInt(int32_t value) {
    this->append("int", value);
}

void SkJsonWriteBuffer::writeIntArray(const int32_t* value, uint32_t count) {
    Json::Value jsonArray(Json::arrayValue);
    for (uint32_t i = 0; i < count; ++i) {
        jsonArray.append(value[i]);
    }
    this->append("intArray", jsonArray);
}

void SkJsonWriteBuffer::writeUInt(uint32_t value) {
    this->append("uint", value);
}

void SkJsonWriteBuffer::writeString(const char* value) {
    this->append("string", value);
}

void SkJsonWriteBuffer::writeFlattenable(const SkFlattenable* flattenable) {
    if (flattenable) {
        SkJsonWriteBuffer flattenableBuffer(fUrlDataManager);
        flattenable->flatten(flattenableBuffer);
        this->append(flattenable->getTypeName(), flattenableBuffer.getValue());
    } else {
        this->append("flattenable", Json::Value());
    }
}

void SkJsonWriteBuffer::writeColor(SkColor color) {
    this->append("color", SkDrawCommand::MakeJsonColor(color));
}

void SkJsonWriteBuffer::writeColorArray(const SkColor* color, uint32_t count) {
    Json::Value jsonArray(Json::arrayValue);
    for (uint32_t i = 0; i < count; ++i) {
        jsonArray.append(SkDrawCommand::MakeJsonColor(color[i]));
    }
    this->append("colorArray", jsonArray);
}

void SkJsonWriteBuffer::writeColor4f(const SkColor4f& color) {
    this->append("color", SkDrawCommand::MakeJsonColor4f(color));
}

void SkJsonWriteBuffer::writeColor4fArray(const SkColor4f* color, uint32_t count) {
    Json::Value jsonArray(Json::arrayValue);
    for (uint32_t i = 0; i < count; ++i) {
        jsonArray.append(SkDrawCommand::MakeJsonColor4f(color[i]));
    }
    this->append("colorArray", jsonArray);
}

void SkJsonWriteBuffer::writePoint(const SkPoint& point) {
    this->append("point", SkDrawCommand::MakeJsonPoint(point));
}

void SkJsonWriteBuffer::writePointArray(const SkPoint* point, uint32_t count) {
    Json::Value jsonArray(Json::arrayValue);
    for (uint32_t i = 0; i < count; ++i) {
        jsonArray.append(SkDrawCommand::MakeJsonPoint(point[i]));
    }
    this->append("pointArray", jsonArray);
}

void SkJsonWriteBuffer::writeMatrix(const SkMatrix& matrix) {
    this->append("matrix", SkDrawCommand::MakeJsonMatrix(matrix));
}

void SkJsonWriteBuffer::writeIRect(const SkIRect& rect) {
    this->append("irect", SkDrawCommand::MakeJsonIRect(rect));
}

void SkJsonWriteBuffer::writeRect(const SkRect& rect) {
    this->append("rect", SkDrawCommand::MakeJsonRect(rect));
}

void SkJsonWriteBuffer::writeRegion(const SkRegion& region) {
    this->append("region", SkDrawCommand::MakeJsonRegion(region));
}

void SkJsonWriteBuffer::writePath(const SkPath& path) {
    this->append("path", SkDrawCommand::MakeJsonPath(path));
}

size_t SkJsonWriteBuffer::writeStream(SkStream* stream, size_t length) {
    // Contents not supported
    SkASSERT(length < Json::Value::maxUInt);
    this->append("stream", static_cast<Json::UInt>(length));
    return 0;
}

void SkJsonWriteBuffer::writeBitmap(const SkBitmap& bitmap) {
    Json::Value jsonBitmap;
    SkDrawCommand::flatten(bitmap, &jsonBitmap, *fUrlDataManager);
    this->append("bitmap", jsonBitmap);
}

void SkJsonWriteBuffer::writeImage(const SkImage* image) {
    Json::Value jsonImage;
    SkDrawCommand::flatten(*image, &jsonImage, *fUrlDataManager);
    this->append("image", jsonImage);
}

void SkJsonWriteBuffer::writeTypeface(SkTypeface* typeface) {
    // Unsupported
    this->append("typeface", Json::Value());
}

void SkJsonWriteBuffer::writePaint(const SkPaint& paint) {
    this->append("paint", SkDrawCommand::MakeJsonPaint(paint, *fUrlDataManager));
}
