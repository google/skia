/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef JsonWriteBuffer_DEFINED
#define JsonWriteBuffer_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkM44.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "src/core/SkWriteBuffer.h"

#include <cstddef>
#include <cstdint>
#include <string_view>

class SkFlattenable;
class SkImage;
class SkJSONWriter;
class SkMatrix;
class SkPaint;
class SkPath;
class SkRegion;
struct SkSamplingOptions;
class SkStream;
class SkTypeface;
class UrlDataManager;
struct SkIRect;
struct SkPoint3;
struct SkPoint;
struct SkRect;

class JsonWriteBuffer final : public SkWriteBuffer {
public:
    JsonWriteBuffer(SkJSONWriter* writer, UrlDataManager* urlDataManager)
            : SkWriteBuffer({}), fUrlDataManager(urlDataManager), fWriter(writer), fCount(0) {}

    void writePad32(const void* buffer, size_t bytes) override;
    void writeByteArray(const void* data, size_t size) override;
    void writeBool(bool value) override;
    void writeScalar(SkScalar value) override;
    void writeScalarArray(const SkScalar* value, uint32_t count) override;
    void writeInt(int32_t value) override;
    void writeIntArray(const int32_t* value, uint32_t count) override;
    void writeUInt(uint32_t value) override;
    void writeString(std::string_view value) override;

    void   writeFlattenable(const SkFlattenable* flattenable) override;
    void   writeColor(SkColor color) override;
    void   writeColorArray(const SkColor* color, uint32_t count) override;
    void   writeColor4f(const SkColor4f& color) override;
    void   writeColor4fArray(const SkColor4f* color, uint32_t count) override;
    void   writePoint(const SkPoint& point) override;
    void   writePointArray(const SkPoint* point, uint32_t count) override;
    void   writePoint3(const SkPoint3& point) override;
    void   write(const SkM44&) override;
    void   writeMatrix(const SkMatrix& matrix) override;
    void   writeIRect(const SkIRect& rect) override;
    void   writeRect(const SkRect& rect) override;
    void   writeRegion(const SkRegion& region) override;
    void   writePath(const SkPath& path) override;
    void   writeSampling(const SkSamplingOptions&) override;
    size_t writeStream(SkStream* stream, size_t length) override;
    void   writeImage(const SkImage*) override;
    void   writeTypeface(SkTypeface* typeface) override;
    void   writePaint(const SkPaint& paint) override;

private:
    void append(const char* type);

    UrlDataManager* fUrlDataManager;
    SkJSONWriter*   fWriter;
    int             fCount;
};

#endif
