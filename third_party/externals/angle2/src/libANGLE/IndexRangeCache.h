//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// IndexRangeCache.h: Defines the gl::IndexRangeCache class which stores information about
// ranges of indices.

#ifndef LIBANGLE_INDEXRANGECACHE_H_
#define LIBANGLE_INDEXRANGECACHE_H_

#include "common/angleutils.h"
#include "common/mathutil.h"

#include "angle_gl.h"

#include <map>

namespace gl
{

class IndexRangeCache
{
  public:
    void addRange(GLenum type,
                  size_t offset,
                  size_t count,
                  bool primitiveRestartEnabled,
                  const IndexRange &range);
    bool findRange(GLenum type,
                   size_t offset,
                   size_t count,
                   bool primitiveRestartEnabled,
                   IndexRange *outRange) const;

    void invalidateRange(size_t offset, size_t size);
    void clear();

  private:
    struct IndexRangeKey
    {
        IndexRangeKey();
        IndexRangeKey(GLenum type, size_t offset, size_t count, bool primitiveRestart);

        bool operator<(const IndexRangeKey &rhs) const;

        GLenum type;
        size_t offset;
        size_t count;
        bool primitiveRestartEnabled;
    };

    typedef std::map<IndexRangeKey, IndexRange> IndexRangeMap;
    IndexRangeMap mIndexRangeCache;
};

}

#endif // LIBANGLE_INDEXRANGECACHE_H_
