//
// Copyright (c) 2002-2011 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// HandleAllocator.cpp: Implements the gl::HandleAllocator class, which is used
// to allocate GL handles.

#include "libANGLE/HandleAllocator.h"

#include <algorithm>

#include "common/debug.h"

namespace gl
{

struct HandleAllocator::HandleRangeComparator
{
    bool operator()(const HandleRange &range, GLuint handle) const
    {
        return (range.end < handle);
    }
};

HandleAllocator::HandleAllocator() : mBaseValue(1), mNextValue(1)
{
    mUnallocatedList.push_back(HandleRange(1, std::numeric_limits<GLuint>::max()));
}

HandleAllocator::HandleAllocator(GLuint maximumHandleValue) : mBaseValue(1), mNextValue(1)
{
    mUnallocatedList.push_back(HandleRange(1, maximumHandleValue));
}

HandleAllocator::~HandleAllocator()
{
}

void HandleAllocator::setBaseHandle(GLuint value)
{
    ASSERT(mBaseValue == mNextValue);
    mBaseValue = value;
    mNextValue = value;
}

GLuint HandleAllocator::allocate()
{
    ASSERT(!mUnallocatedList.empty() || !mReleasedList.empty());

    // Allocate from released list, constant time.
    if (!mReleasedList.empty())
    {
        GLuint reusedHandle = mReleasedList.back();
        mReleasedList.pop_back();
        return reusedHandle;
    }

    // Allocate from unallocated list, constant time.
    auto listIt = mUnallocatedList.begin();

    GLuint freeListHandle = listIt->begin;
    ASSERT(freeListHandle > 0);

    listIt->begin++;
    if (listIt->begin == listIt->end)
    {
        mUnallocatedList.erase(listIt);
    }

    return freeListHandle;
}

void HandleAllocator::release(GLuint handle)
{
    // Add to released list, constant time.
    mReleasedList.push_back(handle);
}

void HandleAllocator::reserve(GLuint handle)
{
    // Clear from released list -- might be a slow operation.
    if (!mReleasedList.empty())
    {
        auto releasedIt = std::find(mReleasedList.begin(), mReleasedList.end(), handle);
        if (releasedIt != mReleasedList.end())
        {
            mReleasedList.erase(releasedIt);
            return;
        }
    }

    // Not in released list, reserve in the unallocated list.
    auto boundIt = std::lower_bound(mUnallocatedList.begin(), mUnallocatedList.end(), handle, HandleRangeComparator());

    ASSERT(boundIt != mUnallocatedList.end());

    GLuint begin = boundIt->begin;
    GLuint end = boundIt->end;

    if (handle == begin || handle == end)
    {
        if (begin + 1 == end)
        {
            mUnallocatedList.erase(boundIt);
        }
        else if (handle == begin)
        {
            boundIt->begin++;
        }
        else
        {
            ASSERT(handle == end);
            boundIt->end--;
        }
        return;
    }

    // need to split the range
    auto placementIt = mUnallocatedList.erase(boundIt);

    if (handle + 1 != end)
    {
        placementIt = mUnallocatedList.insert(placementIt, HandleRange(handle + 1, end));
    }
    if (begin != handle)
    {
        mUnallocatedList.insert(placementIt, HandleRange(begin, handle));
    }
}

}
