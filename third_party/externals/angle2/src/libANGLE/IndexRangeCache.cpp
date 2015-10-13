//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// IndexRangeCache.cpp: Defines the gl::IndexRangeCache class which stores information about
// ranges of indices.

#include "libANGLE/IndexRangeCache.h"

#include "common/debug.h"
#include "libANGLE/formatutils.h"

namespace gl
{

void IndexRangeCache::addRange(GLenum type,
                               size_t offset,
                               size_t count,
                               bool primitiveRestartEnabled,
                               const IndexRange &range)
{
    mIndexRangeCache[IndexRangeKey(type, offset, count, primitiveRestartEnabled)] = range;
}

bool IndexRangeCache::findRange(GLenum type,
                                size_t offset,
                                size_t count,
                                bool primitiveRestartEnabled,
                                IndexRange *outRange) const
{
    auto i = mIndexRangeCache.find(IndexRangeKey(type, offset, count, primitiveRestartEnabled));
    if (i != mIndexRangeCache.end())
    {
        if (outRange)
        {
            *outRange = i->second;
        }
        return true;
    }
    else
    {
        if (outRange)
        {
            *outRange = IndexRange();
        }
        return false;
    }
}

void IndexRangeCache::invalidateRange(size_t offset, size_t size)
{
    size_t invalidateStart = offset;
    size_t invalidateEnd   = offset + size;

    auto i = mIndexRangeCache.begin();
    while (i != mIndexRangeCache.end())
    {
        size_t rangeStart = i->first.offset;
        size_t rangeEnd   = i->first.offset + (GetTypeInfo(i->first.type).bytes * i->first.count);

        if (invalidateEnd < rangeStart || invalidateStart > rangeEnd)
        {
            ++i;
        }
        else
        {
            mIndexRangeCache.erase(i++);
        }
    }
}

void IndexRangeCache::clear()
{
    mIndexRangeCache.clear();
}

IndexRangeCache::IndexRangeKey::IndexRangeKey()
    : IndexRangeCache::IndexRangeKey(GL_NONE, 0, 0, false)
{
}

IndexRangeCache::IndexRangeKey::IndexRangeKey(GLenum type_,
                                              size_t offset_,
                                              size_t count_,
                                              bool primitiveRestartEnabled_)
    : type(type_), offset(offset_), count(count_), primitiveRestartEnabled(primitiveRestartEnabled_)
{
}

bool IndexRangeCache::IndexRangeKey::operator<(const IndexRangeKey &rhs) const
{
    if (type != rhs.type)
    {
        return type < rhs.type;
    }
    if (offset != rhs.offset)
    {
        return offset < rhs.offset;
    }
    if (count != rhs.count)
    {
        return count < rhs.count;
    }
    if (primitiveRestartEnabled != rhs.primitiveRestartEnabled)
    {
        return primitiveRestartEnabled;
    }
    return false;
}

}
