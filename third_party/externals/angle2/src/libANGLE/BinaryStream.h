//
// Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BinaryStream.h: Provides binary serialization of simple types.

#ifndef LIBANGLE_BINARYSTREAM_H_
#define LIBANGLE_BINARYSTREAM_H_

#include "common/angleutils.h"
#include "common/mathutil.h"

#include <cstddef>
#include <string>
#include <vector>
#include <stdint.h>

template <typename T>
void StaticAssertIsFundamental()
{
    // c++11 STL is not available on OSX or Android
#if !defined(ANGLE_PLATFORM_APPLE) && !defined(ANGLE_PLATFORM_ANDROID)
    static_assert(std::is_fundamental<T>::value, "T must be a fundamental type.");
#else
    union { T dummy; } dummy;
    static_cast<void>(dummy);
#endif
}

namespace gl
{

class BinaryInputStream : angle::NonCopyable
{
  public:
    BinaryInputStream(const void *data, size_t length)
    {
        mError = false;
        mOffset = 0;
        mData = static_cast<const uint8_t*>(data);
        mLength = length;
    }

    // readInt will generate an error for bool types
    template <class IntT>
    IntT readInt()
    {
        int value = 0;
        read(&value);
        return static_cast<IntT>(value);
    }

    template <class IntT>
    void readInt(IntT *outValue)
    {
        *outValue = readInt<IntT>();
    }

    bool readBool()
    {
        int value = 0;
        read(&value);
        return (value > 0);
    }

    void readBool(bool *outValue)
    {
        *outValue = readBool();
    }

    void readBytes(unsigned char outArray[], size_t count)
    {
        read<unsigned char>(outArray, count);
    }

    std::string readString()
    {
        std::string outString;
        readString(&outString);
        return outString;
    }

    void readString(std::string *v)
    {
        size_t length;
        readInt(&length);

        if (mError)
        {
            return;
        }

        if (mOffset + length > mLength)
        {
            mError = true;
            return;
        }

        v->assign(reinterpret_cast<const char *>(mData) + mOffset, length);
        mOffset += length;
    }

    void skip(size_t length)
    {
        if (mOffset + length > mLength)
        {
            mError = true;
            return;
        }

        mOffset += length;
    }

    size_t offset() const
    {
        return mOffset;
    }

    bool error() const
    {
        return mError;
    }

    bool endOfStream() const
    {
        return mOffset == mLength;
    }

    const uint8_t *data()
    {
        return mData;
    }

  private:
    bool mError;
    size_t mOffset;
    const uint8_t *mData;
    size_t mLength;

    template <typename T>
    void read(T *v, size_t num)
    {
        StaticAssertIsFundamental<T>();

        size_t length = num * sizeof(T);

        if (mOffset + length > mLength)
        {
            mError = true;
            return;
        }

        memcpy(v, mData + mOffset, length);
        mOffset += length;
    }

    template <typename T>
    void read(T *v)
    {
        read(v, 1);
    }

};

class BinaryOutputStream : angle::NonCopyable
{
  public:
    BinaryOutputStream()
    {
    }

    // writeInt also handles bool types
    template <class IntT>
    void writeInt(IntT param)
    {
        ASSERT(rx::IsIntegerCastSafe<int>(param));
        int intValue = static_cast<int>(param);
        write(&intValue, 1);
    }

    void writeString(const std::string &v)
    {
        writeInt(v.length());
        write(v.c_str(), v.length());
    }

    void writeBytes(const unsigned char *bytes, size_t count)
    {
        write(bytes, count);
    }

    size_t length() const
    {
        return mData.size();
    }

    const void* data() const
    {
        return mData.size() ? &mData[0] : NULL;
    }

  private:
    std::vector<char> mData;

    template <typename T>
    void write(const T *v, size_t num)
    {
        StaticAssertIsFundamental<T>();
        const char *asBytes = reinterpret_cast<const char*>(v);
        mData.insert(mData.end(), asBytes, asBytes + num * sizeof(T));
    }

};
}

#endif  // LIBANGLE_BINARYSTREAM_H_
