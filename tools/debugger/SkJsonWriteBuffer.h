/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJsonWriteBuffer_DEFINED
#define SkJsonWriteBuffer_DEFINED

#include "SkWriteBuffer.h"

#include "SkJSONCPP.h"

class SkPath;
class UrlDataManager;

class SkJsonWriteBuffer final : public SkWriteBuffer {
public:
    SkJsonWriteBuffer(UrlDataManager* urlDataManager)
        : fUrlDataManager(urlDataManager)
        , fJson(Json::objectValue) {}

    bool isCrossProcess() const override { return false; }

    void writeByteArray(const void* data, size_t size) override;
    void writeBool(bool value) override;
    void writeScalar(SkScalar value) override;
    void writeScalarArray(const SkScalar* value, uint32_t count) override;
    void writeInt(int32_t value) override;
    void writeIntArray(const int32_t* value, uint32_t count) override;
    void writeUInt(uint32_t value) override;
    void writeString(const char* value) override;

    void writeFlattenable(const SkFlattenable* flattenable) override;
    void writeColor(SkColor color) override;
    void writeColorArray(const SkColor* color, uint32_t count) override;
    void writePoint(const SkPoint& point) override;
    void writePointArray(const SkPoint* point, uint32_t count) override;
    void writeMatrix(const SkMatrix& matrix) override;
    void writeIRect(const SkIRect& rect) override;
    void writeRect(const SkRect& rect) override;
    void writeRegion(const SkRegion& region) override;
    void writePath(const SkPath& path) override;
    size_t writeStream(SkStream* stream, size_t length) override;
    void writeBitmap(const SkBitmap& bitmap) override;
    void writeImage(const SkImage*) override;
    void writeTypeface(SkTypeface* typeface) override;
    void writePaint(const SkPaint& paint) override;

    const Json::Value& getValue() const { return fJson; }

private:
    void append(const char* type, const Json::Value& value);

    UrlDataManager* fUrlDataManager;
    Json::Value fJson;
};

#endif
